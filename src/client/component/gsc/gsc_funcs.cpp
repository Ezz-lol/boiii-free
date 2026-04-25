#include <atomic>
#include <cstdint>
#include <deque>
#include <mutex>
#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "../game_event.hpp"

#include <string>
#include <unordered_map>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

#include "../command.hpp"

using namespace game::scr;

namespace script {
int64_t find_export_address(const std::string &script_name,
                            const std::string &func_name);
}

namespace gsc_funcs {
namespace {
using vm_opcode_handler_t = void(__fastcall *)(int32_t inst, int64_t *fs_0,
                                               int64_t vmc, bool *terminate);

static std::unordered_map<ScrVarCanonicalName_t, BuiltinFunction>
    custom_builtins;
static std::unordered_map<int64_t, int64_t> function_replacements;
static bool detours_enabled = false;

static vm_opcode_handler_t orig_SafeCreateLocalVariables = nullptr;
static vm_opcode_handler_t orig_CheckClearParams = nullptr;
static std::atomic_bool return_value_set = std::atomic_bool(false);

constexpr uint32_t fnv1a(const char *str) {
  uint32_t hash = 0x811c9dc5;
  while (*str) {
    hash ^= static_cast<uint8_t>(*str++);
    hash *= 0x01000193;
  }
  return hash;
}

void push_string(scriptInstance_t inst, const char *val) {
  Scr_AddString(inst, val);
  return_value_set.store(true);
}

void push_int(scriptInstance_t inst, int val) {
  Scr_AddInt(inst, val);
  return_value_set.store(true);
}

void push_int(scriptInstance_t inst, uint32_t val) {
  Scr_AddInt(inst, static_cast<int32_t>(val));
  return_value_set.store(true);
}

void push_array(scriptInstance_t inst, std::vector<std::string> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); i++) {
    const char *str = arr[i].c_str();
    Scr_AddString(inst, str);
    Scr_AddArray(inst);
  }
  return_value_set.store(true);
}

void push_array(scriptInstance_t inst, std::vector<float> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); i++) {
    Scr_AddFloat(inst, arr[i]);
    Scr_AddArray(inst);
  }
  return_value_set.store(true);
}

void push_array(scriptInstance_t inst, std::vector<const char *> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); i++) {
    Scr_AddString(inst, arr[i]);
    Scr_AddArray(inst);
  }
  return_value_set.store(true);
}

void push_array(scriptInstance_t inst, std::vector<bool> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); i++) {
    Scr_AddInt(inst, arr[i] ? 1 : 0);
    Scr_AddArray(inst);
  }
  return_value_set.store(true);
}

void push_array(scriptInstance_t inst, std::vector<uint32_t> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); i++) {
    Scr_AddInt(inst, static_cast<int32_t>(arr[i]));
    Scr_AddArray(inst);
  }
  return_value_set.store(true);
}

void push_array(scriptInstance_t inst, std::vector<int> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); i++) {
    Scr_AddInt(inst, arr[i]);
    Scr_AddArray(inst);
  }
  return_value_set.store(true);
}

// Push empty array
void push_array(scriptInstance_t inst) {
  Scr_MakeArray(inst);
  return_value_set.store(true);
}

// =====================================================
// Script console commands (addcommand/getcommand)
// =====================================================

std::mutex script_cmd_mutex;
std::vector<std::string> script_cmd_names;
std::deque<std::string> script_cmd_queue;

void script_cmd_handler(const command::params &params) {
  std::string result;
  for (uint32_t i = 0; i < params.size(); i++) {
    if (i > 0)
      result += ' ';
    result += params.get(i);
  }

  std::lock_guard lock(script_cmd_mutex);
  script_cmd_queue.push_back(std::move(result));
}

void clear_script_commands() {
  std::lock_guard lock(script_cmd_mutex);
  script_cmd_queue.clear();
  script_cmd_names.clear();
}

// =====================================================
// Path security for file I/O
// =====================================================

std::filesystem::path get_scriptdata_path() {
  const utils::nt::library host{};
  return host.get_folder() / "boiii" / "scriptdata";
}

bool is_safe_path(const std::string &path) {
  if (path.empty())
    return false;
  if (path.find("..") != std::string::npos)
    return false;
  if (path.size() >= 2 && path[1] == ':')
    return false;
  if (path[0] == '/' || path[0] == '\\')
    return false;
  return true;
}

