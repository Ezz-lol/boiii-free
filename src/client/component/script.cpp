#include <std_include.hpp>

#include "game/impl/db/db.hpp"
#include "script.hpp"

#include <component/dump.hpp>
#include <component/game_event.hpp>
#include <component/gsc/compiler/compiler.hpp>
#include <component/gsc/func/replacefunc.hpp>
#include <component/lua/mod_previews.hpp>
#include <component/mod_stack.hpp>
#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>

#include <game/utils.hpp>

#include <game/impl/scr/gdb.hpp>

#include <execution>

#include <utils/compression.hpp>
#include <utils/concurrency.hpp>
#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/memory.hpp>
#include <utils/string.hpp>

extern "C" {
#include <hksclib.h>
}

using namespace game;

struct TreeDirectory {
  std::filesystem::path path;
  std::optional<std::string> strip_base;
  bool load_recursively;
  bool exclude_map_subtrees = false;
  bool operator==(const TreeDirectory &other) const = default;
};
namespace std {
template <> struct hash<TreeDirectory> {
  std::size_t operator()(const TreeDirectory &p) const noexcept {
    // Fetch individual hash values for the members
    std::size_t h1 = std::hash<std::filesystem::path>{}(p.path);
    std::size_t h2 = std::hash<std::optional<std::string>>{}(p.strip_base);
    std::size_t h3 = std::hash<bool>{}(p.load_recursively);
    std::size_t h4 = std::hash<bool>{}(p.exclude_map_subtrees);

    std::size_t seed = h1;
    seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

    return seed;
  }
};
} // namespace std

namespace script {

void print_asset_log(const std::string_view &message) {
  game::com::Com_Printf(consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT, "%s\n", message.data());
  std::println(std::cerr, "{}", message);
  std::cerr.flush();
  game::trace("{}", message);
}

utils::hook::detour db_find_x_asset_header_hook;

static utils::memory::allocator allocator;

static loadedScriptMap_t loaded_scripts;
static loadedAssetMap_t loaded_assets;
static scriptHashNameMap_t script_hash_names;
static scriptSourceMap_t script_sources;
static scriptGdbMap_t script_gdbs;

std::string normalize_script_name(std::string script_name) {
  size_t start = script_name.find('<');
  size_t end = script_name.find('>');
  if (start != std::string::npos && end != std::string::npos &&
      end > start + 1) {
    const std::string inner = script_name.substr(start + 1, end - start - 1);
    if (inner.contains('/') || inner.contains('\\')) {
      script_name = inner;
    }
  }
  for (char &c : script_name) {
    if (c == '\\') {
      c = '/';
    }
  }
  return script_name;
}

// Sources are stored with their line breaks overwritten by NUL bytes once the
// script is loaded, so both separators end a line
std::string source_line_at(const std::string_view source,
                           const int32_t line_num) {
  constexpr std::string_view LINE_BREAKS("\n\0", 2);
  if (line_num <= 0) {
    return {};
  }

  size_t start = 0;
  for (int32_t current = 1; current < line_num; ++current) {
    start = source.find_first_of(LINE_BREAKS, start);
    if (start == std::string_view::npos) {
      return {};
    }
    ++start;
  }

  const size_t end = source.find_first_of(LINE_BREAKS, start);
  std::string_view line = source.substr(start, end - start);
  if (!line.empty() && line.back() == '\r') {
    line.remove_suffix(1);
  }
  return std::string(line);
}

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
      while (pos < source.size() && source[pos] != '\n') {
        result += source[pos++];
      }
      continue;
    }
    // Skip /* block comments */
    if (pos + 1 < source.size() && source[pos] == '/' &&
        source[pos + 1] == '*') {
      result += source[pos++];
      result += source[pos++];
      while (pos + 1 < source.size() &&
             (source[pos] != '*' || source[pos + 1] != '/')) {
        result += source[pos++];
      }
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
        if (source[pos] == '\\' && pos + 1 < source.size()) {
          result += source[pos++];
        }
        result += source[pos++];
      }
      if (pos < source.size()) {
        result += source[pos++];
      }
      continue;
    }
    if (pos + 1 < source.size() && source[pos] == '/' &&
        source[pos + 1] == '#') {
      const size_t end = source.find("#/", pos + 2);
      if (end != std::string::npos) {
        // Preserve newlines to keep line numbering intact
        for (size_t i = pos; i < end + 2; i++) {
          if (source[i] == '\n') {
            result += '\n';
          }
        }
        pos = end + 2;
        continue;
      }
    }
    result += source[pos++];
  }
  return result;
}

std::string normalize_path(std::string path) {
  for (char &c : path) {
    if (c == '\\') {
      c = '/';
    }
  }
  return path;
}

static std::mutex db_mutex;
const GSC_OBJ *get_linked_obj(const std::string &name) {
  const std::string normalized_name = normalize_script_name(name);

  // Try with extension first, then without
  std::string with_ext = normalized_name;
  const bool has_ext = utils::string::ends_with(with_ext, ".gsc") ||
                       utils::string::ends_with(with_ext, ".csc");
  if (!has_ext) {
    with_ext += ".gsc";
  }

  std::string without_ext = normalized_name;
  if (utils::string::ends_with(without_ext, ".gsc") ||
      utils::string::ends_with(without_ext, ".csc")) {
    without_ext = without_ext.substr(0, without_ext.size() - 4);
  }

  std::string with_ext_norm = normalize_path(with_ext);
  std::string without_ext_norm = normalize_path(without_ext);

  GSC_OBJ *result = nullptr;
  // Check our custom scripts (case-insensitive key search)
  loaded_scripts.for_each([&result, &with_ext_norm, &without_ext_norm](
                              const loadedScriptMap_t::value_type &v) {
    ScriptParseTree *rf = v.second;
    if (result == nullptr && rf != nullptr && rf->buffer != nullptr) {
      std::string key_norm = normalize_path(v.first);
      if (utils::string::iequal(key_norm, with_ext_norm) ||
          utils::string::iequal(key_norm, without_ext_norm)) {
        result = rf->buffer;
      }
    }
  });

  if (result != nullptr) {
    return result;
  }

  // Suffix match: handles full disk paths from runtime replacefunc
  // e.g., "C:/.../data/custom_scripts/foo" matches loaded key
  // "custom_scripts/foo.gsc"
  loaded_scripts.for_each([&result, &without_ext_norm](
                              const loadedScriptMap_t::value_type &v) {
    ScriptParseTree *rf = v.second;
    if (result != nullptr && rf != nullptr && rf->buffer != nullptr) {
      std::string key_no_ext = v.first;
      if (utils::string::ends_with(key_no_ext, ".gsc") ||
          utils::string::ends_with(key_no_ext, ".csc")) {
        key_no_ext = key_no_ext.substr(0, key_no_ext.size() - 4);
      }
      std::string key_no_ext_norm = normalize_path(key_no_ext);

      if (without_ext_norm.size() > key_no_ext_norm.size()) {
        char sep = without_ext_norm[without_ext_norm.size() -
                                    key_no_ext_norm.size() - 1];
        if (sep == '/' && utils::string::iequal(
                              without_ext_norm.substr(without_ext_norm.size() -
                                                      key_no_ext_norm.size()),
                              key_no_ext_norm) == 0) {
          result = rf->buffer;
        }
      }
    }
  });
  if (result != nullptr) {
    return result;
  }

  // Fall back to game's asset database (try .gsc, .csc, and without ext)
  std::string with_csc = without_ext + ".csc";
  for (const std::string &lookup : {with_ext, with_csc, without_ext}) {
    ScriptParseTree *asset = nullptr;
    {
      std::scoped_lock<std::mutex> db_lock(db_mutex);
      asset = db_find_x_asset_header_hook.invoke<ScriptParseTree *>(
          XAssetType::SCRIPTPARSETREE, lookup.c_str(), false, 0);
    }
    if (asset != nullptr && asset->buffer != nullptr) {
      return asset->buffer;
    }
  }

  return nullptr;
}

