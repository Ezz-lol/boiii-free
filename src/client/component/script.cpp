#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"

#include "game_event.hpp"
#include "gsc/gsc_compiler.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

namespace gsc_funcs {
void add_detour(int64_t target_addr, int64_t replacement_addr);
}

namespace script {
namespace {
constexpr size_t GSC_MAGIC = 0x1C000A0D43534780;

utils::hook::detour db_find_x_asset_header_hook;
utils::hook::detour gscr_get_bgb_remaining_hook;

utils::memory::allocator allocator;
std::unordered_map<std::string, game::RawFile *> loaded_scripts;

bool try_parse_raw_hash(const std::string &input, uint32_t &out) {
  auto underscore = input.find('_');
  if (underscore == std::string::npos || underscore + 1 >= input.size())
    return false;

  auto prefix = input.substr(0, underscore);
  if (prefix != "hash" && prefix != "function" && prefix != "var" &&
      prefix != "namespace")
    return false;

  auto hex_part = input.substr(underscore + 1);
  if (hex_part.size() != 8)
    return false;

  for (char c : hex_part) {
    if (!std::isxdigit(static_cast<unsigned char>(c)))
      return false;
  }

  out = static_cast<uint32_t>(std::stoul(hex_part, nullptr, 16));
  return out != 0;
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
  uint32_t target_func_hash;
  std::string replace_script;
  uint32_t replace_func_hash;
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

void fixup_script_imports(char *buf, int len) {
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
    auto *asset = db_find_x_asset_header_hook.invoke<game::RawFile *>(
        game::ASSET_TYPE_SCRIPTPARSETREE, (inc_path + ".gsc").c_str(), false,
        0);
    if (!asset || !asset->buffer)
      asset = db_find_x_asset_header_hook.invoke<game::RawFile *>(
          game::ASSET_TYPE_SCRIPTPARSETREE, (inc_path + ".csc").c_str(), false,
          0);
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
      auto *exps = reinterpret_cast<const t7_export_entry *>(spt + exp_off);
      uint32_t actual_ns = exps[0].ns_name;
      if (actual_ns != path_hash)
        path_to_ns[path_hash] = actual_ns;
    }
  }

  if (path_to_ns.empty())
    return;

  // Walk import table and patch ns_hash where it matches a path hash
  size_t pos = import_offset;
  int patched = 0;
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
      patched++;
    }

    pos += 12 + 4 * static_cast<size_t>(num_refs);
  }
}

const uint8_t *get_spt_buffer(const std::string &name) {
  // Try with extension first, then without
  std::string with_ext = name;
  const bool has_ext = utils::string::ends_with(with_ext, ".gsc") ||
                       utils::string::ends_with(with_ext, ".csc");
  if (!has_ext)
    with_ext += ".gsc";
  std::string without_ext = name;
  if (utils::string::ends_with(without_ext, ".gsc") ||
      utils::string::ends_with(without_ext, ".csc"))
    without_ext = without_ext.substr(0, without_ext.size() - 4);

  // Check our custom scripts (case-insensitive key search)
  for (auto &[key, rf] : loaded_scripts) {
    if (rf && rf->buffer &&
        (_stricmp(key.c_str(), with_ext.c_str()) == 0 ||
         _stricmp(key.c_str(), without_ext.c_str()) == 0))
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
    // Check if without_ext ends with /key_no_ext or \key_no_ext
    if (without_ext.size() > key_no_ext.size()) {
      char sep = without_ext[without_ext.size() - key_no_ext.size() - 1];
      if ((sep == '/' || sep == '\\') &&
          _stricmp(without_ext.c_str() + without_ext.size() - key_no_ext.size(),
                   key_no_ext.c_str()) == 0)
        return reinterpret_cast<const uint8_t *>(rf->buffer);
    }
  }

  // Fall back to game's asset database (try .gsc, .csc, and without ext)
  std::string with_csc = without_ext + ".csc";
  for (auto &lookup : {with_ext, with_csc, without_ext}) {
    auto *asset = db_find_x_asset_header_hook.invoke<game::RawFile *>(
        game::ASSET_TYPE_SCRIPTPARSETREE, lookup.c_str(), false, 0);
    if (asset && asset->buffer)
      return reinterpret_cast<const uint8_t *>(asset->buffer);
  }

  return nullptr;
}