std::filesystem::path resolve_path(const std::string &path) {
  return get_scriptdata_path() / path;
}

std::filesystem::path relative_path(const std::filesystem::path &full_path) {
  const auto scriptdata = get_scriptdata_path();
  std::string relative = full_path.string();
  if (relative.find(scriptdata.string()) == 0) {
    relative.erase(0, scriptdata.string().length());
  }

  // Remove '/' or '\\' prefix if present
  if (!relative.empty() && (relative[0] == '/' || relative[0] == '\\')) {
    relative.erase(0, 1);
  }

  return relative;
}

// =====================================================
// Opcode hooks for replacefunc
// =====================================================

bool try_redirect(int32_t inst, int64_t *fs_0) {
  if (!detours_enabled || inst != 0)
    return false;
  const auto it = function_replacements.find(*fs_0 - 2);
  if (it != function_replacements.end()) {
    *fs_0 = it->second;
    return true;
  }
  return false;
}

void __fastcall hk_SafeCreateLocalVariables(int32_t inst, int64_t *fs_0,
                                            int64_t vmc, bool *terminate) {
  if (!try_redirect(inst, fs_0) && orig_SafeCreateLocalVariables)
    orig_SafeCreateLocalVariables(inst, fs_0, vmc, terminate);
}

void __fastcall hk_CheckClearParams(int32_t inst, int64_t *fs_0, int64_t vmc,
                                    bool *terminate) {
  if (!try_redirect(inst, fs_0) && orig_CheckClearParams)
    orig_CheckClearParams(inst, fs_0, vmc, terminate);
}

void hook_opcode(uint16_t opcode, vm_opcode_handler_t hook,
                 vm_opcode_handler_t *out_orig) {
  size_t tables[3] = {};
  int count = 0;

  if (game::is_server()) {
    tables[count++] = game::relocate(0x14107C150);
  } else {
    tables[count++] = game::relocate(0x1432E6350);
    tables[count++] = game::relocate(0x143306350);
  }

  for (int i = 0; i < count; i++) {
    if (!tables[i])
      continue;
    int64_t *entry = reinterpret_cast<int64_t *>(tables[i] + opcode * 8);
    if (!*out_orig)
      *out_orig = reinterpret_cast<vm_opcode_handler_t>(*entry);
    if (*entry == reinterpret_cast<int64_t>(*out_orig))
      *entry = reinterpret_cast<int64_t>(hook);
  }
}

void builtin_dispatcher(game::scr::scriptInstance_t inst) {
  return_value_set.store(false);

  try {
    const int32_t hash = Scr_GetInt(inst, 0);
    const auto it = custom_builtins.find(hash);
    if (it != custom_builtins.end())
      it->second(inst);
  } catch (const std::exception &e) {
    printf("^1[builtin] Exception: %s\n", e.what());
  } catch (...) {
    printf("^1[builtin] Unknown exception\n");
  }

  if (!return_value_set.load())
    game::scr::Scr_AddInt(inst, 0);
}

// =====================================================
// Core builtins
// =====================================================

// replacefunc: redirect all calls to target_func to replacement_func
void gscr_replacefunc(scriptInstance_t inst) {
  const char *target_script = Scr_GetString(inst, 1);
  const char *target_func = Scr_GetString(inst, 2);
  const char *replace_script = Scr_GetString(inst, 3);
  const char *replace_func = Scr_GetString(inst, 4);

  if (!target_script || !target_func || !replace_script || !replace_func) {
    printf("^1[replacefunc] Missing parameter(s)\n");
    return;
  }

  const auto target_addr =
      script::find_export_address(target_script, target_func);
  const auto replace_addr =
      script::find_export_address(replace_script, replace_func);

  if (target_addr && replace_addr) {
    function_replacements[target_addr] = replace_addr;
    detours_enabled = true;
  } else {
    printf("^1[replacefunc] FAILED %s::%s -> %s::%s", target_script,
           target_func, replace_script, replace_func);
    if (!target_addr)
      printf(" (target not found)");
    if (!replace_addr)
      printf(" (replacement not found)");
    printf("\n");
  }
}