struct export_lookup_result {
  uint8_t *address = nullptr;
  bool script_loaded = false;
  bool valid_object = false;
  std::vector<uint8_t> matching_params;
};

export_lookup_result
resolve_export_address_internal(const std::string &script_name,
                                uint32_t func_hash,
                                int32_t expected_params = -1) {
  export_lookup_result result{};

  const GSC_OBJ *obj = get_linked_obj(script_name);
  result.script_loaded = (obj != nullptr);
  if (obj != nullptr && obj->hasMagic(GSC_OBJ::T7_MAGIC)) {
    result.valid_object = true;

    const std::span<const GSC_EXPORT_ITEM> exports = obj->exports();
    for (uint16_t i = 0; i < static_cast<uint16_t>(exports.size()); i++) {
      if (exports[i].name == func_hash) {
        result.matching_params.push_back(exports[i].param_count);
        if (expected_params < 0 ||
            exports[i].param_count == static_cast<uint8_t>(expected_params)) {
          result.address = reinterpret_cast<uint8_t *>(
              reinterpret_cast<uintptr_t>(obj) + exports[i].address);
          if (expected_params >= 0) {
            break;
          }
        }
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
  pending_detours.access([](std::vector<pending_detour> &pending_detours) {
    std::vector<pending_detour> remaining_detours;
    remaining_detours.reserve(pending_detours.size());

    for (pending_detour &d : pending_detours) {
      const export_lookup_result target = resolve_export_address_internal(
          d.target_script, d.target_func_hash, d.target_params);
      const export_lookup_result replace = resolve_export_address_internal(
          d.replace_script, d.replace_func_hash, d.replace_params);

      if (target.address && replace.address) {
        gsc::func::replacefunc::add_detour(target.address, replace.address);
      } else {
        if (!target.script_loaded || !replace.script_loaded) {
          remaining_detours.push_back(d);
          continue;
        }

        const auto describe_failure = [](const export_lookup_result &result,
                                         const int32_t expected_params) {
          if (!result.valid_object) {
            return std::string("invalid script object");
          }

          if (result.matching_params.empty()) {
            return std::string("function export not found");
          }

          std::string params;
          for (const uint8_t count : result.matching_params) {
            if (!params.empty()) {
              params += ", ";
            }
            params += std::to_string(count);
          }

          return std::format("expected {} parameter(s), available: {}",
                             expected_params, params);
        };

        std::string reason;
        if (!target.address) {
          reason = "target " + describe_failure(target, d.target_params);
        } else {
          reason = "replacement " + describe_failure(replace, d.replace_params);
        }

        const char *err = utils::string::va(
            "[gsc] detour bind failed %s::%s(%d) -> %s::%s(%d): %s",
            d.target_script.c_str(), d.target_func.c_str(), d.target_params,
            d.replace_script.c_str(), d.replace_func.c_str(), d.replace_params,
            reason.c_str());
        print_asset_log(err);
      }
    }

    pending_detours = std::move(remaining_detours);
  });
}

void add_gdb(const std::string &name, std::vector<uint8_t> gdb) {
  const std::string gdb_name = name + ".gdb";
  while (
      script_gdbs.try_emplace_l(gdb_name, [&](scriptGdbMap_t::value_type &v) {
        v.second = std::move(gdb);
      })) {
  }
}

ScriptParseTree *get_loaded_script(const std::string &name) {
  ScriptParseTree *result = nullptr;
  loaded_scripts.if_contains(
      name, [&](loadedScriptMap_t::value_type &v) { result = v.second; });
  return result;
}

std::mutex script_paths_mutex;
std::unordered_map<std::string, std::string> script_paths;

std::string script_path(const std::string &name) {
  std::scoped_lock lock(script_paths_mutex);
  const auto it = script_paths.find(name);
  return it == script_paths.end() ? name : it->second;
}

void print_loading_script(const std::string &name) {
  const char *type = nullptr;
  if (name.ends_with(".lua")) {
    type = "LUA script";
  } else if (name.ends_with(".csc")) {
    type = "CSC script";
  } else if (name.ends_with(".gsc")) {
    type = "GSC script";
  } else {
    type = "";
  }
  const char *log =
      utils::string::va("Loading %s%s'%s'", type, type[0] == NULL ? "" : " ",
                        script_path(name).c_str());
  print_asset_log(log);
}

void load_script(const std::string &name, const std::string &data) {

  const bool is_csc = utils::string::ends_with(name, ".csc");
  const bool is_gsc = utils::string::ends_with(name, ".gsc");

  std::string base_name = name;
  if (!is_gsc && !is_csc) {
    const char *err = utils::string::va(
        "Script '%s' failed to load due to invalid suffix.", name.data());
    print_asset_log(err);
    return;
  }

  // Skip CSC on dedicated server (no client script instance)
  if (!is_csc || is_client()) {
    base_name = name.substr(0, name.size() - 4);
    if (base_name.empty()) {
      const char *err = utils::string::va(
          "Script '%s' failed to load due to invalid name.", name.data());
      print_asset_log(err);
      return;
    }

    ScriptParseTree *parse_tree = allocator.allocate<ScriptParseTree>();
    parse_tree->name = allocator.duplicate_string(name);
    parse_tree->buffer =
        reinterpret_cast<GSC_OBJ *>(allocator.duplicate_string(data));
    parse_tree->len = data.length();

    while (loaded_scripts.try_emplace_l(
        name,
        [&](loadedScriptMap_t::value_type &v) { v.second = parse_tree; })) {
    }
    const char *log =
        utils::string::va("Loaded script '%s' (size %llu bytes)",
                          script_path(name).c_str(), parse_tree->len);
    print_asset_log(log);
  }
}

// ".csc" or ".gsc"
constexpr std::string_view GSC_EXT = ".gsc";
constexpr std::string_view CSC_EXT = ".csc";
constexpr auto SCRIPT_EXT_LEN = GSC_EXT.size();

void load_script_file(std::string &data,
                      const std::filesystem::path &script_file,
                      const std::string &name) {

  if (data.size() >= sizeof(GSC_OBJ::T7_MAGIC) &&
      reinterpret_cast<GSC_OBJ *>(data.data())->hasMagic(GSC_OBJ::T7_MAGIC)) {
    print_loading_script(name);
    load_script(name, data);
  } else {
    const std::string script_file_str = script_file.generic_string();
    if ((utils::string::ends_with(script_file_str, ".gsc") ||
         utils::string::ends_with(script_file_str, ".csc")) &&
        !data.empty()) {
      const bool is_csc = utils::string::ends_with(script_file_str, ".csc");
      const char *script_type = is_csc ? "CSC" : "GSC";

      // Skip CSC on dedicated server
      if (is_client() || !is_csc) {

        // Strip devblocks before compilation
        const std::string cleaned_source = strip_devblocks(data);

        const char *log = utils::string::va(
            "Compiling %s script '%s'", script_type, script_path(name).c_str());
        print_asset_log(log);
        const scriptInstance_t inst =
            is_csc ? SCRIPTINSTANCE_CLIENT : SCRIPTINSTANCE_SERVER;
        gsc::compiler::compile_result result =
            gsc::compiler::compile(inst, cleaned_source, name);
        if (result.success) {

          // Store hash-to-name+line map from this compilation
          for (gsc::hash_name_pair &hn : result.hash_names) {
            // Will loop at most twice. Once if the value vector needs
            // initialized (entry did not already exist), and once to append
            // this entry to the vector
            while (script_hash_names.try_emplace_l(
                hn.hash, [&](decltype(script_hash_names)::value_type &v) {
                  v.second.push_back(
                      {.name = hn.name, .line = hn.line, .params = hn.params});
                })) {
            }
          }

          // Store original source text for this file
          while (script_sources.try_emplace_l(
              name, [&](scriptSourceMap_t::value_type &v) {
                v.second = cleaned_source;
              })) {
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
          load_script(name, bytecode);
          add_gdb(name, result.gdb);

          // Register replacefunc entries as pending detours
          if (!result.replacefuncs.empty()) {
            std::string replace_base = name;
            if (utils::string::ends_with(replace_base, GSC_EXT) ||
                utils::string::ends_with(replace_base, CSC_EXT)) {
              replace_base =
                  replace_base.substr(0, replace_base.size() - SCRIPT_EXT_LEN);
            }

            for (gsc::compiler::replacefunc_entry &rf : result.replacefuncs) {
              const std::string replace_script =
                  rf.replace_script.empty() ? replace_base : rf.replace_script;
              pending_detours.access(
                  [&](std::vector<pending_detour> &pending_detours) {
                    pending_detours.push_back(
                        {.target_script = rf.target_script,
                         .target_func = rf.target_func,
                         .target_func_hash = gsc::gsc_hash(rf.target_func),
                         .target_params = rf.target_params,
                         .replace_script = replace_script,
                         .replace_func = rf.replace_func,
                         .replace_func_hash = gsc::gsc_hash(rf.replace_func),
                         .replace_params = rf.replace_params});
                  });
            }
          }
        } else {
          const char *err_header = utils::string::va(
              "^1*********************%s COMPILE ERROR*********************",
              script_type);
          print_asset_log(err_header);
          for (const gsc::compiler::compile_error &err : result.errors) {
            const char *file_log =
                utils::string::va("^1  File:    ^5%s", err.file.data());
            print_asset_log(file_log);
            if (err.line > 0) {
              const char *line_column_log = utils::string::va(
                  "^1  Line:    ^2%d^7, ^1Column: ^2%d", err.line, err.column);
              print_asset_log(line_column_log);
              std::string src_line = source_line_at(data, err.line);
              if (!src_line.empty()) {
                const char *src_log =
                    utils::string::va("^1  Source:  ^7%s", src_line.data());
                print_asset_log(src_log);
              }
            }
            const char *error_log =
                utils::string::va("^1  Error:   ^1%s", err.message.data());
            print_asset_log(error_log);
            const char *footer_p1_log =
                utils::string::va("^1------------------------------------------"
                                  "------------------");
            print_asset_log(footer_p1_log);
          }
          print_asset_log(utils::string::va(
              "^1  Result:  ^7%s",
              name.starts_with("scripts/")
                  ? "script not loaded, the game's original script is used"
                  : "script not loaded, the map continues without it"));
          const char *footer_p2_log = utils::string::va(
              "^1************************************************************");
          print_asset_log(footer_p2_log);
        }
      }
    }
  }
}

struct queued_script {
  std::string name;
  std::string base_name;
  scriptInstance_t inst;
};

static std::vector<queued_script> execution_queue;

bool is_script_file_name(const std::string_view &name) {
  return name.ends_with(GSC_EXT) || name.ends_with(".csc");
}

void queue_script_execution(const std::string &name) {
  const bool is_csc = name.ends_with(CSC_EXT);
  if (!is_script_file_name(name) || (is_csc && !is_client()) ||
      name.size() <= 4 || get_loaded_script(name) == nullptr) {
    return;
  }

  execution_queue.push_back(
      {.name = name,
       .base_name = name.substr(0, name.size() - 4),
       .inst = is_csc ? SCRIPTINSTANCE_CLIENT : SCRIPTINSTANCE_SERVER});
}

std::optional<std::string> find_missing_include(const queued_script &script) {
  const ScriptParseTree *tree = get_loaded_script(script.name);
  if (tree != nullptr && tree->buffer != nullptr) {
    const std::string_view &extension =
        script.inst == SCRIPTINSTANCE_CLIENT ? CSC_EXT : GSC_EXT;
    const char *base = reinterpret_cast<const char *>(tree->buffer);
    for (const uint32_t offset : tree->buffer->includes()) {
      if (offset < tree->len) {
        std::string basename(base + offset,
                             strnlen(base + offset, tree->len - offset));
        if (!basename.empty()) {
          const std::string name = std::format("{}{}", basename, extension);
          if (db::xasset::DB_FindXAssetHeader(XAssetType::SCRIPTPARSETREE,
                                              name.c_str(), false, 0)
                  .scriptParseTree == nullptr) {
            return std::move(basename);
          }
        }
      }
    }
  }
  return std::nullopt;
}

void execute_queued_script(const queued_script &script) {
  if (const std::optional<std::string> missing = find_missing_include(script)) {
    print_asset_log(utils::string::va(
        "Skipped script '%s' because it needs '%s', which this game mode does "
        "not have",
        script_path(script.name).c_str(), missing->c_str()));
    return;
  }

  scr::Scr_LoadScript(script.inst, script.base_name.data());

  print_asset_log(utils::string::va("Loaded script '%s' into the VM",
                                    script_path(script.name).c_str()));

  objFileInfo_t *obj = get_obj_by_name(script.inst, script.name);
  if (obj != nullptr) {
    script_sources.modify_if(
        script.name, [&](scriptSourceMap_t::value_type &v) {
          char *src = v.second.data();
          obj->debugInfo.source = src;
          obj->debugInfo.gdb = nullptr;
          obj->debugInfo.sourceLen = static_cast<int32_t>(v.second.size());
          for (size_t i = 0; i < v.second.size(); ++i) {
            char *c = &src[i];
            if (*c == '\n' || *c == '\r') {
              *c = '\0';
            }
          }
        });
  }
}

constexpr const std::string_view GAMETYPE_PREFIXES[] = {"zm", "mp", "cp"};

bool is_map_override_directory_name(const std::string_view &name) {
  return std::ranges::any_of(
      GAMETYPE_PREFIXES, [&name](const std::string_view &prefix) {
        return name.size() > prefix.size() && name.starts_with(prefix) &&
               name[prefix.size()] == '_';
      });
}

void load_scripts_directory(
    const std::string &script_dir, const bool load, const bool recurse,
    const std::optional<std::string_view> strip_base = std::nullopt,
    const bool exclude_map_subtrees = false,
    std::unordered_set<std::string> *processed_scripts = nullptr,
    std::mutex *processed_scripts_mutex = nullptr) {
  if (utils::io::directory_exists(script_dir)) {
    std::vector<std::filesystem::path> scripts =
        utils::io::list_files(script_dir, recurse, false);

    const auto load_dir_file_cb = [load, strip_base, exclude_map_subtrees,
                                   processed_scripts, processed_scripts_mutex,
                                   script_dir](
                                      const std::filesystem::path &script) {
      if (exclude_map_subtrees) {
        std::error_code ec;
        const std::filesystem::path relative =
            std::filesystem::relative(script, script_dir, ec);
        // Only map folders are left for the map's own pass, not files named
        // like one
        if (!ec && relative.begin() != relative.end() &&
            std::next(relative.begin()) != relative.end() &&
            is_map_override_directory_name(relative.begin()->string()))
          return;
      }
      if (!is_script_file_name(script.generic_string()) ||
          std::filesystem::is_directory(script)) {
        return;
      }

      std::string name = script.generic_string();
      const std::string appdata_path = (get_data_path() / "").generic_string();
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
      const std::string path = name;
      if (strip_base.has_value()) {
        std::vector<std::string_view> name_parts =
            utils::string::split(std::string_view(name), '/');
        const std::vector<std::string_view> strip_parts =
            utils::string::split(std::string_view(strip_base.value()), '/');
        bool matches = name_parts.size() > strip_parts.size();
        for (size_t part = 0; matches && part < strip_parts.size(); ++part) {
          matches = name_parts[part + 1] == strip_parts[part];
        }
        if (matches) {
          name_parts.erase(name_parts.begin() + 1,
                           name_parts.begin() + 1 + strip_parts.size());
          name = utils::string::join(name_parts, "/");
        }
      }

      const std::string key = utils::string::to_lower(name);
      if (processed_scripts) {
        std::scoped_lock lock(*processed_scripts_mutex);
        if (!processed_scripts->insert(key).second) {
          return;
        }
      }

      if (path != name) {
        std::scoped_lock lock(script_paths_mutex);
        script_paths.insert_or_assign(name, path);
      }

      if (load) {
        queue_script_execution(name);
        return;
      }

      std::string data;
      if (utils::io::read_file(script, &data)) {
        load_script_file(data, script, name);
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

std::optional<std::filesystem::path> get_game_type_specific_directory() {
  switch (com::Com_SessionMode_GetMode()) {
  case eModes::MULTIPLAYER:
    return "mp";
  case eModes::ZOMBIES:
    return "zm";
  case eModes::CAMPAIGN:
    return "cp";
  default:
    return std::nullopt;
  }
}

std::optional<std::filesystem::path> get_map_specific_directory() {
  const std::string_view mapname = get_mapname().value_or("");
  return mapname.empty() ? std::nullopt : std::optional(mapname);
}

std::optional<std::filesystem::path> get_mod_specific_directory() {
  const std::string_view mod = game::ugc::active_mod->publisherId;
  return mod.empty() ? std::nullopt : std::optional(mod);
}

bool is_shared_tree_dir(const std::filesystem::path &dir,
                        const std::unordered_set<std::string_view> &mod_ids) {
  if (std::filesystem::is_directory(dir)) {
    const std::filesystem::path dirname = dir.filename();
    const std::string dirname_str = dirname.generic_string();
    return !mod_ids.contains(dirname_str) &&
           !std::ranges::any_of(
               GAMETYPE_PREFIXES, [&dirname_str](std::string_view prefix) {
                 return prefix == dirname_str ||
                        (dirname_str.size() > prefix.size() &&
                         utils::string::starts_with(dirname_str, prefix) &&
                         dirname_str[prefix.size()] ==
                             '_'); /* map override script tree */
               });
  }
  return false;
}

template <const size_t N>
std::unordered_set<TreeDirectory>
shared_tree_directories(const std::filesystem::path (&roots)[N],
                        const std::filesystem::path &tree) {
  std::unordered_set<std::string_view> mod_ids;
  for (uint32_t mod_idx = 0; mod_idx < game::ugc::modsPool.count; ++mod_idx) {
    const std::string_view mod_id =
        game::ugc::modsPool.data[mod_idx].publisherId;
    if (!mod_id.empty()) {
      mod_ids.insert(mod_id);
    }
  }
  std::unordered_set<TreeDirectory> root_entries;
  for (const std::filesystem::path &root : roots) {
    for (const std::filesystem::path &entry :
         utils::io::list_files(root / tree, false, true)) {
      if (is_shared_tree_dir(entry, mod_ids)) {
        root_entries.insert({.path = std::filesystem::relative(entry, root),
                             .strip_base = std::nullopt,
                             .load_recursively = true});
      }
    }
  }

  return root_entries;
}

void load_tree(std::filesystem::path tree, bool queue_execution = false) {
  const utils::nt::library host{};

  const std::filesystem::path &data_directory = get_data_path();
  const std::filesystem::path &boiii_directory = get_boiii_dir_path();
  std::unordered_set<std::string> compiled_scripts;
  std::unordered_set<std::string> executed_scripts;
  std::mutex processed_scripts_mutex;

  const auto load = [&data_directory, &boiii_directory, &compiled_scripts,
                     &executed_scripts, &processed_scripts_mutex](
                        const TreeDirectory &dir, const bool load = false) {
    std::unordered_set<std::string> *processed_scripts =
        load ? &executed_scripts : &compiled_scripts;
    load_scripts_directory((data_directory / dir.path).string(), load,
                           dir.load_recursively, dir.strip_base,
                           dir.exclude_map_subtrees, processed_scripts,
                           &processed_scripts_mutex);
    load_scripts_directory((boiii_directory / dir.path).string(), load,
                           dir.load_recursively, dir.strip_base,
                           dir.exclude_map_subtrees, processed_scripts,
                           &processed_scripts_mutex);
  };

  std::vector<TreeDirectory> applicable_tree_dirs;
  {
    const std::unordered_set<TreeDirectory> shared_tree_dirs =
        shared_tree_directories({data_directory, boiii_directory}, tree);

    applicable_tree_dirs = std::vector<TreeDirectory>(shared_tree_dirs.begin(),
                                                      shared_tree_dirs.end());
  }

  const std::optional<std::filesystem::path> game_type =
      get_game_type_specific_directory();
  if (game_type.has_value()) {
    applicable_tree_dirs.push_back({.path = tree / game_type.value(),
                                    .strip_base = std::nullopt,
                                    .load_recursively = true,
                                    .exclude_map_subtrees = true});
  }

  const std::optional<std::filesystem::path> map_name =
      get_map_specific_directory();
  if (map_name.has_value()) {
    const std::string map_name_str = map_name->generic_string();
    applicable_tree_dirs.push_back({.path = tree / map_name.value(),
                                    .strip_base = map_name_str,
                                    .load_recursively = true});
    if (game_type.has_value()) {
      const std::filesystem::path nested_base =
          game_type.value() / map_name.value();
      applicable_tree_dirs.push_back(
          {.path = tree / nested_base,
           .strip_base = nested_base.generic_string(),
           .load_recursively = true});
    }
  }

  const std::optional<std::filesystem::path> mod_id =
      get_mod_specific_directory();
  if (mod_id.has_value()) {
    applicable_tree_dirs.push_back({.path = tree / mod_id.value(),
                                    .strip_base = mod_id->generic_string(),
                                    .load_recursively = true});
  }

  applicable_tree_dirs.push_back(
      {.path = tree, .strip_base = std::nullopt, .load_recursively = false});

  /*
    First, compile and load each script into our lookup table.
    We must do this before loading any scripts into the VM to ensure all
    dependencies are available for lookup upon first script load.
  */
  std::for_each(std::execution::seq, applicable_tree_dirs.begin(),
                applicable_tree_dirs.end(), load);

  if (queue_execution) {
    std::for_each(std::execution::seq, applicable_tree_dirs.begin(),
                  applicable_tree_dirs.end(),
                  [load](const TreeDirectory &dir) { load(dir, true); });
  }
}

void load_scripts() {
  {
    std::scoped_lock lock(script_paths_mutex);
    script_paths.clear();
  }
  // The "scripts" tree is for overriding stock scripts the game uses
  load_tree("scripts", false);
  // The "custom_scripts" tree is for new scripts we must execute
  load_tree("custom_scripts", true);
  for (const auto &[name, data] : mod_stack::script_copies()) {
    print_loading_script(name);
    load_script(name, data);
    queue_script_execution(name);
  }
}

void load_rawfile_buf(const std::string &name, const std::string &data) {

  /*
     This will permanently leak the memory allocated for the asset's name and
     contents. However, if we _do not_ leak this memory, the lua VM continues to
     access the freed memory, even after shutdown.

     This also means that we can never replace an overridden asset in the
     `loaded_assets`, even where it has been modified or should be overriden
     by a mod-specific or map-specific script.

     TODO: find the correct point of engine execution to free these assets and
     hook it accordingly.
  */

  if (!loaded_assets.if_contains(
          name, []([[maybe_unused]] const loadedAssetMap_t::value_type &) {})) {
    RawFile *raw_file = reinterpret_cast<RawFile *>(
        db::xasset::pool::DB_AssetPoolAlloc_Impl(XAssetType::RAWFILE));
    char *rawfile_name = reinterpret_cast<char *>(malloc(name.size() + 1));
    strscpy(rawfile_name, name.data(), name.size() + 1);
    raw_file->name = rawfile_name;
    raw_file->buffer = reinterpret_cast<uint8_t *>(malloc(data.length()));
    memcpy(raw_file->buffer, data.data(), data.length());
    raw_file->len = data.length();

    while (
        loaded_assets.try_emplace_l(name, [&](loadedAssetMap_t::value_type &v) {
          v.second.type = XAssetType::RAWFILE;
          v.second.header.rawfile = raw_file;
        })) {
    }
    print_asset_log(utils::string::va("Loaded rawfile '%s' (size %llu bytes)",
                                      name.data(), raw_file->len));
  }
}

void load_stringtable_buf(const std::string &name, const std::string &data) {

  using namespace game::ddl;
  if (!loaded_assets.if_contains(
          name, []([[maybe_unused]] const loadedAssetMap_t::value_type &) {})) {
    StringTable *table = new StringTable(name, data);

    while (
        loaded_assets.try_emplace_l(name, [&](loadedAssetMap_t::value_type &v) {
          v.second.type = XAssetType::STRINGTABLE;
          v.second.header.stringTable = table;
        })) {
    }
    print_asset_log(utils::string::va("Loaded stringtable '%s'", name.data()));
  }
}

constexpr uint8_t HKS_LUA_BYTECODE_MAGIC[8] = {'\x1b', 'L',    'u',    'a',
                                               'Q',    '\x0e', '\x01', '\x04'};
bool lua_bytecode(std::string &data) {
  return data.size() > std::size(HKS_LUA_BYTECODE_MAGIC) &&
         memcmp(HKS_LUA_BYTECODE_MAGIC, data.data(),
                std::size(HKS_LUA_BYTECODE_MAGIC)) == 0;
}

int write_lua_bytecode([[maybe_unused]] hksc_State *state, const void *data,
                       const size_t size, void *output) {
  try {
    static_cast<std::string *>(output)->append(static_cast<const char *>(data),
                                               size);
    return 0;
  } catch (...) {
    return 1;
  }
}

int dump_lua_bytecode(hksc_State *state, void *output) {
  return lua_dump(state, write_lua_bytecode, output);
}

bool compile_lua(std::string &data, const std::filesystem::path &script_file,
                 const std::string &name) {
  hksc_StateSettings settings{};
  hksI_settings(&settings);
  settings.mode = HKSC_MODE_SOURCE;
  settings.compilersettings.strip = BYTECODE_STRIPPING_ALL;
  settings.compilersettings.ignore_debug = 1;

  hksc_State *state = hksI_newstate(&settings);
  if (state == nullptr) {
    print_asset_log(utils::string::va(
        "Failed to compile Lua rawfile '%s': out of memory", name.data()));
    return false;
  }

  std::string bytecode;
  const std::string source = script_file.generic_string();
  const int status =
      hksI_parser_buffer(state, data.data(), data.size(), source.data(),
                         dump_lua_bytecode, &bytecode);
  if (status != 0) {
    const char *error = lua_geterror(state);
    print_asset_log(
        utils::string::va("Failed to compile Lua rawfile '%s': %s", name.data(),
                          error == nullptr ? "unknown compiler error" : error));
    hksI_close(state);
    return false;
  }

  hksI_close(state);
  if (!lua_bytecode(bytecode)) {
    print_asset_log(utils::string::va(
        "Failed to compile Lua rawfile '%s': invalid bytecode", name.data()));
    return false;
  }

  data = std::move(bytecode);
  return true;
}

void load_asset_file(std::string &data,
                     const std::filesystem::path &script_file,
                     const std::string &name) {
  const std::string ext = script_file.extension().generic_string();

  if (ext == ".csv") {
    load_stringtable_buf(name, data);
  } else if (!name.ends_with(".lua") || lua_bytecode(data) ||
             compile_lua(data, script_file, name)) {
    print_loading_script(name);
    load_rawfile_buf(name, data);
  }
}

void load_assets_directory(
    const std::string &script_dir, const bool recurse,
    const std::optional<std::string_view> strip_base = std::nullopt) {
  if (utils::io::directory_exists(script_dir)) {
    std::vector<std::filesystem::path> scripts =
        utils::io::list_files(script_dir, recurse, false);

    const auto load_dir_file_cb =
        [strip_base](const std::filesystem::path &script) {
          std::string data;
          if (!std::filesystem::is_directory(script) &&
              utils::io::read_file(script, &data)) {

            std::string name = script.generic_string();
            const std::string appdata_path =
                (get_data_path() / "").generic_string();
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
            if (strip_base.has_value()) {
              std::vector<std::string_view> name_parts =
                  utils::string::split(std::string_view(name), '/');
              if (name_parts[0] == strip_base.value()) {
                name_parts.erase(name_parts.begin());
                name = utils::string::join(name_parts, "/");
              }
            }

            load_asset_file(data, script, name);
          }
        };

    std::for_each(std::execution::par, scripts.begin(), scripts.end(),
                  load_dir_file_cb);
  }
}

void load_assets_directory_from_trees(const TreeDirectory &dir) {
  load_assets_directory((get_data_path() / dir.path).string(),
                        dir.load_recursively, dir.strip_base);
  load_assets_directory((get_boiii_dir_path() / dir.path).string(),
                        dir.load_recursively, dir.strip_base);
}

static std::mutex script_load_mutex;
void load_assets() {
  std::scoped_lock lock(script_load_mutex);

  const utils::nt::library host{};

  std::unordered_set<TreeDirectory> applicable_tree_dirs = {
      {.path = "lua", .strip_base = std::nullopt, .load_recursively = true},
      {.path = "", .strip_base = std::nullopt, .load_recursively = false},
      {.path = "aim_assist",
       .strip_base = std::nullopt,
       .load_recursively = true},
      {.path = "con_restrict",
       .strip_base = std::nullopt,
       .load_recursively = true},
      {.path = "devgui", .strip_base = std::nullopt, .load_recursively = true},
      {.path = "fpstool", .strip_base = std::nullopt, .load_recursively = true},
      {.path = "gamedata",
       .strip_base = std::nullopt,
       .load_recursively = true},
      {.path = "leaderboards",
       .strip_base = std::nullopt,
       .load_recursively = true},
      {.path = "radiant", .strip_base = std::nullopt, .load_recursively = true},
      {.path = "scriptbundle",
       .strip_base = std::nullopt,
       .load_recursively = true},
      {.path = "ui", .strip_base = std::nullopt, .load_recursively = true},
      {.path = "ui_mp", .strip_base = std::nullopt, .load_recursively = true},
      {.path = "vehicle", .strip_base = std::nullopt, .load_recursively = true},
      {.path = "video", .strip_base = std::nullopt, .load_recursively = true},
      {.path = "vision", .strip_base = std::nullopt, .load_recursively = true},
      {.path = "zone", .strip_base = std::nullopt, .load_recursively = true},
      {.path = "zone_source",
       .strip_base = std::nullopt,
       .load_recursively = true},
      {.path = "animtrees",
       .strip_base = std::nullopt,
       .load_recursively = true},
      {.path = "radiant",
       .strip_base = std::nullopt,
       .load_recursively = true}};

  if (game::is_client()) {
    applicable_tree_dirs.insert(
        {.path = "ui", .strip_base = std::nullopt, .load_recursively = true});
    applicable_tree_dirs.insert({.path = "ui_mp",
                                 .strip_base = std::nullopt,
                                 .load_recursively = true});
    applicable_tree_dirs.insert(
        {.path = "lui", .strip_base = std::nullopt, .load_recursively = true});
  }

  const std::optional<std::filesystem::path> map_name =
      get_map_specific_directory();
  std::unordered_set<TreeDirectory> map_override_directories;
  if (map_name.has_value()) {
    for (const TreeDirectory &tree : applicable_tree_dirs) {
      TreeDirectory override_tree = tree;
      override_tree.path = map_name.value() / tree.path;
      override_tree.strip_base = map_name->generic_string();
      map_override_directories.insert(override_tree);
    }
  }

  const std::optional<std::filesystem::path> mod_id =
      get_mod_specific_directory();
  std::unordered_set<TreeDirectory> mod_override_directories;
  if (mod_id.has_value()) {
    for (const TreeDirectory &tree : applicable_tree_dirs) {
      TreeDirectory override_tree = tree;
      override_tree.path = mod_id.value() / tree.path;
      override_tree.strip_base = mod_id->generic_string();
      mod_override_directories.insert(override_tree);
    }
  }
  applicable_tree_dirs.reserve(applicable_tree_dirs.size() +
                               mod_override_directories.size() +
                               map_override_directories.size());
  applicable_tree_dirs.insert(mod_override_directories.begin(),
                              mod_override_directories.end());
  applicable_tree_dirs.insert(map_override_directories.begin(),
                              map_override_directories.end());

  std::for_each(std::execution::par, applicable_tree_dirs.begin(),
                applicable_tree_dirs.end(), load_assets_directory_from_trees);
}

XAsset get_loaded_asset(const std::string &name) {
  std::scoped_lock script_load_lock(script_load_mutex);
  XAsset result{};
  loaded_assets.if_contains(
      name, [&](const loadedAssetMap_t::value_type &v) { result = v.second; });
  return result;
}

XAssetHeader DB_FindXAssetHeader_TryOverride(const XAssetType type,
                                             const char *name,
                                             const bool error_if_missing,
                                             const int32_t wait_time) {
  XAssetHeader result{.rawfile = nullptr};
  // Check our loaded assets first to attempt override or custom script load
  if (name != nullptr && name[0] != NULL) {
    switch (type) {
    case XAssetType::SCRIPTPARSETREE: {
      result.scriptParseTree = get_loaded_script(name);
      break;
    }
    case XAssetType::RAWFILE: {
      result.rawfile = get_loaded_asset(name).header.rawfile;
      break;
    }
    case XAssetType::STRINGTABLE: {
      result.stringTable = get_loaded_asset(name).header.stringTable;
      break;
    }

    case XAssetType::IMAGE: {
      result.image = mod_previews::find_image(name);
      break;
    }
    default:
      break;
    }
  }

  if (result.data == nullptr) {
    result = db_find_x_asset_header_hook.invoke<XAssetHeader>(
        type, name, error_if_missing, wait_time);
  }

  if (nonnull(result.rawfile)) {
    dump::dump_requested_assets(type, name, result);
  }

  return result;
}

void free_loaded_assets() {
  // Collect keys first so we don't invalidate iterators while traversing.
  std::vector<std::string> keys_to_erase;
  loaded_assets.for_each(
      [&keys_to_erase](const loadedAssetMap_t::value_type &v) {
        switch (v.second.type) {
        case XAssetType::STRINGTABLE: {
          keys_to_erase.push_back(v.first);
          break;
        }
        default: {
          break;
        }
        }
      });

  for (const std::string &key : keys_to_erase) {
    loaded_assets.erase_if(key, [](loadedAssetMap_t::value_type &v) {
      switch (v.second.type) {
      case XAssetType::STRINGTABLE: {
        delete v.second.header.stringTable;
        v.second.header.stringTable = nullptr;
        return true;
      }
      default: {
        return false;
      }
      }
    });
  }
}

void clear_script_memory() {
  std::scoped_lock lock(script_load_mutex);

  free_loaded_assets();
  execution_queue.clear();
  loaded_scripts.clear();
  script_gdbs.clear();
  script_hash_names.clear();
  script_sources.clear();
  pending_detours.access([](std::vector<pending_detour> &pending_detours) {
    pending_detours.clear();
  });
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
          script_gdbs[key] = value;
        }
      }
    } catch (...) {
    }
    std::filesystem::remove(scriptgdb_archive_path);
  }

  utils::compression::zip::write_file(
      scriptgdb_archive_path,
      std::vector(script_gdbs.begin(), script_gdbs.end()));
}

int32_t CG_TestServerScriptChecksum_AlwaysMatch() { return 1; }

// Global hash→name lookup table loaded from data/lookup_tables/hash_names.txt
static std::unordered_map<uint32_t, std::string> global_hash_table;
static std::once_flag hash_table_load_flag;

void load_global_hash_table() {
  std::call_once(hash_table_load_flag, [] {
    const auto try_load = [](const std::filesystem::path &path) {
      std::string data;
      if (utils::io::read_file(path.string(), &data)) {
        size_t count = 0;
        std::istringstream stream(data);
        std::string line;
        while (std::getline(stream, line)) {
          if (!line.empty()) {
            const size_t space = line.find(' ');
            if (space != std::string::npos) {
              ScrVarCanonicalName_t hash = static_cast<uint32_t>(
                  std::strtoul(line.substr(0, space).c_str(), nullptr, 16));
              if (hash != 0) {
                global_hash_table[hash] = line.substr(space + 1);
              }
              count++;
            }
          }
        }
        const char *log = utils::string::va("Loaded %zu hash names from '%s'",
                                            count, path.string().c_str());
        print_asset_log(log);
        return true;
      }

      return false;
    };

    // Try appdata path first, then exe-relative path
    const std::filesystem::path appdata =
        get_data_path() / "lookup_tables" / "hash_names.txt";
    if (!try_load(appdata)) {
      const std::filesystem::path host = utils::nt::library{}.get_folder() /
                                         "boiii" / "data" / "lookup_tables" /
                                         "hash_names.txt";
      try_load(host);
    }
  });
}

void begin_load_scripts_stub(scriptInstance_t inst, int32_t user) {
  load_global_hash_table();

  scr::Scr_BeginLoadScripts(inst, user);

  if (com::Com_IsInGame()) {
    {
      std::scoped_lock lock(script_load_mutex);
      execution_queue.clear();
      load_scripts();
      rebuild_script_gdb();
    }

    for (const queued_script &script : execution_queue) {
      execute_queued_script(script);
    }

    std::scoped_lock lock(script_load_mutex);
    if (!pending_detours.access<bool>(
            [](const std::vector<pending_detour> &pending_detours) {
              return pending_detours.empty();
            })) {
      apply_pending_detours();
    }
  }
}

std::string resolve_hash(ScrVarCanonicalName_t hash) {
  std::optional<std::string> result;
  script_hash_names.if_contains(
      hash, [&result](const scriptHashNameMap_t::value_type &v) {
        if (!v.second.empty()) {
          result = v.second[0].name;
        }
      });

  // Fallback: global hash table from data file
  if (global_hash_table.contains(hash)) {
    result = global_hash_table[hash];
  }

  return result.value_or("");
}

uint8_t *find_export_address(const std::string &script_name,
                             const std::string &func_name,
                             int32_t expected_params) {
  return find_export_address_internal(script_name, gsc::gsc_hash(func_name),
                                      expected_params);
}

int32_t resolve_hash_line(ScrVarCanonicalName_t hash, int32_t num_params) {
  int32_t result = 0;
  script_hash_names.if_contains(
      hash, [&result, num_params](const scriptHashNameMap_t::value_type &v) {
        for (const hash_info &entry : v.second) {
          if (entry.params == static_cast<uint8_t>(num_params) &&
              entry.line > 0) {
            result = entry.line;
            break;
          }
        }
      });
  return result;
}

std::string get_source_line(const std::string &file, int32_t line_num) {
  const std::string wanted =
      utils::string::to_lower(normalize_script_name(file));
  std::string result;
  script_sources.for_each([&](const scriptSourceMap_t::value_type &v) {
    if (result.empty() &&
        utils::string::to_lower(normalize_script_name(v.first)) == wanted) {
      result = source_line_at(v.second, line_num);
    }
  });

  const size_t first = result.find_first_not_of(" \t");
  return first == std::string::npos ? std::string() : result.substr(first);
}

namespace {
const GSC_EXPORT_ITEM *containing_export(const scriptInstance_t inst,
                                         const uint8_t *pos) {
  if (pos != nullptr) {
    const objFileInfo_t *info =
        Scr_FindObjFileInfo_Impl(inst, const_cast<uint8_t *>(pos - 1));
    if (info != nullptr) {
      const GSC_OBJ *obj = info->activeVersion;
      const uint8_t *base = reinterpret_cast<const uint8_t *>(obj);
      const uint32_t offset = static_cast<uint32_t>(pos - 1 - base);
      const GSC_EXPORT_ITEM *found = nullptr;
      for (const GSC_EXPORT_ITEM &item : obj->exports()) {
        if (item.address <= offset &&
            (found == nullptr || item.address > found->address)) {
          found = &item;
        }
      }
      return found;
    }
  }

  return nullptr;
}

const uint8_t *containing_function(const scriptInstance_t inst,
                                   const uint8_t *pos) {
  const GSC_EXPORT_ITEM *item = containing_export(inst, pos);
  if (item == nullptr) {
    return nullptr;
  }
  const objFileInfo_t *info =
      Scr_FindObjFileInfo_Impl(inst, const_cast<uint8_t *>(pos - 1));
  return reinterpret_cast<const uint8_t *>(info->activeVersion) + item->address;
}

std::string function_name(const scriptInstance_t inst, const uint8_t *pos) {
  const GSC_EXPORT_ITEM *item = containing_export(inst, pos);
  return item == nullptr ? std::string{}
                         : std::format("{}::{}", resolve_hash(item->name_space),
                                       resolve_hash(item->name));
}
} // namespace

std::vector<script_frame> get_script_frames(scriptInstance_t inst,
                                            const uint8_t *pos) {
  if (is_server()) {
    sv_detailedScriptErrors->set(true);
  }

  std::vector<uint8_t *> positions;
  positions.emplace_back(pos ? const_cast<uint8_t *>(pos)
                             : vm::gFs->instance[inst].pos);
  for (int32_t stack_idx = vm::gScrVmPub->instance[inst].function_count - 1;
       stack_idx > -1; --stack_idx) {
    positions.emplace_back(
        vm::gScrVmPub->instance[inst].function_frame_start[stack_idx].fs.pos);
  }

  if (positions.size() > 1) {
    const uint8_t *root = containing_function(inst, positions.back());
    if (root != nullptr &&
        root == containing_function(inst, positions[positions.size() - 2])) {
      positions.pop_back();
    }
  }

  std::vector<script_frame> frames;
  for (uint8_t *position : positions) {
    script_frame &frame = frames.emplace_back();
    if (position != nullptr &&
        position != reinterpret_cast<uint8_t *>(vm::g_endPos.get())) {
      const char *file = nullptr;
      const char *source = nullptr;
      Scr_GetFileAndLineNum_Impl(inst, position - 1, &file, &frame.line,
                                 &source);
      frame.file = file == nullptr ? "" : file;
      frame.function = function_name(inst, position);
      if (frame.line >= 0) {
        ++frame.line;
        for (; source != nullptr && (source[0] == ' ' || source[0] == '\t');
             ++source) {
        }
        frame.source = source == nullptr ? "" : source;
      }
    }
  }
  return frames;
}

std::vector<std::string> get_script_callstack(scriptInstance_t inst,
                                              const uint8_t *pos) {
  std::vector<std::string> callstack;
  for (const script_frame &frame : get_script_frames(inst, pos)) {
    if (frame.file.empty()) {
      callstack.emplace_back(
          "Missing file and line information - not currently executing "
          "script bytecode or reached end of executed script bytecode.");
    } else if (frame.line < 0) {
      callstack.push_back(
          std::format("\tfile '{}' - missing line information\n", frame.file));
    } else {
      callstack.push_back(std::format("\tfile '{}', line {} :: {}\n",
                                      frame.file, frame.line, frame.source));
    }
  }
  return callstack;
}

utils::hook::detour Scr_Error_hook;

void Scr_Error_LogAll(scriptInstance_t inst, const char *error, bool terminal) {
  if (game::log_script_errors()) {
    void *caller_addr = _ReturnAddress();
    if (is_server()) {
      sv_detailedScriptErrors->set(true);
    }
    if (terminal) {
      vm::gScrVarPub->instance[inst].developer = true;
      vm::gScrVmPub->instance[inst].debugCode = true;
    }

    const std::vector<std::string> callstack = get_script_callstack(inst);
    std::string prev_code_positions_string{};
    if (!callstack.empty()) {
      prev_code_positions_string = std::format("[0] {}", callstack[0]);
      for (size_t i = 1; i < callstack.size(); ++i) {
        prev_code_positions_string += std::format("\n[{}] {}", i, callstack[i]);
      }
    }

    const char *error_log = utils::string::va(
        "Scr_Error called from 0x%p with inst: %s, "
#ifndef NDEBUG
        "gFs.pos: 0x%p, gFs.top: 0x%p, gFs.startTop: 0x%p, gFs.threadId: "
        "0x%08X, "
        "gFs.localVarCount: %lu, "
#endif
        "error: \"%s\", terminal: %s, callstack:\n%s",
        derelocate(caller_addr), serialize(inst),
#ifndef NDEBUG
        vm::gFs->instance[inst].pos,
        game::derelocate(vm::gFs->instance[inst].top),
        game::derelocate(vm::gFs->instance[inst].startTop),
        vm::gFs->instance[inst].threadId, vm::gFs->instance[inst].localVarCount,
#endif
        error == nullptr ? "NULL" : error, terminal ? "true" : "false",
        prev_code_positions_string.c_str());

    print_asset_log(error_log);
  }
  Scr_Error_hook.invoke(inst, error, terminal);
}

utils::hook::detour CG_TestServerScriptChecksum_hook;

utils::hook::detour LoadScriptGDB2_hook;
utils::hook::detour LoadScriptGDB_hook;
utils::hook::detour Scr_FindObjFileInfo_hook;
utils::hook::detour Scr_GetFileAndLineNum_hook;

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("script");

  void post_unpack() override {
    // Return custom or overrided scripts if found
    db_find_x_asset_header_hook.create(DB_FindXAssetHeader,
                                       DB_FindXAssetHeader_TryOverride);

    // Free our scripts when the game ends
    game_event::on_g_shutdown_game(clear_script_memory);

    // Load our custom/overriding scripts
    utils::hook::call(select(0x141AA255F, 0x1402D81FF),
                      begin_load_scripts_stub);

    // Force GSC checksums to be valid
    CG_TestServerScriptChecksum_hook.create(
        game::cg::CG_TestServerScriptChecksum,
        CG_TestServerScriptChecksum_AlwaysMatch);

    Scr_Error_hook.create(Scr_Error, Scr_Error_LogAll);

    LoadScriptGDB_hook.create(LoadScriptGDB, LoadScriptGDB_Impl);
    LoadScriptGDB2_hook.create(LoadScriptGDB2, LoadScriptGDB2_Impl);

    Scr_FindObjFileInfo_hook.create(Scr_FindObjFileInfo,
                                    Scr_FindObjFileInfo_Impl);
    Scr_GetFileAndLineNum_hook.create(Scr_GetFileAndLineNum,
                                      Scr_GetFileAndLineNum_Impl);
  }
};
} // namespace script

REGISTER_COMPONENT(script::component)
