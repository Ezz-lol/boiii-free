#include "../std_include.hpp"
#include "loader/component_loader.hpp"
#include "../game/game.hpp"
#include "../game/utils.hpp"
#include "../game/impl/scr/scr.hpp"

#include "game_event.hpp"
#include "gsc/gsc_compiler.hpp"
#include "scheduler.hpp"

#include <unordered_map>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>
using namespace game::db::xasset;
using namespace game::scr;

namespace gsc_funcs {
void add_detour(uint8_t *target_addr, uint8_t *replacement_addr);
}

namespace script {
namespace {
constexpr size_t GSC_MAGIC = 0x1C000A0D43534780;

utils::hook::detour db_find_x_asset_header_hook;
utils::hook::detour gscr_get_bgb_tokens_remaining_hook;

utils::memory::allocator allocator;
std::unordered_map<std::string, RawFile *> loaded_scripts;
static constexpr std::array<std::string_view, 5> SCR_HASH_LITERAL_PREFIXES = {
    "hash", "id", "function", "var", "namespace"};

bool is_hash_literal_prefix(const std::string &s) {
  for (uint32_t i = 0; i < SCR_HASH_LITERAL_PREFIXES.size(); i++) {
    if (s == SCR_HASH_LITERAL_PREFIXES[i]) {
      return true;
    }
  }

  return false;
}

bool try_parse_raw_hash(const std::string &input, uint32_t &out) {

  if (input.size() > 0) {
    std::string inputSubstr = input;
    if (inputSubstr[0] == '_') {
      inputSubstr = inputSubstr.substr(1);
    }
    const size_t underscoreIdx = inputSubstr.find('_');
    if (underscoreIdx != std::string::npos &&
        underscoreIdx < inputSubstr.size()) {

      const std::string prefix = inputSubstr.substr(0, underscoreIdx);
      if (is_hash_literal_prefix(prefix)) {

        const std::string hex_part = inputSubstr.substr(underscoreIdx + 1);
        if (hex_part.size() == 8) {

          for (char c : hex_part) {
            if (!std::isxdigit(static_cast<unsigned char>(c)))
              return false;
          }

          out = static_cast<uint32_t>(std::stoul(hex_part, nullptr, 16));
          return out != 0;
        }
      }
    }
  }

  return false;
}

uint32_t gsc_hash(const std::string &str) {
  uint32_t raw = 0;
  if (try_parse_raw_hash(str, raw))
    return raw;

  uint32_t h = 0x4B9ACE2F;
  for (char c : str)
    h = (static_cast<uint32_t>(std::tolower(static_cast<unsigned char>(c))) ^
         h) *
        0x1000193;
  h *= 0x1000193;
  return h;
}

std::string normalize_script_name(std::string script_name) {
  auto start = script_name.find('<');
  auto end = script_name.find('>');
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
std::vector<pending_detour> pending_detours;

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

void fixup_script_imports(char *buf, int32_t len) {
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

    std::string inc_path(buf + str_off);
    // Normalize include path (forward slashes, lowercase) before hashing
    for (char &c : inc_path) {
      if (c == '\\')
        c = '/';
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    uint32_t path_hash = gsc_hash(inc_path);

    // Look up the actual game SPT for this include path (try .gsc then .csc)
    auto *asset = db_find_x_asset_header_hook.invoke<RawFile *>(
        XAssetType::SCRIPTPARSETREE, (inc_path + ".gsc").c_str(), false, 0);
    if (!asset || !asset->buffer)
      asset = db_find_x_asset_header_hook.invoke<RawFile *>(
          XAssetType::SCRIPTPARSETREE, (inc_path + ".csc").c_str(), false, 0);

    if (!asset || !asset->buffer)
      continue;

    auto *spt = reinterpret_cast<const uint8_t *>(asset->buffer);
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
  std::vector<pending_detour> remaining_detours;
  remaining_detours.reserve(pending_detours.size());

  for (pending_detour &d : pending_detours) {
    const auto target = resolve_export_address_internal(
        d.target_script, d.target_func_hash, d.target_params);
    const auto replace = resolve_export_address_internal(
        d.replace_script, d.replace_func_hash, d.replace_params);

    if (target.address && replace.address) {
      gsc_funcs::add_detour(target.address, replace.address);
    } else {
      if (!target.script_loaded || !replace.script_loaded) {
        remaining_detours.push_back(d);
        continue;
      }

      printf("[gsc] detour bind failed %s::%s(%d) -> %s::%s(%d)\n",
             d.target_script.c_str(), d.target_func.c_str(), d.target_params,
             d.replace_script.c_str(), d.replace_func.c_str(),
             d.replace_params);
    }
  }

  pending_detours = std::move(remaining_detours);
}

struct hash_info {
  std::string name;
  int32_t line;
  uint8_t params;
};
std::unordered_map<uint32_t, std::vector<hash_info>> script_hash_names;

std::unordered_map<std::string, std::string> script_sources;

RawFile *get_loaded_script(const std::string &name) {
  const auto itr = loaded_scripts.find(name);
  return (itr == loaded_scripts.end()) ? nullptr : itr->second;
}

void print_loading_script(const std::string &name) {
  const char *type = utils::string::ends_with(name, ".csc") ? "CSC" : "GSC";
  printf("Loading %s script '%s'\n", type, name.data());
}

void load_script(const std::string &input_name, const std::string &data,
                 const bool load) {

  std::string name = std::string(input_name);
  const std::string appdata_path =
      (game::get_appdata_path() / "data/").generic_string();
  const std::string host_path =
      (utils::nt::library{}.get_folder() / "boiii/").generic_string();

  size_t i = name.find(appdata_path);
  if (i != std::string::npos) {
    name.erase(i, appdata_path.length());
  }

  i = name.find(host_path);
  if (i != std::string::npos) {
    name.erase(i, host_path.length());
  }

  const bool is_csc = utils::string::ends_with(name, ".csc");
  const bool is_gsc = utils::string::ends_with(name, ".gsc");

  std::string base_name = name;
  if (!is_gsc && !is_csc) {
    printf("Script '%s' failed to load due to invalid suffix.\n", name.data());
    return;
  }

  // Skip CSC on dedicated server (no client script instance)
  if (is_csc && game::is_server()) {
    return;
  }

  base_name = name.substr(0, name.size() - 4);
  if (base_name.empty()) {
    printf("Script '%s' failed to load due to invalid name.\n", name.data());
    return;
  }

  RawFile *raw_file = allocator.allocate<RawFile>();
  raw_file->name = allocator.duplicate_string(name);
  raw_file->buffer = allocator.duplicate_string(data);
  raw_file->len = static_cast<int>(data.length());

  fixup_script_imports(const_cast<char *>(raw_file->buffer), raw_file->len);

  loaded_scripts[name] = raw_file;
  printf("Loaded script '%s' (size %llu bytes)\n", name.data(), raw_file->len);

  if (load) {
    const scriptInstance_t inst =
        is_csc ? SCRIPTINSTANCE_CLIENT : SCRIPTINSTANCE_SERVER;
    game::scr::Scr_LoadScript(inst, base_name.data());
  }
}

void load_script_file(std::string &data, const std::string &script_file,
                      const bool load) {
  if (data.size() >= sizeof(GSC_MAGIC) &&
      !std::memcmp(data.data(), &GSC_MAGIC, sizeof(GSC_MAGIC))) {
    print_loading_script(script_file);
    load_script(script_file, data, load);
  } else if ((utils::string::ends_with(script_file, ".gsc") ||
              utils::string::ends_with(script_file, ".csc")) &&
             !data.empty()) {
    const bool is_csc = utils::string::ends_with(script_file, ".csc");
    const char *script_type = is_csc ? "CSC" : "GSC";

    // Skip CSC on dedicated server
    if (is_csc && game::is_server())
      return;

    // Strip devblocks before compilation
    std::string cleaned_source = strip_devblocks(data);

    printf("Compiling %s script '%s'\n", script_type, script_file.data());
    gsc_compiler::compile_result result =
        gsc_compiler::compile(cleaned_source, script_file);
    if (result.success) {
      std::string bytecode(result.bytecode.begin(), result.bytecode.end());

      // Store hash-to-name+line map from this compilation
      for (gsc_compiler::hash_name_pair &hn : result.hash_names) {
        script_hash_names[hn.hash].push_back({hn.name, hn.line, hn.params});
      }

      // Store original source text for this file
      script_sources[script_file] = data;

      print_loading_script(script_file);
      load_script(script_file, bytecode, load);

      // Register replacefunc entries as pending detours
      if (!result.replacefuncs.empty()) {
        std::string replace_base = script_file;
        if (utils::string::ends_with(replace_base, ".gsc") ||
            utils::string::ends_with(replace_base, ".csc"))
          replace_base = replace_base.substr(0, replace_base.size() - 4);

        for (gsc_compiler::replacefunc_entry &rf : result.replacefuncs) {
          const std::string replace_script =
              rf.replace_script.empty() ? replace_base : rf.replace_script;
          pending_detours.push_back(
              {rf.target_script, rf.target_func, gsc_hash(rf.target_func),
               rf.target_params, replace_script, rf.replace_func,
               gsc_hash(rf.replace_func), rf.replace_params});
        }
      }
    } else {
      auto get_source_line = [](const std::string &src,
                                int32_t line_num) -> std::string {
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

      printf("^1*********************%s COMPILE ERROR*********************\n",
             script_type);
      for (const auto &err : result.errors) {
        printf("^1  File:    ^5%s\n", err.file.data());
        if (err.line > 0) {
          printf("^1  Line:    ^2%d^7, ^1Column: ^2%d\n", err.line, err.column);
          std::string src_line = get_source_line(data, err.line);
          if (!src_line.empty()) {
            printf("^1  Source:  ^7%s\n", src_line.data());
          }
        }
        printf("^1  Error:   ^1%s\n", err.message.data());
        printf("^1---------------------------------------------------------"
               "---\n");
      }
      printf("^1***********************************************************"
             "*\n");
    }
  }
}

void load_scripts_folder(const std::string &script_dir, const bool load,
                         const bool recurse) {
  if (!utils::io::directory_exists(script_dir)) {
    return;
  }

  std::vector<std::filesystem::path> scripts =
      utils::io::list_files(script_dir, recurse, false);
  for (const auto &script : scripts) {
    std::string data;
    std::string script_path_str = script.generic_string();
    if (!std::filesystem::is_directory(script) &&
        utils::io::read_file(script_path_str, &data)) {

      load_script_file(data, script_path_str, load);
    }
  }
}

std::optional<std::filesystem::path> get_game_type_specific_folder() {
  switch (game::com::Com_SessionMode_GetMode()) {
  case game::eModes::MULTIPLAYER:
    return "mp";
  case game::eModes::ZOMBIES:
    return "zm";
  case game::eModes::CAMPAIGN:
    return "cp";
  default:
    return {};
  }
}

std::optional<std::filesystem::path> get_map_specific_folder() {
  const std::string_view mapname = game::get_mapname().value_or("");
  if (mapname.empty()) {
    return {};
  }

  return mapname;
}

void load_scripts() {
  const utils::nt::library host{};

  const std::filesystem::path data_folder = game::get_appdata_path() / "data";
  const std::filesystem::path boiii_folder = host.get_folder() / "boiii";

  const auto load = [&data_folder,
                     &boiii_folder](const std::filesystem::path &folder,
                                    const bool load, const bool recurse) {
    load_scripts_folder((data_folder / folder).string(), load, recurse);
    load_scripts_folder((boiii_folder / folder).string(), load, recurse);
  };

  // scripts folder is for overriding stock scripts the game uses
  load("scripts", false, true);

  std::vector<std::filesystem::path> applicable_custom_script_paths = {
      "custom_scripts/shared", "custom_scripts/core",
      "custom_scripts/codescripts"};
  const std::optional<std::filesystem::path> game_type =
      get_game_type_specific_folder();
  if (game_type.has_value()) {
    applicable_custom_script_paths.push_back("custom_scripts" /
                                             game_type.value());
  }

  const std::optional<std::filesystem::path> map_name =
      get_map_specific_folder();
  if (map_name.has_value()) {
    applicable_custom_script_paths.push_back("custom_scripts" /
                                             map_name.value());
  }

  /*
    First, compile and load each script into our lookup table.
    We must do this before loading any scripts into the VM to ensure all
    dependencies are available for lookup upon first custom script load.
  */
  for (const std::filesystem::path &path : applicable_custom_script_paths) {
    load(path, false, true);
  }
  load("custom_scripts", false, false);

  // Now, load the custom scripts into the VM.
  for (const std::filesystem::path &path : applicable_custom_script_paths) {
    load(path, true, true);
  }
  load("custom_scripts", true, false);
}

RawFile *get_loaded_map_script(const char *name) {
  // "scripts/${mapname}/${scripts_sub_path}"
  const std::optional<std::string_view> mapname = game::get_mapname();
  if (mapname.has_value() && !mapname.value().empty()) {
    const std::string_view search_name = name;
    // Replace "scripts" prefix with "scripts/${mapname}"
    size_t first_sep = search_name.find('/');
    if (first_sep != std::string::npos) {
      const std::string_view prefix = search_name.substr(0, first_sep + 1);
      if (prefix == "scripts/") {
        const std::string override_path =
            std::string(prefix) /* "scripts/" */ +
            std::string(mapname.value()) +
            std::string(search_name.substr(
                first_sep)) /* relative path under "scripts" tree */;
        return get_loaded_script(override_path);
      }
    }
  }

  return nullptr;
}

XAssetHeader db_find_x_asset_header_stub(const XAssetType type,
                                         const char *name,
                                         const bool error_if_missing,
                                         const int32_t wait_time) {
  // Check our loaded scripts FIRST to avoid "Could not find scriptparsetree"
  // spam
  if (type == XAssetType::SCRIPTPARSETREE) {
    RawFile *script = get_loaded_script(name);
    if (script != nullptr) {
      return static_cast<XAssetHeader>(script);
    }

    // Try to get map-specific script override
    script = get_loaded_map_script(name);
    if (script != nullptr) {
      return static_cast<XAssetHeader>(script);
    }
  }

  return db_find_x_asset_header_hook.invoke<XAssetHeader>(
      type, name, error_if_missing, wait_time);
}

static std::mutex script_load_lock;

void clear_script_memory() {
  std::lock_guard lock(script_load_lock);

  loaded_scripts.clear();
  script_hash_names.clear();
  script_sources.clear();
  pending_detours.clear();
  allocator.clear();
}

void begin_load_scripts_stub(scriptInstance_t inst, int32_t user) {
  std::lock_guard lock(script_load_lock);

  game::scr::Scr_BeginLoadScripts(inst, user);

  if (game::com::Com_IsInGame() && !game::com::Com_IsRunningUILevel()) {
    load_scripts();

    if (!pending_detours.empty())
      apply_pending_detours();
  }
}

int server_script_checksum_stub() { return 1; }

void gscr_getbgbtokensremaining_stub(scriptInstance_t inst,
                                     [[maybe_unused]] scr_entref_t entref) {
  game::scr::Scr_AddInt(inst, 255);
}
} // namespace

// Global hash→name lookup table loaded from data/lookup_tables/hash_names.txt
std::unordered_map<uint32_t, std::string> global_hash_table;
std::once_flag hash_table_load_flag;

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
      printf("Loaded %zu hash names from '%s'\n", count, path.string().c_str());
      return true;
    };

    // Try appdata path first, then exe-relative path
    const std::filesystem::path appdata =
        game::get_appdata_path() / "data" / "lookup_tables" / "hash_names.txt";
    if (try_load(appdata))
      return;
    const std::filesystem::path host = utils::nt::library{}.get_folder() /
                                       "boiii" / "data" / "lookup_tables" /
                                       "hash_names.txt";
    if (try_load(host))
      return;
  });
}