// clearreplacefuncs: remove all active function replacements
void gscr_clearreplacefuncs([[maybe_unused]] scriptInstance_t inst) {
  if (!function_replacements.empty())
    printf("[clearreplacefuncs] Clearing %zu replacement(s)\n",
           function_replacements.size());
  function_replacements.clear();
  detours_enabled = false;
}

void gscr_println(scriptInstance_t inst) {
  const char *msg = Scr_GetString(inst, 1);
  game::com::Com_Printf(0, 0, "%s\n", msg ? msg : "");
  printf("%s\n", msg ? msg : "");
  fflush(stdout);
}

void gscr_print(scriptInstance_t inst) {
  const char *msg = Scr_GetString(inst, 1);
  game::com::Com_Printf(0, 0, "%s", msg ? msg : "");
  printf("%s", msg ? msg : "");
  fflush(stdout);
}

void gscr_printf(scriptInstance_t inst) {
  const char *format = Scr_GetString(inst, 1);
  if (!format)
    return;
  std::string buffer;

  int32_t arg_index = 2;
  for (size_t i = 0; format[i] != '\0'; i++) {
    if (format[i] == '%' &&
        arg_index <= static_cast<int32_t>(Scr_GetNumParam(inst))) {
      char specifier = format[++i];
      switch (specifier) {
      case 's': {
        const char *arg = Scr_GetString(inst, arg_index);
        if (!arg) {
          Scr_ParamError(inst, arg_index,
                         "Argument to printf is not a string; string expected "
                         "for %s specifier");
          return;
        }
        buffer.insert(buffer.end(), arg, arg + std::strlen(arg));
        break;
      }
      case 'd':
      case 'i': {
        int arg = Scr_GetInt(inst, arg_index);
        buffer += std::to_string(arg);
        break;
      }
      case 'o': {
        int arg = Scr_GetInt(inst, arg_index);
        char conv_buffer[33];
        auto [ptr, ec] = std::to_chars(
            conv_buffer, conv_buffer + sizeof(conv_buffer), arg, 8);
        if (ec == std::errc()) {
          *ptr = '\0';
          buffer += conv_buffer;
        } else {
          Scr_ParamError(inst, arg_index,
                         "Failed to format integer argument for %o specifier");
          return;
        }
        break;
      }
      case 'u': {
        int arg = Scr_GetInt(inst, arg_index);
        buffer += std::to_string(static_cast<uint32_t>(arg));
        break;
      }
      case 'x':
      case 'X': {
        int arg = Scr_GetInt(inst, arg_index);
        char conv_buffer[9];
        auto [ptr, ec] = std::to_chars(
            conv_buffer, conv_buffer + sizeof(conv_buffer), arg, 16);
        if (ec == std::errc()) {
          *ptr = '\0';
          if (specifier == 'X') {
            for (char *p = conv_buffer; *p; p++) {
              *p = static_cast<char>(
                  std::toupper(static_cast<unsigned char>(*p)));
            }
          }
          buffer += conv_buffer;
        } else {
          Scr_ParamError(inst, arg_index,
                         "Failed to format integer argument for %x specifier");
          return;
        }
        break;
      }

      case 'c': {
        int arg = Scr_GetInt(inst, arg_index);
        buffer.push_back(static_cast<char>(arg));
        break;
      }
      case 'f': {
        float arg = Scr_GetFloat(inst, arg_index);
        buffer += std::to_string(arg);
        break;
      }
        /*
         TODO:
         - width and precision specifiers (e.g. %.2f, %5d)
         - %p - is this even possible to support?
         - %n - is this possible?
         - length modifiers for floats (e.g. %Lf)
            - Scr_GetFloat only returns 32-bit float, so we would need to add a
              new function Scr_GetDouble to retrieve 64-bit double arguments
         - length modifiers for integers (e.g. %lld, %hhd)
             1. Scr_GetInt only returns 32-bit int, so we would need to add new
                functions Scr_GetInt64 and Scr_GetInt8 to retrieve 64-bit and
                8-bit integer arguments, respectively
              2. We would also need to modify the argument parsing logic to
                 determine which Scr_Get function to call based on the length
                 modifier in the format string
          - handle %% for literal % character
        */

        // Either not a specifier or unsupported. Just treat it as a normal %
        // character and continue.
      default:
        buffer.push_back('%');
        buffer.push_back(specifier);
      }
      arg_index++;
    } else {
      buffer.push_back(format[i]);
    }
  }

  game::com::Com_Printf(0, 0, "%s", buffer.data());
  printf("%s", buffer.data());
  fflush(stdout);
}