int64_t find_export_address_internal(const std::string &script_name,
                                     uint32_t func_hash) {
  const uint8_t *buffer = get_spt_buffer(script_name);
  if (!buffer)
    return 0;

  uint64_t magic = 0;
  std::memcpy(&magic, buffer, sizeof(magic));
  if (magic != GSC_MAGIC)
    return 0;

  uint32_t export_offset = 0;
  uint16_t export_count = 0;
  std::memcpy(&export_offset, buffer + 0x20, sizeof(export_offset));
  std::memcpy(&export_count, buffer + 0x3A, sizeof(export_count));

  auto *exports =
      reinterpret_cast<const t7_export_entry *>(buffer + export_offset);
  for (uint16_t i = 0; i < export_count; i++) {
    if (exports[i].func_name == func_hash)
      return reinterpret_cast<int64_t>(buffer + exports[i].offset);
  }
  return 0;
}

void apply_pending_detours() {
  for (auto &d : pending_detours) {
    int64_t target =
        find_export_address_internal(d.target_script, d.target_func_hash);
    int64_t replace =
        find_export_address_internal(d.replace_script, d.replace_func_hash);
    if (target && replace) {
      gsc_funcs::add_detour(target, replace);
    } else {
      printf("^1[replacefunc] FAILED: %s::0x%08X -> %s::0x%08X\n",
             d.target_script.c_str(), d.target_func_hash,
             d.replace_script.c_str(), d.replace_func_hash);

      if (!target)
        printf("^1  Reason: target function not found in '%s' (script %s "
               "loaded)\n",
               d.target_script.c_str(),
               get_spt_buffer(d.target_script) ? "IS" : "NOT");
      if (!replace)
        printf("^1  Reason: replacement function not found in '%s' (script %s "
               "loaded)\n",
               d.replace_script.c_str(),
               get_spt_buffer(d.replace_script) ? "IS" : "NOT");
    }
  }
}

struct hash_info {
  std::string name;
  int line;
  uint8_t params;
};
std::unordered_map<uint32_t, std::vector<hash_info>> script_hash_names;

std::unordered_map<std::string, std::string> script_sources;

game::RawFile *get_loaded_script(const std::string &name) {
  const auto itr = loaded_scripts.find(name);
  return (itr == loaded_scripts.end()) ? nullptr : itr->second;
}

void print_loading_script(const std::string &name) {
  const char *type = utils::string::ends_with(name, ".csc") ? "CSC" : "GSC";
  printf("Loading %s script '%s'\n", type, name.data());
}

void load_script(std::string &name, const std::string &data,
                 const bool is_custom) {
  const auto appdata_path =
      (game::get_appdata_path() / "data/").generic_string();
  const auto host_path =
      (utils::nt::library{}.get_folder() / "boiii/").generic_string();

  auto i = name.find(appdata_path);
  if (i != std::string::npos) {
    name.erase(i, appdata_path.length());
  }

  i = name.find(host_path);
  if (i != std::string::npos) {
    name.erase(i, host_path.length());
  }

  const bool is_csc = utils::string::ends_with(name, ".csc");
  const bool is_gsc = utils::string::ends_with(name, ".gsc");

  auto base_name = name;
  if (!is_gsc && !is_csc) {
    printf("Script '%s' failed to load due to invalid suffix.\n", name.data());
    return;
  }

  // Skip CSC on dedicated server (no client script instance)
  if (is_csc && game::is_server()) {
    return;
  }

  if (is_custom) {
    base_name = name.substr(0, name.size() - 4);
    if (base_name.empty()) {
      printf("Script '%s' failed to load due to invalid name.\n", name.data());
      return;
    }
  }

  auto *raw_file = allocator.allocate<game::RawFile>();
  raw_file->name = allocator.duplicate_string(name);
  raw_file->buffer = allocator.duplicate_string(data);
  raw_file->len = static_cast<int>(data.length());

  fixup_script_imports(const_cast<char *>(raw_file->buffer), raw_file->len);

  loaded_scripts[name] = raw_file;

  if (is_custom) {
    const auto inst =
        is_csc ? game::SCRIPTINSTANCE_CLIENT : game::SCRIPTINSTANCE_SERVER;
    game::Scr_LoadScript(inst, base_name.data());
  }
}