std::string resolve_hash(uint32_t hash) {
  if (script_hash_names.contains(hash) && !script_hash_names[hash].empty())
    return script_hash_names[hash][0].name;

  // Fallback: global hash table from data file
  load_global_hash_table();
  if (global_hash_table.contains(hash))
    return global_hash_table[hash];

  return {};
}

uint8_t *find_export_address(const std::string &script_name,
                             const std::string &func_name,
                             int32_t expected_params) {
  return find_export_address_internal(script_name, gsc_hash(func_name),
                                      expected_params);
}

int resolve_hash_line(uint32_t hash, int32_t num_params) {
  if (script_hash_names.contains(hash)) {
    for (hash_info &entry : script_hash_names[hash])
      if (entry.params == static_cast<uint8_t>(num_params) && entry.line > 0)
        return entry.line;
  }
  return 0;
}

std::string get_source_line(const std::string &file, int32_t line_num) {
  // Try to find source by matching file path suffix
  for (auto &[path, src] : script_sources) {
    if (file.find(path) != std::string::npos ||
        path.find(file) != std::string::npos ||
        file.find(std::filesystem::path(path).filename().string()) !=
            std::string::npos) {
      if (line_num <= 0)
        return {};
      int32_t current = 1;
      size_t start = 0;
      while (current < line_num && start < src.size()) {
        if (src[start] == '\n')
          current++;
        start++;
      }
      if (current != line_num)
        return {};
      size_t end = src.find('\n', start);
      if (end == std::string::npos)
        end = src.size();
      std::string line = src.substr(start, end - start);
      if (!line.empty() && line.back() == '\r')
        line.pop_back();
      return line;
    }
  }
  return {};
}