void gscr_executecommand(scriptInstance_t inst) {
  const char *cmd = Scr_GetString(inst, 1);
  if (cmd)
    game::cbuf::Cbuf_AddText(0, utils::string::va("%s\n", cmd));
}

// addcommand("name") - registers a console command that GSC can read via
// getcommand() and you are free to to whatever you with it once you detect
// the command
void gscr_addcommand(scriptInstance_t inst) {
  const char *name = Scr_GetString(inst, 1);
  if (!name || !name[0])
    return;

  const std::string cmd_name(name);
  {
    std::lock_guard lock(script_cmd_mutex);
    for (const auto &existing : script_cmd_names) {
      if (existing == cmd_name)
        return; // Already registered
    }
    script_cmd_names.push_back(cmd_name);
  }

  command::add(cmd_name, [](const command::params &params) {
    script_cmd_handler(params);
  });
}

// getcommand() - returns the next queued command string, or "" if none
void gscr_getcommand(scriptInstance_t inst) {
  std::lock_guard lock(script_cmd_mutex);
  if (script_cmd_queue.empty()) {
    push_string(inst, "");
    return;
  }
  auto cmd = std::move(script_cmd_queue.front());
  script_cmd_queue.pop_front();
  push_string(inst, cmd.c_str());
}

// say: broadcast a chat message to all players
// GSC: say("Hello world");
void gscr_say(scriptInstance_t inst) {
  const char *msg = Scr_GetString(inst, 1);
  if (msg)
    game::sv::SV_GameSendServerCommand(
        -1, game::net::SV_CMD_CAN_IGNORE_0,
        utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
}

// tell: send a private chat message to a specific client
// GSC: player tell("Hello");
void gscr_tell(scriptInstance_t inst) {
  const int32_t client_num = Scr_GetInt(inst, 1);
  const char *msg = Scr_GetString(inst, 2);
  if (client_num >= 0 && client_num < 18 && msg)
    game::sv::SV_GameSendServerCommand(
        client_num, game::net::SV_CMD_CAN_IGNORE_0,
        utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
}

// =====================================================
// File I/O builtins, paths relative to boiii/scriptdata/
// =====================================================

void gscr_writefile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  const char *data = Scr_GetString(inst, 2);

  if (!path || !data || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }

  const std::filesystem::path full = resolve_path(path);
  const std::filesystem::path parent = full.parent_path();
  if (!parent.empty()) {
    utils::io::create_directory(parent);
  }
  bool append = Scr_GetBoolOptional(inst, 3, false);

  bool result = utils::io::write_file(full.string(), data, append);
  push_int(inst, result ? 1 : 0);
}

void gscr_readfile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_string(inst, "");
    return;
  }
  const std::filesystem::path full = resolve_path(path);
  std::string data;
  if (utils::io::read_file(full.string(), &data))
    push_string(inst, data.c_str());
  else
    push_string(inst, "");
}

void gscr_appendfile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  const char *data = Scr_GetString(inst, 2);
  if (!path || !data || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  const std::filesystem::path full = resolve_path(path);
  const std::filesystem::path parent = full.parent_path();
  if (!parent.empty())
    utils::io::create_directory(parent);
  push_int(inst, utils::io::write_file(full.string(), data, true) ? 1 : 0);
}

void gscr_fileexists(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  push_int(inst, utils::io::file_exists(resolve_path(path).string()) ? 1 : 0);
}

void gscr_removefile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  push_int(inst, utils::io::remove_file(resolve_path(path)) ? 1 : 0);
}

void gscr_removedirectory(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  push_int(inst, utils::io::remove_directory(resolve_path(path), true) ? 1 : 0);
}

void gscr_rm(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }

  bool recurse = Scr_GetBoolOptional(inst, 2, false);

  if (utils::io::directory_exists(resolve_path(path))) {
    push_int(inst,
             utils::io::remove_directory(resolve_path(path), recurse) ? 1 : 0);
  } else {
    push_int(inst, utils::io::remove_file(resolve_path(path)) ? 1 : 0);
  }
}

void gscr_filesize(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  push_int(inst,
           static_cast<int>(utils::io::file_size(resolve_path(path).string())));
}