void load_scripts_folder(const std::string &script_dir, const bool is_custom) {
  if (!utils::io::directory_exists(script_dir)) {
    return;
  }

  const auto scripts = utils::io::list_files(script_dir);

  std::error_code e;
  for (const auto &script : scripts) {
    std::string data;
    auto script_file = script.generic_string();
    if (!std::filesystem::is_directory(script, e) &&
        utils::io::read_file(script_file, &data)) {
      if (data.size() >= sizeof(GSC_MAGIC) &&
          !std::memcmp(data.data(), &GSC_MAGIC, sizeof(GSC_MAGIC))) {
        print_loading_script(script_file);
        load_script(script_file, data, is_custom);
      } else if ((utils::string::ends_with(script_file, ".gsc") ||
                  utils::string::ends_with(script_file, ".csc")) &&
                 !data.empty()) {
        const bool is_csc = utils::string::ends_with(script_file, ".csc");
        const char *script_type = is_csc ? "CSC" : "GSC";

        // Skip CSC on dedicated server
        if (is_csc && game::is_server())
          continue;

        // Strip devblocks before compilation
        auto cleaned_source = strip_devblocks(data);

        printf("Compiling %s script '%s'\n", script_type, script_file.data());
        auto result = gsc_compiler::compile(cleaned_source, script_file);
        if (result.success) {
          std::string bytecode(result.bytecode.begin(), result.bytecode.end());

          // Store hash-to-name+line map from this compilation
          for (auto &hn : result.hash_names) {
            script_hash_names[hn.hash].push_back({hn.name, hn.line, hn.params});
          }

          // Store original source text for this file
          script_sources[script_file] = data;

          print_loading_script(script_file);
          load_script(script_file, bytecode, is_custom);

          // Register replacefunc entries as pending detours
          if (!result.replacefuncs.empty()) {
            std::string replace_base = script_file;
            if (utils::string::ends_with(replace_base, ".gsc") ||
                utils::string::ends_with(replace_base, ".csc"))
              replace_base = replace_base.substr(0, replace_base.size() - 4);

            for (auto &rf : result.replacefuncs) {

              pending_detours.push_back({rf.target_script,
                                         gsc_hash(rf.target_func), replace_base,
                                         gsc_hash(rf.replace_func)});
            }
          }
        } else {
          auto get_source_line = [](const std::string &src,
                                    int line_num) -> std::string {
            if (line_num <= 0)
              return "";
            int current = 1;
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
            auto line = src.substr(start, end - start);
            if (!line.empty() && line.back() == '\r')
              line.pop_back();
            return line;
          };

          printf(
              "^1*********************%s COMPILE ERROR*********************\n",
              script_type);
          for (const auto &err : result.errors) {
            printf("^1  File:    ^5%s\n", err.file.data());
            if (err.line > 0) {
              printf("^1  Line:    ^2%d^7, ^1Column: ^2%d\n", err.line,
                     err.column);
              auto src_line = get_source_line(data, err.line);
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

      continue;
    }

    // Do not traverse directories for custom scripts.
    if (std::filesystem::is_directory(script, e) && !is_custom) {
      load_scripts_folder(script_file, is_custom);
    }
  }
}

std::optional<std::filesystem::path> get_game_type_specific_folder() {
  switch (game::Com_SessionMode_GetMode()) {
  case game::MODE_MULTIPLAYER:
    return "mp";
  case game::MODE_ZOMBIES:
    return "zm";
  case game::MODE_CAMPAIGN:
    return "cp";
  default:
    return {};
  }
}

void load_scripts() {
  const utils::nt::library host{};

  const auto data_folder = game::get_appdata_path() / "data";
  const auto boiii_folder = host.get_folder() / "boiii";

  const auto load = [&data_folder,
                     &boiii_folder](const std::filesystem::path &folder,
                                    const bool is_custom) {
    load_scripts_folder((data_folder / folder).string(), is_custom);
    load_scripts_folder((boiii_folder / folder).string(), is_custom);
  };

  // scripts folder is for overriding stock scripts the game uses
  load("scripts", false);

  // custom_scripts is for loading completely custom scripts the game doesn't
  // use
  load("custom_scripts", true);

  if (const auto game_type = get_game_type_specific_folder();
      game_type.has_value()) {
    load("custom_scripts" / game_type.value(), true);
  }

  const std::filesystem::path mapname = game::get_dvar_string("mapname");
  if (!mapname.empty()) {
    load("custom_scripts" / mapname, true);
  }
}

game::RawFile *db_find_x_asset_header_stub(const game::XAssetType type,
                                           const char *name,
                                           const bool error_if_missing,
                                           const int wait_time) {
  // Check our loaded scripts FIRST to avoid "Could not find scriptparsetree"
  // spam
  if (type == game::ASSET_TYPE_SCRIPTPARSETREE) {
    auto *script = get_loaded_script(name);
    if (script)
      return script;
  }

  return db_find_x_asset_header_hook.invoke<game::RawFile *>(
      type, name, error_if_missing, wait_time);
}

void clear_script_memory() {
  loaded_scripts.clear();
  script_hash_names.clear();
  script_sources.clear();
  pending_detours.clear();
  allocator.clear();
}

// Fix up imports that use full-path namespace hashes to match the actual
// target script's #namespace hash. This allows full-path call syntax:
//   scripts\zm\_zm_score::add_to_player_score(points)
// The import ns_hash is gsc_hash("scripts/zm/_zm_score"), but the game script's
void begin_load_scripts_stub(game::scriptInstance_t inst, int user) {
  game::Scr_BeginLoadScripts(inst, user);

  if (game::Com_IsInGame() && !game::Com_IsRunningUILevel()) {
    load_scripts();

    if (!pending_detours.empty()) {
      apply_pending_detours();
    }
  }
}

int server_script_checksum_stub() { return 1; }

void scr_loot_get_item_quantity_stub(
    game::scriptInstance_t inst, [[maybe_unused]] game::scr_entref_t entref) {
  game::Scr_AddInt(inst, 255);
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
        auto space = line.find(' ');
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
    const auto appdata =
        game::get_appdata_path() / "data" / "lookup_tables" / "hash_names.txt";
    if (try_load(appdata))
      return;
    const auto host = utils::nt::library{}.get_folder() / "boiii" / "data" /
                      "lookup_tables" / "hash_names.txt";
    if (try_load(host))
      return;
  });
}

std::string resolve_hash(uint32_t hash) {
  auto it = script_hash_names.find(hash);
  if (it != script_hash_names.end() && !it->second.empty())
    return it->second[0].name;

  // Fallback: global hash table from data file
  load_global_hash_table();
  auto git = global_hash_table.find(hash);
  if (git != global_hash_table.end())
    return git->second;

  return {};
}

int64_t find_export_address(const std::string &script_name,
                            const std::string &func_name) {
  return find_export_address_internal(script_name, gsc_hash(func_name));
}

int resolve_hash_line(uint32_t hash, int num_params) {
  auto it = script_hash_names.find(hash);
  if (it != script_hash_names.end()) {
    for (auto &entry : it->second)
      if (entry.params == static_cast<uint8_t>(num_params) && entry.line > 0)
        return entry.line;
  }
  return 0;
}

std::string get_source_line(const std::string &file, int line_num) {
  // Try to find source by matching file path suffix
  for (auto &[path, src] : script_sources) {
    if (file.find(path) != std::string::npos ||
        path.find(file) != std::string::npos ||
        file.find(std::filesystem::path(path).filename().string()) !=
            std::string::npos) {
      if (line_num <= 0)
        return {};
      int current = 1;
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
      auto line = src.substr(start, end - start);
      if (!line.empty() && line.back() == '\r')
        line.pop_back();
      return line;
    }
  }
  return {};
}

struct component final : generic_component {
  void post_unpack() override {
    // Return custom or overrided scripts if found
    db_find_x_asset_header_hook.create(game::select(0x141420ED0, 0x1401D5FB0),
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
    gscr_get_bgb_remaining_hook.create(game::select(0x141A8CAB0, 0x1402D2310),
                                       scr_loot_get_item_quantity_stub);
  }
};
}; // namespace script

REGISTER_COMPONENT(script::component)
