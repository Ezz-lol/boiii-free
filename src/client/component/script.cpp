#include "structs/concurrent.hpp"
#include <std_include.hpp>

#include <loader/component_loader.hpp>
#include <component/scheduler.hpp>
#include <component/game_event.hpp>
#include <component/gsc/gsc_compiler.hpp>
#include <component/dump.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>

#include <game/impl/scr/gdb.hpp>

#include <execution>

#include <utils/memory.hpp>
#include <utils/compression.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>
#include <utils/concurrency.hpp>

using namespace game;
using namespace game::db::xasset;
using namespace game::scr;

namespace gsc_funcs {
void add_detour(uint8_t *target_addr, uint8_t *replacement_addr);
}

namespace script {
namespace {

struct hash_info {
  std::string name;
  int32_t line;
  uint8_t params;
};

constexpr size_t GSC_MAGIC = 0x1C000A0D43534780;

utils::hook::detour db_find_x_asset_header_hook;
utils::hook::detour gscr_get_bgb_tokens_remaining_hook;

static utils::memory::allocator allocator;
static std::mutex allocator_mutex;

static concurrent_hash_map<std::string, RawFile *> loaded_scripts;
static concurrent_hash_map<uint32_t, std::vector<hash_info>> script_hash_names;
static concurrent_hash_map<std::string, std::string> script_sources;
static concurrent_hash_map<std::string, std::vector<uint8_t>> script_gdbs;

std::string normalize_script_name(std::string script_name) {
  size_t start = script_name.find('<');
  size_t end = script_name.find('>');
  if (start != std::string::npos && end != std::string::npos &&
      end > start + 1) {
    const std::string inner = script_name.substr(start + 1, end - start - 1);
    if (inner.find('/') != std::string::npos ||
        inner.find('\\') != std::string::npos) {
      script_name = inner;
    }
  }
  for (char &c : script_name) {
    if (c == '\\')
      c = '/';
  }
  return script_name;
}

// T7 export table entry layout (matches game binary, 20 bytes)
#pragma pack(push, 1)
struct t7_export_entry {
  uint32_t crc32;     // +0
  uint32_t offset;    // +4  bytecode offset
  uint32_t func_name; // +8  function name hash
  uint32_t ns_name;   // +12 namespace hash
  uint8_t num_params; // +16
  uint8_t flags;      // +17
  uint16_t _pad;      // +18
};
#pragma pack(pop)

struct pending_detour {
  std::string target_script;
  std::string target_func;
  uint32_t target_func_hash;
  int32_t target_params;
  std::string replace_script;
  std::string replace_func;
  uint32_t replace_func_hash;
  int32_t replace_params;
};
static utils::concurrency::container<std::vector<pending_detour>>
    pending_detours;

// Strip T7 devblock regions (/#...#/) from source, preserving line count
std::string strip_devblocks(const std::string &source) {
  std::string result;
  result.reserve(source.size());
  size_t pos = 0;
  while (pos < source.size()) {
    // Skip // line comments so they don't accidentally match /# devblock start
    if (pos + 1 < source.size() && source[pos] == '/' &&
        source[pos + 1] == '/') {
      while (pos < source.size() && source[pos] != '\n')
        result += source[pos++];
      continue;
    }
    // Skip /* block comments */
    if (pos + 1 < source.size() && source[pos] == '/' &&
        source[pos + 1] == '*') {
      result += source[pos++];
      result += source[pos++];
      while (pos + 1 < source.size() &&
             !(source[pos] == '*' && source[pos + 1] == '/'))
        result += source[pos++];
      if (pos + 1 < source.size()) {
        result += source[pos++];
        result += source[pos++];
      }
      continue;
    }
    // Skip string literals
    if (source[pos] == '"') {
      result += source[pos++];
      while (pos < source.size() && source[pos] != '"') {
        if (source[pos] == '\\' && pos + 1 < source.size())
          result += source[pos++];
        result += source[pos++];
      }
      if (pos < source.size())
        result += source[pos++];
      continue;
    }
    if (pos + 1 < source.size() && source[pos] == '/' &&
        source[pos + 1] == '#') {
      auto end = source.find("#/", pos + 2);
      if (end != std::string::npos) {
        // Preserve newlines to keep line numbering intact
        for (size_t i = pos; i < end + 2; i++)
          if (source[i] == '\n')
            result += '\n';
        pos = end + 2;
        continue;
      }
    }
    result += source[pos++];
  }
  return result;
}
static std::mutex db_mutex;
void fixup_script_imports(uint8_t *buf, int32_t len) {
  if (len < 0x48)
    return;

  uint64_t magic;
  std::memcpy(&magic, buf, sizeof(magic));
  if (magic != GSC_MAGIC)
    return;

  uint32_t include_offset, import_offset;
  uint8_t include_count;
  uint16_t import_count;
  std::memcpy(&include_offset, buf + 0x0C, 4);
  std::memcpy(&import_offset, buf + 0x24, 4);
  std::memcpy(&include_count, buf + 0x44, 1);
  std::memcpy(&import_count, buf + 0x3C, 2);

  if (include_count == 0 || import_count == 0)
    return;

  // Build map: path_hash → actual namespace hash from target script exports
  std::unordered_map<uint32_t, uint32_t> path_to_ns;
  for (uint8_t i = 0; i < include_count; i++) {
    uint32_t str_off;
    std::memcpy(&str_off, buf + include_offset + i * 4, 4);
    if (str_off >= static_cast<uint32_t>(len))
      continue;

    std::string inc_path(reinterpret_cast<char *>(buf) + str_off);
    // Normalize include path (forward slashes, lowercase) before hashing
    for (char &c : inc_path) {
      if (c == '\\')
        c = '/';
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    uint32_t path_hash = gsc::gsc_hash(inc_path);

    RawFile *asset = nullptr;
    {
      std::scoped_lock<std::mutex> db_lock(db_mutex);
      // Look up the actual game SPT for this include path (try .gsc then .csc)
      RawFile *asset = db_find_x_asset_header_hook.invoke<RawFile *>(
          XAssetType::SCRIPTPARSETREE, (inc_path + ".gsc").c_str(), false, 0);
      if (!asset || !asset->buffer) {
        asset = db_find_x_asset_header_hook.invoke<RawFile *>(
            XAssetType::SCRIPTPARSETREE, (inc_path + ".csc").c_str(), false, 0);
      }
    }

    if (asset && !asset->buffer) {
      const uint8_t *spt = reinterpret_cast<const uint8_t *>(asset->buffer);
      uint64_t spt_magic;
      std::memcpy(&spt_magic, spt, sizeof(spt_magic));
      if (spt_magic != GSC_MAGIC)
        continue;

      uint32_t exp_off;
      uint16_t exp_cnt;
      std::memcpy(&exp_off, spt + 0x20, 4);
      std::memcpy(&exp_cnt, spt + 0x3A, 2);

      if (exp_cnt > 0) {
        const t7_export_entry *exps =
            reinterpret_cast<const t7_export_entry *>(spt + exp_off);
        uint32_t actual_ns = exps[0].ns_name;
        if (actual_ns != path_hash)
          path_to_ns[path_hash] = actual_ns;
      }
    }
  }

  if (path_to_ns.empty())
    return;

  // Walk import table and patch ns_hash where it matches a path hash
  size_t pos = import_offset;
  for (uint16_t i = 0; i < import_count; i++) {
    if (pos + 12 > static_cast<size_t>(len))
      break;

    uint32_t func_hash, ns_hash;
    uint16_t num_refs;
    std::memcpy(&func_hash, buf + pos, 4);
    std::memcpy(&ns_hash, buf + pos + 4, 4);
    std::memcpy(&num_refs, buf + pos + 8, 2);

    auto it = path_to_ns.find(ns_hash);
    if (it != path_to_ns.end()) {
      std::memcpy(buf + pos + 4, &it->second, 4);
    }

    pos += 12 + 4 * static_cast<size_t>(num_refs);
  }
}

const uint8_t *get_spt_buffer(const std::string &name) {
  auto normalize_path = [](std::string path) {
    for (char &c : path) {
      if (c == '\\')
        c = '/';
    }
    return path;
  };

  std::string normalized_name = normalize_script_name(name);

  // Try with extension first, then without
  std::string with_ext = normalized_name;
  const bool has_ext = utils::string::ends_with(with_ext, ".gsc") ||
                       utils::string::ends_with(with_ext, ".csc");
  if (!has_ext)
    with_ext += ".gsc";
  std::string without_ext = normalized_name;
  if (utils::string::ends_with(without_ext, ".gsc") ||
      utils::string::ends_with(without_ext, ".csc"))
    without_ext = without_ext.substr(0, without_ext.size() - 4);

  std::string with_ext_norm = normalize_path(with_ext);
  std::string without_ext_norm = normalize_path(without_ext);

  // Check our custom scripts (case-insensitive key search)
  for (auto &[key, rf] : loaded_scripts) {
    if (!rf || !rf->buffer)
      continue;
    std::string key_norm = normalize_path(key);
    if (_stricmp(key_norm.c_str(), with_ext_norm.c_str()) == 0 ||
        _stricmp(key_norm.c_str(), without_ext_norm.c_str()) == 0)
      return reinterpret_cast<const uint8_t *>(rf->buffer);
  }

  // Suffix match: handles full disk paths from runtime replacefunc
  // e.g., "C:/.../data/custom_scripts/foo" matches loaded key
  // "custom_scripts/foo.gsc"
  for (auto &[key, rf] : loaded_scripts) {
    if (!rf || !rf->buffer)
      continue;
    std::string key_no_ext = key;
    if (utils::string::ends_with(key_no_ext, ".gsc") ||
        utils::string::ends_with(key_no_ext, ".csc"))
      key_no_ext = key_no_ext.substr(0, key_no_ext.size() - 4);
    std::string key_no_ext_norm = normalize_path(key_no_ext);

    if (without_ext_norm.size() > key_no_ext_norm.size()) {
      char sep = without_ext_norm[without_ext_norm.size() -
                                  key_no_ext_norm.size() - 1];
      if (sep == '/' &&
          _stricmp(without_ext_norm.c_str() + without_ext_norm.size() -
                       key_no_ext_norm.size(),
                   key_no_ext_norm.c_str()) == 0)
        return reinterpret_cast<const uint8_t *>(rf->buffer);
    }
  }

  // Fall back to game's asset database (try .gsc, .csc, and without ext)
  std::string with_csc = without_ext + ".csc";
  for (const std::string &lookup : {with_ext, with_csc, without_ext}) {
    RawFile *asset = db_find_x_asset_header_hook.invoke<RawFile *>(
        XAssetType::SCRIPTPARSETREE, lookup.c_str(), false, 0);
    if (asset && asset->buffer)
      return reinterpret_cast<const uint8_t *>(asset->buffer);
  }

  return nullptr;
}

struct export_lookup_result {
  uint8_t *address = nullptr;
  bool script_loaded = false;
};

export_lookup_result
resolve_export_address_internal(const std::string &script_name,
                                uint32_t func_hash,
                                int32_t expected_params = -1) {
  export_lookup_result result{};

  const uint8_t *buffer = get_spt_buffer(script_name);
  result.script_loaded = (buffer != nullptr);
  if (!buffer) {
    return result;
  }

  uint64_t magic = 0;
  std::memcpy(&magic, buffer, sizeof(magic));
  if (magic != GSC_MAGIC) {
    return result;
  }

  uint32_t export_offset = 0;
  std::memcpy(&export_offset, buffer + 0x20, sizeof(export_offset));
  uint16_t export_count = 0;
  std::memcpy(&export_count, buffer + 0x3A, sizeof(export_count));

  const t7_export_entry *exports =
      reinterpret_cast<const t7_export_entry *>(buffer + export_offset);
  for (uint16_t i = 0; i < export_count; i++) {
    if (exports[i].func_name == func_hash) {
      if (expected_params >= 0 &&
          exports[i].num_params != static_cast<uint8_t>(expected_params)) {
        continue;
      }

      result.address = reinterpret_cast<uint8_t *>(
          reinterpret_cast<uintptr_t>(buffer) + exports[i].offset);
      if (expected_params >= 0) {
        break;
      }
    }
  }

  return result;
}

uint8_t *find_export_address_internal(const std::string &script_name,
                                      uint32_t func_hash,
                                      int32_t expected_params = -1) {
  return resolve_export_address_internal(script_name, func_hash,
                                         expected_params)
      .address;
}

void apply_pending_detours() {
  pending_detours.access([](auto &pending_detours) {
    std::vector<pending_detour> remaining_detours;
    remaining_detours.reserve(pending_detours.size());

    for (pending_detour &d : pending_detours) {
      const export_lookup_result target = resolve_export_address_internal(
          d.target_script, d.target_func_hash, d.target_params);
      const export_lookup_result replace = resolve_export_address_internal(
          d.replace_script, d.replace_func_hash, d.replace_params);

      if (target.address && replace.address) {
        gsc_funcs::add_detour(target.address, replace.address);
      } else {
        if (!target.script_loaded || !replace.script_loaded) {
          remaining_detours.push_back(d);
          continue;
        }

        const char *err = utils::string::va(
            "[gsc] detour bind failed %s::%s(%d) -> %s::%s(%d)",
            d.target_script.c_str(), d.target_func.c_str(), d.target_params,
            d.replace_script.c_str(), d.replace_func.c_str(), d.replace_params);
        fprintf(stderr, "%s\n", err);
        fflush(stderr);
        game::trace("%s", err);
      }
    }

    pending_detours = std::move(remaining_detours);
  });
}

void add_gdb(const std::string &name, std::vector<uint8_t> gdb) {
  const std::string gdb_name = name + ".gdb";
  while (script_gdbs.try_emplace_l(
      gdb_name, [&](auto &v) { v.second = std::move(gdb); })) {
  }
}

RawFile *get_loaded_script(const std::string &name) {
  RawFile *result = nullptr;
  loaded_scripts.if_contains(name, [&](const auto &v) { result = v.second; });
  return result;
}

void print_loading_script(const std::string &name) {
  const char *type = utils::string::ends_with(name, ".csc") ? "CSC" : "GSC";
  const char *log =
      utils::string::va("Loading %s script '%s'", type, name.data());
  printf("%s\n", log);
  game::trace("%s", log);
}

void load_script(const std::string &name, const std::string &data,
                 const bool load) {

  const bool is_csc = utils::string::ends_with(name, ".csc");
  const bool is_gsc = utils::string::ends_with(name, ".gsc");

  std::string base_name = name;
  if (!is_gsc && !is_csc) {
    const char *err = utils::string::va(
        "Script '%s' failed to load due to invalid suffix.", name.data());
    fprintf(stderr, "%s\n", err);
    fflush(stderr);
    game::trace("%s", err);
    return;
  }

  // Skip CSC on dedicated server (no client script instance)
  if (is_csc && is_server()) {
    return;
  }

  base_name = name.substr(0, name.size() - 4);
  if (base_name.empty()) {
    const char *err = utils::string::va(
        "Script '%s' failed to load due to invalid name.", name.data());
    fprintf(stderr, "%s\n", err);
    fflush(stderr);
    game::trace("%s", err);
    return;
  }

  RawFile *raw_file;
  {
    std::scoped_lock<std::mutex> alloc_lock(allocator_mutex);
    raw_file = allocator.allocate<RawFile>();
    raw_file->name = allocator.duplicate_string(name);
    raw_file->buffer =
        reinterpret_cast<uint8_t *>(allocator.duplicate_string(data));
  }
  raw_file->len = static_cast<int>(data.length());

  fixup_script_imports(const_cast<uint8_t *>(raw_file->buffer), raw_file->len);

  while (loaded_scripts.try_emplace_l(name,
                                      [&](auto &v) { v.second = raw_file; })) {
  }
  const char *log = utils::string::va("Loaded script '%s' (size %llu bytes)",
                                      name.data(), raw_file->len);
  fprintf(stdout, "%s\n", log);
  fflush(stdout);
  game::trace("%s", log);

  if (load) {
    const scriptInstance_t inst =
        is_csc ? SCRIPTINSTANCE_CLIENT : SCRIPTINSTANCE_SERVER;
    scr::Scr_LoadScript(inst, base_name.data());
  }
}

void load_script_file(std::string &data,
                      const std::filesystem::path &script_file,
                      const bool load) {
  const std::string script_file_str = script_file.generic_string();
  if (data.size() >= sizeof(GSC_MAGIC) &&
      !std::memcmp(data.data(), &GSC_MAGIC, sizeof(GSC_MAGIC))) {
    print_loading_script(script_file_str);
    load_script(script_file_str, data, load);
  } else if ((utils::string::ends_with(script_file_str, ".gsc") ||
              utils::string::ends_with(script_file_str, ".csc")) &&
             !data.empty()) {
    const bool is_csc = utils::string::ends_with(script_file_str, ".csc");
    const char *script_type = is_csc ? "CSC" : "GSC";

    std::string name = script_file_str;
    const std::string appdata_path =
        (get_appdata_path() / "data/").generic_string();
    const std::string host_path =
        (utils::nt::library{}.get_folder() / "boiii/").generic_string();

    size_t i = name.find(appdata_path);
    if (i != std::string::npos) {
      name.erase(0, i + appdata_path.length());
    }

    i = name.find(host_path);
    if (i != std::string::npos) {
      name.erase(0, i + host_path.length());
    }

    // Skip CSC on dedicated server
    if (is_csc && is_server())
      return;

    // Strip devblocks before compilation
    const std::string cleaned_source = strip_devblocks(data);

    const char *log = utils::string::va("Compiling %s script '%s'", script_type,
                                        name.c_str());
    fprintf(stdout, "%s\n", log);
    fflush(stdout);
    game::trace("%s", log);
    gsc_compiler::compile_result result =
        gsc_compiler::compile(cleaned_source, name);
    if (result.success) {

      // Store hash-to-name+line map from this compilation
      for (gsc::hash_name_pair &hn : result.hash_names) {
        // Will loop at most twice. Once if the value vector needs initialized
        // (entry did not already exist), and once to append this entry to the
        // vector
        while (script_hash_names.try_emplace_l(
            hn.hash, [&](decltype(script_hash_names)::value_type &v) {
              v.second.push_back({hn.name, hn.line, hn.params});
            })) {
        }
      }

      // Store original source text for this file
      while (script_sources.try_emplace_l(
          name, [&](auto &v) { v.second = std::move(cleaned_source); })) {
      }

#ifndef NDEBUG
      // Dump compiled bytecode to file for debugging
      // ".gsc" -> ".gscc", ".csc" -> ".cscc"
      const std::filesystem::path bytecode_out_path = script_file_str + "c";

      utils::io::write_file_bytes(bytecode_out_path, result.bytecode.data(),
                                  result.bytecode.size(), false);

      const std::filesystem::path gdb_out_path = script_file_str + ".gdb";
      utils::io::write_file_bytes(gdb_out_path, result.gdb.data(),
                                  result.gdb.size(), false);

#endif

      print_loading_script(name);
      std::string bytecode(result.bytecode.begin(), result.bytecode.end());
      load_script(name, bytecode, load);
      add_gdb(name, result.gdb);
      objFileInfo_t *obj =
          get_obj_by_name(scriptInstance_t::SCRIPTINSTANCE_SERVER, name);
      if (obj) {
        script_sources.modify_if(name, [&](auto &v) {
          char *src = v.second.data();
          obj->debugInfo.source = src;
          obj->debugInfo.sourceLen = v.second.size();
          for (size_t i = 0; i < v.second.size(); ++i) {
            char *c = &src[i];
            if (*c == '\n' || *c == '\r') {
              *c = '\0';
            }
          }
        });
      }

      // Register replacefunc entries as pending detours
      if (!result.replacefuncs.empty()) {
        std::string replace_base = name;
        if (utils::string::ends_with(replace_base, ".gsc") ||
            utils::string::ends_with(replace_base, ".csc"))
          replace_base = replace_base.substr(0, replace_base.size() - 4);

        for (gsc_compiler::replacefunc_entry &rf : result.replacefuncs) {
          const std::string replace_script =
              rf.replace_script.empty() ? replace_base : rf.replace_script;
          pending_detours.access([&](auto &pending_detours) {
            pending_detours.push_back(
                {rf.target_script, rf.target_func,
                 gsc::gsc_hash(rf.target_func), rf.target_params,
                 replace_script, rf.replace_func,
                 gsc::gsc_hash(rf.replace_func), rf.replace_params});
          });
        }
      }
    } else {
      const std::function<std::string(const std::string &src, int32_t line_num)>
          get_source_line =
              [](const std::string &src, int32_t line_num) -> std::string {
        if (line_num <= 0)
          return "";
        int32_t current = 1;
        size_t start = 0;
        while (current < line_num && start < src.size()) {
          if (src[start] == '\n')
            current++;
          start++;
        }
        if (current != line_num)
          return "";
        size_t end = src.find('\n', start);
        if (end == std::string::npos)
          end = src.size();
        std::string line = src.substr(start, end - start);
        if (!line.empty() && line.back() == '\r')
          line.pop_back();
        return line;
      };

      const char *err_header = utils::string::va(
          "^1*********************%s COMPILE ERROR*********************",
          script_type);
      fprintf(stderr, "%s\n", err_header);
      fflush(stderr);
      game::trace("%s", err_header);
      for (const auto &err : result.errors) {
        const char *file_log =
            utils::string::va("^1  File:    ^5%s", err.file.data());
        fprintf(stderr, "%s\n", file_log);
        fflush(stderr);
        game::trace("%s", file_log);
        if (err.line > 0) {
          const char *line_column_log = utils::string::va(
              "^1  Line:    ^2%d^7, ^1Column: ^2%d", err.line, err.column);
          fprintf(stderr, "%s\n", line_column_log);
          fflush(stderr);
          game::trace("%s", line_column_log);
          std::string src_line = get_source_line(data, err.line);
          if (!src_line.empty()) {
            const char *src_log =
                utils::string::va("^1  Source:  ^7%s", src_line.data());
            fprintf(stderr, "%s\n", src_log);
            fflush(stderr);
            game::trace("%s", src_log);
          }
        }
        const char *error_log =
            utils::string::va("^1  Error:   ^1%s", err.message.data());
        fprintf(stderr, "%s\n", error_log);
        fflush(stderr);
        game::trace("%s", error_log);
        const char *footer_p1_log = utils::string::va(
            "^1------------------------------------------------------------");
        fprintf(stderr, "%s\n", footer_p1_log);
        fflush(stderr);
        game::trace("%s", footer_p1_log);
      }
      const char *footer_p2_log = utils::string::va(
          "^1************************************************************");
      fprintf(stderr, "%s\n", footer_p2_log);
      fflush(stderr);
      game::trace("%s", footer_p2_log);
    }
  }
}

void load_scripts_directory(const std::string &script_dir, const bool load,
                            const bool recurse) {
  if (utils::io::directory_exists(script_dir)) {
    std::vector<std::filesystem::path> scripts =
        utils::io::list_files(script_dir, recurse, false);

    const auto load_dir_file_cb = [load](const std::filesystem::path &script) {
      std::string data;
      if (!std::filesystem::is_directory(script) &&
          utils::io::read_file(script, &data)) {

        load_script_file(data, script, load);
      }
    };
    if (load) {
      std::for_each(std::execution::seq, scripts.begin(), scripts.end(),
                    load_dir_file_cb);
    } else {
      std::for_each(std::execution::par, scripts.begin(), scripts.end(),
                    load_dir_file_cb);
    }
  }
}
} // namespace

std::optional<std::filesystem::path> get_game_type_specific_folder() {
  switch (com::Com_SessionMode_GetMode()) {
  case eModes::MULTIPLAYER:
    return "mp";
  case eModes::ZOMBIES:
    return "zm";
  case eModes::CAMPAIGN:
    return "cp";
  default:
    return {};
  }
}

std::optional<std::filesystem::path> get_map_specific_folder() {
  const std::string_view mapname = get_mapname().value_or("");
  if (mapname.empty()) {
    return {};
  }

  return mapname;
}

constexpr const std::string_view gametype_prefixes[] = {"zm", "mp", "cp"};
bool is_shared_tree_dir(const std::filesystem::path &dir) {
  const std::filesystem::path dirname = dir.filename();
  const std::string dirname_str = dirname.generic_string();
  for (const std::string_view prefix : gametype_prefixes) {
    if (prefix == dirname_str ||
        (dirname_str.size() > prefix.size() &&
         utils::string::starts_with(prefix, dirname_str) &&
         dirname_str[2] == '_') /* map override script tree */) {
      return false;
    }
  }
  return true;
}

std::vector<std::filesystem::path>
shared_tree_directories(const std::filesystem::path &tree) {
  std::vector<std::filesystem::path> root_entries =
      utils::io::list_files(tree, false, true);
  // Iterate from end to start so as not to invalidate subsequently iterated
  // indices upon removing entry
  for (size_t i = root_entries.size(); i > 0; --i) {
    const size_t idx = i - 1;
    if (!is_shared_tree_dir(root_entries[idx])) {
      root_entries.erase(root_entries.begin() + idx);
    }
  }
  return root_entries;
}

void load_tree(std::filesystem::path tree, bool execImmediate = false) {
  const utils::nt::library host{};

  const std::filesystem::path data_folder = get_appdata_path() / "data";
  const std::filesystem::path boiii_folder = host.get_folder() / "boiii";

  const auto load = [&data_folder,
                     &boiii_folder](const std::filesystem::path &folder,
                                    const bool load, const bool recurse) {
    load_scripts_directory((data_folder / folder).string(), load, recurse);
    load_scripts_directory((boiii_folder / folder).string(), load, recurse);
  };

  std::vector<std::filesystem::path> applicable_tree_dirs =
      shared_tree_directories(tree);
  const std::optional<std::filesystem::path> game_type =
      get_game_type_specific_folder();
  if (game_type.has_value()) {
    applicable_tree_dirs.push_back(tree / game_type.value());
  }

  const std::optional<std::filesystem::path> map_name =
      get_map_specific_folder();
  if (map_name.has_value()) {
    applicable_tree_dirs.push_back(tree / map_name.value());
  }

  /*
    First, compile and load each script into our lookup table.
    We must do this before loading any scripts into the VM to ensure all
    dependencies are available for lookup upon first script load.
  */
  std::for_each(
      std::execution::par, applicable_tree_dirs.begin(),
      applicable_tree_dirs.end(),
      [load](const std::filesystem::path &path) { load(path, false, true); });
  load(tree, false, false);

  if (execImmediate) {
    // Now, load the scripts into the VM.
    std::for_each(
        std::execution::seq, applicable_tree_dirs.begin(),
        applicable_tree_dirs.end(),
        [load](const std::filesystem::path &path) { load(path, true, true); });
    load(tree, true, false);
  }
}

void load_scripts() {
  // scripts tree is for overriding stock scripts the game uses
  load_tree("scripts", false);
  // Custom scripts is for new scripts we must execute
  load_tree("custom_scripts", true);
}

RawFile *get_loaded_map_script(const char *name) {
  // "scripts/${mapname}/${scripts_sub_path}"
  const std::optional<std::string_view> mapname = get_mapname();
  if (mapname.has_value() && !mapname.value().empty()) {
    const std::string_view search_name = name;
    // Replace "scripts/" tree name with "scripts/${mapname}/"
    size_t first_sep = search_name.find('/');
    if (first_sep != std::string::npos) {
      const std::string_view tree = search_name.substr(0, first_sep + 1);
      if (tree == "scripts/") {
        const std::string override_path =
            std::string(tree) /* "scripts/" */ + std::string(mapname.value()) +
            std::string(search_name.substr(
                first_sep)) /* "/" + relative path under "scripts" tree */;
        return get_loaded_script(override_path);
      }
    }
  }

  return nullptr;
}

XAssetHeader DB_FindXAssetHeader_TryOverride(const XAssetType type,
                                             const char *name,
                                             const bool error_if_missing,
                                             const int32_t wait_time) {
  XAssetHeader result{.rawfile = nullptr};
  // Check our loaded scripts FIRST to avoid "Could not find scriptparsetree"
  // spam
  if (name && name[0] && type == XAssetType::SCRIPTPARSETREE) {
    result.rawfile = get_loaded_script(name);

    // Try to get map-specific script override
    if (result.rawfile == nullptr) {
      result.rawfile = get_loaded_map_script(name);
    }
  }

  if (result.rawfile == nullptr) {
    result = db_find_x_asset_header_hook.invoke<XAssetHeader>(
        type, name, error_if_missing, wait_time);
  }
  if (nonnull(result.rawfile)) {
    dump::dump_requested_assets(type, name, result);
  }

  return result;
}

static std::mutex script_load_lock;

void clear_script_memory() {
  std::scoped_lock lock(script_load_lock);

  loaded_scripts.clear();
  script_gdbs.clear();
  script_hash_names.clear();
  script_sources.clear();
  pending_detours.access(
      [](auto &pending_detours) { pending_detours.clear(); });
  allocator.clear();
}

void rebuild_script_gdb() {
  const std::filesystem::path scriptgdb_archive_path =
      get_game_path() / "zone" / "scriptgdb.zip";

  if (std::filesystem::exists(scriptgdb_archive_path)) {
    const std::string data = utils::io::read_file(scriptgdb_archive_path);
    try {
      const std::unordered_map<std::string, std::vector<uint8_t>>
          existing_entries = utils::compression::zip::extract(data);
      for (const auto &[key, value] : existing_entries) {
        if (!script_gdbs.contains(key)) {
          script_gdbs[key] = std::move(value);
        }
      }
    } catch (...) {
    }
    std::filesystem::remove(scriptgdb_archive_path);
  }

  const std::vector<std::pair<std::string, std::vector<uint8_t>>> gdb_pairs{
      script_gdbs.begin(), script_gdbs.end()};
  utils::compression::zip::write_file(scriptgdb_archive_path, gdb_pairs);
}

int server_script_checksum_stub() { return 1; }

void gscr_getbgbtokensremaining_stub(scriptInstance_t inst,
                                     [[maybe_unused]] scr_entref_t entref) {
  scr::Scr_AddInt(inst, 255);
}

// Global hash→name lookup table loaded from data/lookup_tables/hash_names.txt
static std::unordered_map<uint32_t, std::string> global_hash_table;
static std::once_flag hash_table_load_flag;

void load_global_hash_table() {
  std::call_once(hash_table_load_flag, [] {
    const auto try_load = [](const std::filesystem::path &path) {
      std::string data;
      if (!utils::io::read_file(path.string(), &data))
        return false;
      size_t count = 0;
      std::istringstream stream(data);
      std::string line;
      while (std::getline(stream, line)) {
        if (line.empty())
          continue;
        size_t space = line.find(' ');
        if (space == std::string::npos)
          continue;
        uint32_t hash = static_cast<uint32_t>(
            std::strtoul(line.substr(0, space).c_str(), nullptr, 16));
        if (hash != 0)
          global_hash_table[hash] = line.substr(space + 1);
        count++;
      }
      const char *log = utils::string::va("Loaded %zu hash names from '%s'",
                                          count, path.string().c_str());
      printf("%s\n", log);
      game::trace("%s", log);
      return true;
    };

    // Try appdata path first, then exe-relative path
    const std::filesystem::path appdata =
        get_appdata_path() / "data" / "lookup_tables" / "hash_names.txt";
    if (!try_load(appdata)) {
      const std::filesystem::path host = utils::nt::library{}.get_folder() /
                                         "boiii" / "data" / "lookup_tables" /
                                         "hash_names.txt";
      try_load(host);
    }
  });
}

void begin_load_scripts_stub(scriptInstance_t inst, int32_t user) {
  std::scoped_lock lock(script_load_lock);
  load_global_hash_table();

  scr::Scr_BeginLoadScripts(inst, user);

  if (com::Com_IsInGame() && !com::Com_IsRunningUILevel()) {
    load_scripts();

    if (!pending_detours.access<bool>(
            [](auto &pending_detours) { return pending_detours.empty(); })) {
      apply_pending_detours();
    }

    rebuild_script_gdb();
  }
}

std::string resolve_hash(uint32_t hash) {

  std::optional<std::string> result;
  script_hash_names.if_contains(hash, [&result](const auto &v) {
    if (!v.second.empty()) {
      result = v.second[0].name;
    }
  });

  // Fallback: global hash table from data file
  if (global_hash_table.contains(hash))
    result = global_hash_table[hash];

  return result.value_or("");
}

uint8_t *find_export_address(const std::string &script_name,
                             const std::string &func_name,
                             int32_t expected_params) {
  return find_export_address_internal(script_name, gsc::gsc_hash(func_name),
                                      expected_params);
}

int resolve_hash_line(uint32_t hash, int32_t num_params) {
  int32_t result = 0;
  script_hash_names.if_contains(hash, [&result, num_params](const auto &v) {
    for (const hash_info &entry : v.second) {
      if (entry.params == static_cast<uint8_t>(num_params) && entry.line > 0) {
        result = entry.line;
        break;
      }
    }
  });
  return result;
}

std::string get_source_line(const std::string &file, int32_t line_num) {
  std::optional<std::string> result = std::nullopt;
  // Try to find source by matching file path suffix
  script_sources.for_each([&result, file, line_num](const auto &v) {
    if (!result.has_value()) {
      const std::string path = v.first;
      const std::string src = v.second;
      if (file.find(path) != std::string::npos ||
          path.find(file) != std::string::npos ||
          file.find(std::filesystem::path(path).filename().string()) !=
              std::string::npos) {
        if (line_num > 0) {
          int32_t current = 1;
          size_t start = 0;
          while (current < line_num && start < src.size()) {
            if (src[start] == '\n')
              current++;
            start++;
          }
          if (current == line_num) {
            size_t end = src.find('\n', start);
            if (end == std::string::npos) {
              end = src.size();
            }
            std::string line = src.substr(start, end - start);
            if (!line.empty() && line.back() == '\r') {
              line.pop_back();
            }
            result = std::optional(line);
          }
        }
      }
    }
  });
  return result.value_or("");
}

const char *Scr_PrevCodePos(scriptInstance_t inst, volatile uint8_t *codePos) {
  char *filename = nullptr;
  int32_t lineNum = 0;
  char *sourceLine = nullptr;

  if (codePos != nullptr &&
      codePos != reinterpret_cast<uint8_t *>(vm::g_endPos.get())) {
    Scr_GetFileAndLineNum_Impl(inst, const_cast<uint8_t *>(codePos) - 1,
                               const_cast<const char **>(&filename), &lineNum,
                               const_cast<const char **>(&sourceLine));
    if (lineNum < 0) {
      return utils::string::va("\tfile '%s' - missing line information\n",
                               filename);
    } else {
      if (sourceLine) {
        for (; *sourceLine == ' ' || *sourceLine == '\t'; ++sourceLine) {
        }
      }
      return utils::string::va("\tfile '%s', line %d :: %s\n", filename,
                               lineNum + 1, sourceLine ? sourceLine : "");
    }
  }

  return "Missing file and line information - not currently executing script "
         "bytecode or "
         "reached end of executed script bytecode.";
}

utils::hook::detour Scr_Error_hook;
void Scr_Error_LogAll(scriptInstance_t inst, const char *error, bool terminal) {
  void *callerAddr = _ReturnAddress();
  if (is_server()) {
    sv_detailedScriptErrors->set(true);
  }
  if (terminal) {
    vm::gScrVarPub->instance[inst].developer = true;
    vm::gScrVmPub->instance[inst].debugCode = true;
  }

  std::string prevCodePositionsString = "";
  for (int32_t stackIdx = 0;
       stackIdx < vm::gScrVmPub->instance[inst].function_count - 1;
       ++stackIdx) {
    prevCodePositionsString += std::format("[{}] ", stackIdx);
    prevCodePositionsString += Scr_PrevCodePos(
        inst,
        vm::gScrVmPub->instance[inst].function_frame_start[stackIdx].fs.pos);
    prevCodePositionsString += "\n";
  }

  const uint8_t final_stack_idx =
      vm::gScrVmPub->instance[inst].function_count - 1;
  prevCodePositionsString += std::format("[{}] ", final_stack_idx);
  prevCodePositionsString +=
      Scr_PrevCodePos(inst, vm::gScrVmPub->instance[inst]
                                .function_frame_start[final_stack_idx]
                                .fs.pos);

#ifndef NDEBUG
  const vm::function_frame_t *current_frame =
      &vm::gScrVmPub->instance[inst].function_frame_start[0];
#endif
  const char *error_log = utils::string::va(
      "Scr_Error called from 0x%p with inst: %s, "
#ifndef NDEBUG
      "gScrVmPub->instance[inst].function_frame_start[0].fs.pos: 0x%p, "
      "gScrVmPub->instance[inst].function_frame_start[0].fs.top: 0x%p, "
      "gScrVmPub->instance[inst].function_frame_start[0].fs.startTop: 0x%p, "
      "gScrVmPub->instance[inst].function_frame_start[0].fs.threadId: 0x%08X, "
      "gScrVmPub->instance[inst].function_frame_start[0].fs.localVarCount: %lu,"
      "gFs.pos: 0x%p, "
      "gFs.top: 0x%p, gFs.startTop: 0x%p, gFs.threadId: 0x%08X, "
      "gFs.localVarCount: %lu, "
#endif
      "error: \"%s\", terminal: "
      "%s\nCallstack:\n%s",
      derelocate(callerAddr), serialize(inst),
#ifndef NDEBUG
      current_frame->fs.pos, current_frame->fs.top, current_frame->fs.startTop,
      current_frame->fs.threadId, current_frame->fs.localVarCount,
      vm::gFs->instance[inst].pos, vm::gFs->instance[inst].top,
      vm::gFs->instance[inst].startTop, vm::gFs->instance[inst].threadId,
      vm::gFs->instance[inst].localVarCount,
#endif
      error ? error : "NULL", terminal ? "true" : "false",
      prevCodePositionsString.c_str());

  fprintf(stderr, "%s\n", error_log);
  fflush(stderr);
#ifndef NDEBUG
  trace("%s", error_log);
#endif

  return Scr_Error_hook.invoke(inst, error, terminal);
}

utils::hook::detour Hunk_UserFree_hook;
void Hunk_UserFree_NotScriptPoolAlloc(hunk::HunkUser *user, void *ptr) {
  bool should_skip = false;
  loaded_scripts.for_each([&should_skip, ptr](const auto &v) {
    if (!should_skip && contains(v.second, sizeof(RawFile), ptr)) {
      should_skip = true;
    }
  });
  if (should_skip) {
    return;
  }

  script_gdbs.for_each([&should_skip, ptr](const auto &v) {
    if (!should_skip && contains(v.second.data(), v.second.size(), ptr)) {
      should_skip = true;
    }
  });

  if (should_skip) {
    return;
  }

  script_sources.for_each([&should_skip, ptr](const auto &v) {
    if (!should_skip && contains(v.second.data(), v.second.size(), ptr)) {
      should_skip = true;
    }
  });

  if (should_skip) {
    return;
  }

  return Hunk_UserFree_hook.invoke(user, ptr);
}

utils::hook::detour LoadScriptGDB2_hook;
utils::hook::detour LoadScriptGDB_hook;
utils::hook::detour Scr_FindObjFileInfo_hook;
utils::hook::detour Scr_GetFileAndLineNum_hook;
struct component final : generic_component {
  void post_unpack() override {
    // Return custom or overrided scripts if found
    db_find_x_asset_header_hook.create(DB_FindXAssetHeader.get(),
                                       DB_FindXAssetHeader_TryOverride);

    // Free our scripts when the game ends
    game_event::on_g_shutdown_game(clear_script_memory);

    // Load our custom/overriding scripts
    utils::hook::call(select(0x141AAE92F, 0x1402D81FF),
                      begin_load_scripts_stub);

    // Force GSC checksums to be valid
    utils::hook::call(select(0x1408F2E5D, 0x1400E2D22),
                      server_script_checksum_stub);

    // Workaround for "Out of X" gobblegum
    gscr_get_bgb_tokens_remaining_hook.create(gscr::GScr_GetBGBTokensRemaining,
                                              gscr_getbgbtokensremaining_stub);

    if (utils::flags::has_flag("log-script-errors")) {
      // Log all script errors, even when non-fatal and/or `developer` is
      // disabled
      Scr_Error_hook.create(Scr_Error, Scr_Error_LogAll);
    }

    if (is_server()) {
      Hunk_UserFree_hook.create(hunk::Hunk_UserFree,
                                Hunk_UserFree_NotScriptPoolAlloc);

      LoadScriptGDB_hook.create(LoadScriptGDB, LoadScriptGDB_Impl);
      LoadScriptGDB2_hook.create(LoadScriptGDB2, LoadScriptGDB2_Impl);
    }

    Scr_FindObjFileInfo_hook.create(Scr_FindObjFileInfo,
                                    Scr_FindObjFileInfo_Impl);
    Scr_GetFileAndLineNum_hook.create(Scr_GetFileAndLineNum,
                                      Scr_GetFileAndLineNum_Impl);
  }
};
} // namespace script

REGISTER_COMPONENT(script::component)