void gscr_createdirectory(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }

  bool result = utils::io::create_directory(resolve_path(path));
  push_int(inst, result ? 1 : 0);
}

void gscr_directoryexists(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  push_int(inst, utils::io::directory_exists(resolve_path(path)) ? 1 : 0);
}

void gscr_listfiles(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_string(inst, "");
    return;
  }
  const std::filesystem::path full = resolve_path(path);
  if (!utils::io::directory_exists(full)) {
    push_string(inst, "");
    return;
  }
  const std::vector<std::filesystem::path> files = utils::io::list_files(full);
  std::string result;
  for (const auto &f : files) {
    if (!result.empty())
      result += ",";
    result += f.filename().string();
  }
  push_string(inst, result.c_str());
}

/*
 ls(path, recurse = false, include_directories = false)
 Lists files in a directory, optionally recursively and including directories.
 Returns an array of file/directory paths.
*/
void gscr_ls(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_array(inst);
    return;
  }

  bool recurse = Scr_GetBoolOptional(inst, 2, false);
  bool include_directories = Scr_GetBoolOptional(inst, 3, false);

  const std::filesystem::path full = resolve_path(path);
  if (!utils::io::directory_exists(full)) {
    push_array(inst);
    return;
  }

  const std::vector<std::filesystem::path> entries =
      utils::io::list_files(full, recurse, include_directories);

  std::vector<std::string> str_entries;
  str_entries.reserve(entries.size());
  std::transform(
      entries.begin(), entries.end(), std::back_inserter(str_entries),
      [](const std::filesystem::path &p) { return relative_path(p).string(); });
  push_array(inst, std::move(str_entries));
}

// =====================================================
// JSON builtins, simple string-based operations
// =====================================================

void gscr_jsonvalid(scriptInstance_t inst) {
  const char *json_str = Scr_GetString(inst, 1);
  if (!json_str) {
    push_int(inst, 0);
    return;
  }
  rapidjson::Document doc;
  doc.Parse(json_str);
  push_int(inst, doc.HasParseError() ? 0 : 1);
}

// jsonparse(json_string, key), returns value as string
void gscr_jsonparse(scriptInstance_t inst) {
  const char *json_str = Scr_GetString(inst, 1);
  const char *key = Scr_GetString(inst, 2);
  if (!json_str || !key) {
    push_string(inst, "");
    return;
  }
  rapidjson::Document doc;
  doc.Parse(json_str);
  if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember(key)) {
    push_string(inst, "");
    return;
  }
  const auto &val = doc[key];
  if (val.IsString())
    push_string(inst, val.GetString());
  else if (val.IsInt())
    push_string(inst, std::to_string(val.GetInt()).c_str());
  else if (val.IsInt64())
    push_string(inst, std::to_string(val.GetInt64()).c_str());
  else if (val.IsDouble())
    push_string(inst, std::to_string(val.GetDouble()).c_str());
  else if (val.IsBool())
    push_string(inst, val.GetBool() ? "true" : "false");
  else if (val.IsNull())
    push_string(inst, "null");
  else {
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    val.Accept(writer);
    push_string(inst, buf.GetString());
  }
}

// jsonset(json_string, key, value_string), sets key, returns modified json
void gscr_jsonset(scriptInstance_t inst) {
  const char *json_str = Scr_GetString(inst, 1);
  const char *key = Scr_GetString(inst, 2);
  const char *val_str = Scr_GetString(inst, 3);
  if (!json_str || !key || !val_str) {
    push_string(inst, json_str ? json_str : "{}");
    return;
  }
  rapidjson::Document doc;
  doc.Parse(json_str);
  if (doc.HasParseError() || !doc.IsObject())
    doc.SetObject();

  auto &alloc = doc.GetAllocator();

  // try to parse the value as JSON first
  rapidjson::Document val_doc;
  val_doc.Parse(val_str);

  rapidjson::Value k(key, alloc);

  if (doc.HasMember(key))
    doc.RemoveMember(key);

  if (val_doc.HasParseError()) {
    rapidjson::Value v(val_str, alloc);
    doc.AddMember(k, v, alloc);
  } else {
    rapidjson::Value v;
    v.CopyFrom(val_doc, alloc);
    doc.AddMember(k, v, alloc);
  }

  rapidjson::StringBuffer buf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
  doc.Accept(writer);
  push_string(inst, buf.GetString());
}