utils::hook::detour Scr_IsFloatTrue_hook;
utils::hook::detour Scr_IsTrue_hook;

struct component final : generic_component {
  void post_unpack() override {

    // Return custom or overrided scripts if found
    db_find_x_asset_header_hook.create(DB_FindXAssetHeader.get(),
                                       db_find_x_asset_header_stub);

    // Free our scripts when the game ends
    game_event::on_g_shutdown_game(clear_script_memory);

    // Load our custom/overriding scripts
    utils::hook::call(game::select(0x141AAE92F, 0x1402D81FF),
                      begin_load_scripts_stub);

    // Force GSC checksums to be valid
    utils::hook::call(game::select(0x1408F2E5D, 0x1400E2D22),
                      server_script_checksum_stub);

    // Workaround for "Out of X" gobblegum
    gscr_get_bgb_tokens_remaining_hook.create(
        gscr::GScr_GetBGBTokensRemaining.get(),
        gscr_getbgbtokensremaining_stub);

    // Fix common "cannot cast undefined to bool" error in flagsys.gsc on
    // launching usermap in private match
    Scr_IsFloatTrue_hook.create(game::scr::Scr_IsFloatTrue.get(),
                                game::scr::Scr_IsTrue_Impl);
    Scr_IsTrue_hook.create(game::scr::Scr_IsTrue.get(),
                           game::scr::Scr_IsTrue_Impl);
  }
};
} // namespace script

REGISTER_COMPONENT(script::component)