// jsondump(filepath, json_string), writes json to file
void gscr_jsondump(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  const char *json_str = Scr_GetString(inst, 2);
  if (!path || !json_str || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  const std::filesystem::path full = resolve_path(path);
  const std::filesystem::path parent = full.parent_path();
  if (!parent.empty())
    utils::io::create_directory(parent);
  push_int(inst, utils::io::write_file(full.string(), json_str) ? 1 : 0);
}

// =====================================================
// Int64 builtins, string-based 64-bit arithmetic
// =====================================================

int64_t parse_int64_arg(scriptInstance_t inst, unsigned int index) {
  const char *str = Scr_GetString(inst, index);
  if (str && str[0])
    return std::strtoll(str, nullptr, 0);
  return static_cast<int64_t>(Scr_GetInt(inst, index));
}

void gscr_int64_op(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 1);
  const char *op = Scr_GetString(inst, 2);
  const int64_t b = parse_int64_arg(inst, 3);

  if (!op) {
    push_string(inst, "0");
    return;
  }

  const std::string op_str(op);
  int64_t result = 0;
  bool is_comparison = false;
  bool cmp_result = false;

  if (op_str == "+")
    result = a + b;
  else if (op_str == "-")
    result = a - b;
  else if (op_str == "*")
    result = a * b;
  else if (op_str == "/") {
    result = (b != 0) ? a / b : 0;
  } else if (op_str == "%") {
    result = (b != 0) ? a % b : 0;
  } else if (op_str == "&")
    result = a & b;
  else if (op_str == "|")
    result = a | b;
  else if (op_str == "^")
    result = a ^ b;
  else if (op_str == "~")
    result = ~a;
  else if (op_str == "<<")
    result = a << b;
  else if (op_str == ">>")
    result = a >> b;
  else if (op_str == "++")
    result = a + 1;
  else if (op_str == "--")
    result = a - 1;
  else if (op_str == ">") {
    is_comparison = true;
    cmp_result = a > b;
  } else if (op_str == ">=") {
    is_comparison = true;
    cmp_result = a >= b;
  } else if (op_str == "<") {
    is_comparison = true;
    cmp_result = a < b;
  } else if (op_str == "<=") {
    is_comparison = true;
    cmp_result = a <= b;
  } else if (op_str == "==") {
    is_comparison = true;
    cmp_result = a == b;
  } else if (op_str == "!=") {
    is_comparison = true;
    cmp_result = a != b;
  } else {
    push_string(inst, "0");
    return;
  }

  if (is_comparison)
    push_int(inst, cmp_result ? 1 : 0);
  else
    push_string(inst, std::to_string(result).c_str());
}

void gscr_int64_isint(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 1);
  push_int(inst, (val >= INT32_MIN && val <= INT32_MAX) ? 1 : 0);
}

void gscr_int64_toint(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 1);
  push_int(inst, static_cast<int>(val));
}

void gscr_int64_min(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 1);
  const int64_t b = parse_int64_arg(inst, 2);
  push_string(inst, std::to_string(std::min(a, b)).c_str());
}

void gscr_int64_max(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 1);
  const int64_t b = parse_int64_arg(inst, 2);
  push_string(inst, std::to_string(std::max(a, b)).c_str());
}

void gscr_int64_abs(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 1);
  push_string(inst, std::to_string(a < 0 ? -a : a).c_str());
}

void gscr_int64_clamp(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 1);
  const int64_t lo = parse_int64_arg(inst, 2);
  const int64_t hi = parse_int64_arg(inst, 3);
  push_string(inst, std::to_string(std::clamp(val, lo, hi)).c_str());
}

void gscr_int64_tostring(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 1);
  push_string(inst, std::to_string(val).c_str());
}

void gscr_getfunction(scriptInstance_t inst) {
  const char *script_name = Scr_GetString(inst, 1);
  const char *func_name = Scr_GetString(inst, 2);
  if (!script_name || !func_name) {
    push_int(inst, 0);
    return;
  }
  const int64_t addr = script::find_export_address(script_name, func_name);
  push_int(inst, addr ? 1 : 0);
}
} // namespace

void add_detour(int64_t target_addr, int64_t replacement_addr) {
  function_replacements[target_addr] = replacement_addr;
  detours_enabled = true;
}

struct component final : generic_component {
  void post_unpack() override {

    // Core
    custom_builtins[fnv1a("replacefunc")] = gscr_replacefunc;
    custom_builtins[fnv1a("executecommand")] = gscr_executecommand;
    custom_builtins[fnv1a("say")] = gscr_say;
    custom_builtins[fnv1a("tell")] = gscr_tell;
    custom_builtins[fnv1a("println")] = gscr_println;
    custom_builtins[fnv1a("print")] = gscr_print;
    custom_builtins[fnv1a("printf")] = gscr_printf;

    // File I/O
    custom_builtins[fnv1a("writefile")] = gscr_writefile;
    custom_builtins[fnv1a("readfile")] = gscr_readfile;
    custom_builtins[fnv1a("appendfile")] = gscr_appendfile;
    custom_builtins[fnv1a("fileexists")] = gscr_fileexists;
    custom_builtins[fnv1a("removefile")] = gscr_removefile;
    custom_builtins[fnv1a("removedirectory")] = gscr_removedirectory;
    custom_builtins[fnv1a("rmdir")] = gscr_removedirectory;
    custom_builtins[fnv1a("rm")] = gscr_rm;
    custom_builtins[fnv1a("filesize")] = gscr_filesize;
    custom_builtins[fnv1a("createdirectory")] = gscr_createdirectory;
    custom_builtins[fnv1a("mkdir")] = gscr_createdirectory;
    custom_builtins[fnv1a("directoryexists")] = gscr_directoryexists;
    custom_builtins[fnv1a("listfiles")] = gscr_listfiles;
    custom_builtins[fnv1a("ls")] = gscr_ls;

    // JSON
    custom_builtins[fnv1a("jsonvalid")] = gscr_jsonvalid;
    custom_builtins[fnv1a("jsonparse")] = gscr_jsonparse;
    custom_builtins[fnv1a("jsonset")] = gscr_jsonset;
    custom_builtins[fnv1a("jsondump")] = gscr_jsondump;

    // Int64
    custom_builtins[fnv1a("int64_op")] = gscr_int64_op;
    custom_builtins[fnv1a("int64_isint")] = gscr_int64_isint;
    custom_builtins[fnv1a("int64_toint")] = gscr_int64_toint;
    custom_builtins[fnv1a("int64_min")] = gscr_int64_min;
    custom_builtins[fnv1a("int64_max")] = gscr_int64_max;
    custom_builtins[fnv1a("int64_abs")] = gscr_int64_abs;
    custom_builtins[fnv1a("int64_clamp")] = gscr_int64_clamp;
    custom_builtins[fnv1a("int64_tostring")] = gscr_int64_tostring;

    // Function lookup
    custom_builtins[fnv1a("getfunction")] = gscr_getfunction;

    // Console commands
    custom_builtins[fnv1a("addcommand")] = gscr_addcommand;
    custom_builtins[fnv1a("getcommand")] = gscr_getcommand;

    // Utility
    custom_builtins[fnv1a("clearreplacefuncs")] = gscr_clearreplacefuncs;

    auto *builtin_def = reinterpret_cast<BuiltinFunctionDef *>(
        game::select(0x1432D7D70, 0x14106DD70));
    builtin_def->max_args = 255;
    builtin_def->actionFunc = reinterpret_cast<void *>(builtin_dispatcher);

    hook_opcode(0x01D2, hk_SafeCreateLocalVariables,
                &orig_SafeCreateLocalVariables);
    hook_opcode(0x000D, hk_CheckClearParams, &orig_CheckClearParams);

    game_event::on_g_shutdown_game([] {
      if (!function_replacements.empty())
        printf("[gsc] Clearing %zu replacefunc(s) on map shutdown\n",
               function_replacements.size());
      function_replacements.clear();
      detours_enabled = false;
      clear_script_commands();
    });

    game_event::on_g_init_game([] {
      if (!function_replacements.empty())
        printf("[gsc] Clearing %zu stale replacefunc(s) on map init\n",
               function_replacements.size());
      function_replacements.clear();
      detours_enabled = false;
    });
  }
};
} // namespace gsc_funcs

REGISTER_COMPONENT(gsc_funcs::component)