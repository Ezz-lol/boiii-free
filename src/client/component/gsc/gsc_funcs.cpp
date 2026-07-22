#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/utils.hpp>
#include "../game_event.hpp"
#include "../name.hpp"

#include <optional>
#include <string>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

#include "../command.hpp"
#include "gsc_funcs.hpp"

using namespace game;
using namespace game::scr;

namespace gsc::custom_builtins {
CustomBuiltinMap<BuiltinFunctionDef> functions;
CustomBuiltinMap<BuiltinMethodDef> methods;
} // namespace gsc::custom_builtins

namespace script {
uint8_t *find_export_address(const std::string &script_name,
                             const std::string &func_name,
                             int expected_params = -1);
} // namespace script

namespace gsc_funcs {
using namespace gsc;
namespace {

static std::unordered_map<uint8_t *, uint8_t *> function_replacements;
static std::unordered_map<game::ClientNum_t, std::unordered_set<std::string>>
    client_dvar_changes;
static std::atomic_bool detours_enabled = false;

static vm::op::VM_OP_FUNC_PTR VM_OP_SafeCreateLocalVariables_Handler_orig =
    nullptr;
static vm::op::VM_OP_FUNC_PTR VM_OP_CheckClearParams_Handler_orig = nullptr;

// =====================================================
// Script console commands (addcommand/getcommand)
// =====================================================

std::mutex script_cmd_mutex;
std::vector<std::string> script_cmd_names;
std::deque<std::string> script_cmd_queue;

std::string normalize_command_name(std::string value) {
  std::transform(
      value.begin(), value.end(), value.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

std::string extract_command_name(const std::string &value) {
  const size_t start = value.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return {};
  }

  const size_t end = value.find_first_of(" \t", start);
  const std::string name = value.substr(start, end - start);
  return normalize_command_name(name);
}

void script_cmd_handler(const command::params &params) {
  std::string result;
  for (int32_t i = 0; i < params.size(); i++) {
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

std::string trim_copy(std::string value) {
  const size_t start = value.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return {};
  }

  const size_t end = value.find_last_not_of(" \t");
  return value.substr(start, end - start + 1);
}

std::optional<std::string> extract_dvar_name(const char *dvar_cmd) {
  if (!dvar_cmd || !dvar_cmd[0]) {
    return {};
  }

  const std::string trimmed = trim_copy(dvar_cmd);
  if (trimmed.empty()) {
    return {};
  }

  std::function<std::string(size_t &)> next_token =
      [&](size_t &cursor) -> std::string {
    while (cursor < trimmed.size() &&
           (trimmed[cursor] == ' ' || trimmed[cursor] == '\t')) {
      cursor++;
    }

    const size_t start = cursor;
    while (cursor < trimmed.size() && trimmed[cursor] != ' ' &&
           trimmed[cursor] != '\t') {
      cursor++;
    }

    return trimmed.substr(start, cursor - start);
  };

  size_t cursor = 0;
  std::string first = next_token(cursor);
  if (first.empty()) {
    return {};
  }

  const std::string lowered = normalize_command_name(first);
  if (lowered == "set" || lowered == "seta" || lowered == "sets" ||
      lowered == "reset") {
    std::string second = next_token(cursor);
    if (!second.empty()) {
      return second;
    }
  }

  return first;
}

void reset_tracked_client_dvars() {
  for (const std::pair<const game::ClientNum_t, std::unordered_set<std::string>>
           &pair : client_dvar_changes) {
    const game::ClientNum_t &client_num = pair.first;
    const std::unordered_set<std::string> &dvars = pair.second;

    if (game::valid_client_num(client_num)) {

      for (const std::string &dvar_name : dvars) {
        game::sv::SV_GameSendServerCommand(
            client_num, game::net::SV_CMD_CAN_IGNORE_0,
            utils::string::va("c \"reset %s\"", dvar_name.c_str()));
      }
    }
  }
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
  const std::filesystem::path scriptdata = get_scriptdata_path();
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
// HUD text state (server-only, safe setText hooks)
// =====================================================

utils::hook::detour hecmd_settext_hook;
utils::hook::detour g_tagindex_hook;

std::mutex hud_text_mutex;
static std::unordered_map<uint32_t, int> hudelem_cfgstr_map;
int localized_cfgstr_base = -1;

thread_local int last_cfgstr_result = -1;
thread_local int last_cfgstr_start = -1;
thread_local int last_cfgstr_max = -1;

int g_tagindex_stub(const char *string, int start, int max, int create,
                    const char *errormsg) {
  int result =
      g_tagindex_hook.invoke<int>(string, start, max, create, errormsg);
  last_cfgstr_start = start;
  last_cfgstr_max = max;
  last_cfgstr_result = result;
  return result;
}

void hecmd_settext_stub(game::scr::scriptInstance_t inst,
                        game::scr::scr_entref_t *entref) {
  const uint32_t he_idx = entref->u.hudElemIndex;

  {
    std::lock_guard lock(hud_text_mutex);
    if (hudelem_cfgstr_map.contains(he_idx)) {
      const char *text = game::scr::Scr_GetString(inst, 1);
      if (text) {
        const int cfg_idx = hudelem_cfgstr_map[he_idx];
        const int start = last_cfgstr_start;
        const int max = last_cfgstr_max;
        const bool range_ok = (start >= 0 && max > 0 && cfg_idx >= start &&
                               cfg_idx < (start + max));
        if (cfg_idx >= 0 && range_ok && game::sv::SV_Loaded()) {
          game::sv::SV_SetConfigstring(cfg_idx, text);
          return;
        }
      }
    }
  }

  last_cfgstr_result = -1;
  last_cfgstr_start = -1;
  last_cfgstr_max = -1;
  hecmd_settext_hook.invoke<void>(inst, entref);

  {
    std::lock_guard lock(hud_text_mutex);
    if (last_cfgstr_result >= 0) {
      hudelem_cfgstr_map[he_idx] = last_cfgstr_result;
      if (last_cfgstr_start >= 0)
        localized_cfgstr_base = last_cfgstr_start;
    }
  }
}

void clear_hud_text_state() {
  std::lock_guard lock(hud_text_mutex);
  hudelem_cfgstr_map.clear();
  localized_cfgstr_base = -1;
}

static std::atomic_bool settext_hooks_installed = false;

void install_settext_hooks() {
  if (!settext_hooks_installed.load(std::memory_order_seq_cst) &&
      game::is_server()) {

    /*
     TODO: this needs to be reviewed and fixed ASAP.

     The function signature used in G_TagIndex's hook is incorrect.
     G_TagIndex takes only one argument: `const char* name`.
     It has been verified that no further arguments are used in the function.

     Access and usage of the values of any further arguments should be
     considered unsafe, undefined behaviour.
    */
    g_tagindex_hook.create(game::G_TagIndex.get(), g_tagindex_stub);
    hecmd_settext_hook.create(game::scr::cmd::he::HECmd_SetText.get(),
                              hecmd_settext_stub);
    settext_hooks_installed.store(true, std::memory_order_seq_cst);
  }
}

void remove_settext_hooks() {
  if (settext_hooks_installed.load(std::memory_order_seq_cst)) {
    g_tagindex_hook.clear();
    hecmd_settext_hook.clear();
    settext_hooks_installed.store(false, std::memory_order_seq_cst);
  }
}

// =====================================================
// Opcode hooks for replacefunc
// =====================================================

bool try_redirect(scriptInstance_t inst, vm::function_stack_t *fs) {
  if (detours_enabled.load(std::memory_order_seq_cst) &&
      inst == SCRIPTINSTANCE_SERVER) {
    uint8_t *redirected = fs->pos - 2;
    if (function_replacements.contains(redirected)) {
      fs->pos = function_replacements[redirected];
      return true;
    }
  }
  return false;
}

void VM_OP_SafeCreateLocalVariables_Handler_stub(
    scriptInstance_t inst, vm::function_stack_t *fs,
    volatile vm::ScrVmContext_t *vmc, bool *terminate) {
  if (!try_redirect(inst, fs) && VM_OP_SafeCreateLocalVariables_Handler_orig)
    VM_OP_SafeCreateLocalVariables_Handler_orig(inst, fs, vmc, terminate);
}

void VM_OP_CheckClearParams_Handler_stub(scriptInstance_t inst,
                                         vm::function_stack_t *fs,
                                         volatile vm::ScrVmContext_t *vmc,
                                         bool *terminate) {
  if (!try_redirect(inst, fs) && VM_OP_CheckClearParams_Handler_orig)
    VM_OP_CheckClearParams_Handler_orig(inst, fs, vmc, terminate);
}

void hook_opcode(vm::op::OP_TYPE opcode, vm::op::VM_OP_FUNC_PTR hook,
                 vm::op::VM_OP_FUNC_PTR *out_orig) {
  vm::op::VM_OP_FUNC_PTR *handler = vm::op::op_handler(opcode);
  if (!*out_orig)
    *out_orig = *handler;
  if (*handler == *out_orig)
    *handler = hook;
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
  const uint32_t argc = game::scr::Scr_GetNumParam(inst);
  const int target_params = argc >= 6 ? Scr_GetInt(inst, 5) : -1;
  const int replace_params = argc >= 7 ? Scr_GetInt(inst, 6) : -1;

  if (!target_script || !target_func || !replace_script || !replace_func) {
    return;
  }

  uint8_t *target_addr =
      script::find_export_address(target_script, target_func, target_params);
  uint8_t *replace_addr =
      script::find_export_address(replace_script, replace_func, replace_params);

  if (target_addr && replace_addr) {
    function_replacements[target_addr] = replace_addr;
    detours_enabled.store(true, std::memory_order_seq_cst);
  } else {
    fprintf(stderr, "[replacefunc] failed %s::%s -> %s::%s", target_script,
            target_func, replace_script, replace_func);
    if (!target_addr) {
      fprintf(stderr, " (target not found)");
    }

    if (!replace_addr) {
      fprintf(stderr, " (replacement not found)");
    }
    fprintf(stderr, "\n");
    fflush(stderr);
  }
}

// clearreplacefuncs: remove all active function replacements
void gscr_clearreplacefuncs([[maybe_unused]] scriptInstance_t inst) {
  function_replacements.clear();
  detours_enabled.store(false, std::memory_order_seq_cst);
}

void gscr_println(scriptInstance_t inst) {
  const char *msg = Scr_GetString(inst, 1);
  game::com::Com_Printf(0, game::consoleLabel_e::DEFAULT, "%s\n",
                        msg ? msg : "");
  fprintf(stdout, "%s\n", msg ? msg : "");
  fflush(stdout);
}

void gscr_print(scriptInstance_t inst) {
  const char *msg = Scr_GetString(inst, 1);
  game::com::Com_Printf(0, game::consoleLabel_e::DEFAULT, "%s", msg ? msg : "");
  fprintf(stdout, "%s", msg ? msg : "");
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

  game::com::Com_Printf(0, game::consoleLabel_e::DEFAULT, "%s", buffer.data());
  fprintf(stdout, "%s", buffer.data());
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
  const std::string cmd_key = normalize_command_name(cmd_name);
  {
    std::lock_guard lock(script_cmd_mutex);
    for (const std::string &existing : script_cmd_names) {
      if (existing == cmd_key)
        return; // Already registered
    }
    script_cmd_names.push_back(cmd_key);
  }

  command::add(cmd_name, [](const command::params &params) {
    script_cmd_handler(params);
  });
}

// getcommand("name") - returns the next queued command for that name, or ""
// getcommand() - returns the next queued command string regardless of name
void gscr_getcommand(scriptInstance_t inst) {
  std::lock_guard lock(script_cmd_mutex);
  const uint32_t argc = Scr_GetNumParam(inst);
  if (argc >= 1) {
    const char *requested = Scr_GetString(inst, 1);
    const std::string requested_name =
        requested ? normalize_command_name(requested) : std::string{};

    if (!requested_name.empty()) {
      for (auto it = script_cmd_queue.begin(); it != script_cmd_queue.end();
           ++it) {
        if (extract_command_name(*it) == requested_name) {
          const std::string cmd = *it;
          script_cmd_queue.erase(it);
          push_string(inst, cmd.c_str());
          return;
        }
      }

      push_string(inst, "");
      return;
    }
  }

  if (script_cmd_queue.empty()) {
    push_string(inst, "");
    return;
  }

  std::string cmd = std::move(script_cmd_queue.front());
  script_cmd_queue.pop_front();
  push_string(inst, cmd.c_str());
}

// say: broadcast a chat message to all players
// GSC: say("Hello world");
void gscr_say(scriptInstance_t inst) {
  const char *msg = Scr_GetString(inst, 1);
  if (msg)
    game::sv::SV_GameSendServerCommand(
        game::INVALID_CLIENT_INDEX, game::net::SV_CMD_CAN_IGNORE_0,
        utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
}

namespace gscr_tell {

// tell: send a private chat message to a specific client
// GSC: player tell("Hello");
void method(scriptInstance_t inst, scr_entref_t *entref) {
  const game::ClientNum_t client_num =
      static_cast<game::ClientNum_t>(Scr_GetInt(inst, 1));
  const char *msg = Scr_GetString(inst, 2);
  if (game::valid_client_num(client_num) && msg)
    game::sv::SV_GameSendServerCommand(
        client_num, game::net::SV_CMD_CAN_IGNORE_0,
        utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
}

// tell: send a private chat message to a specific client
// GSC: player tell("Hello");
void func(scriptInstance_t inst) {
  scr_entref_t entref;
  Scr_GetEntityRef(&entref, inst, 0);
  method(inst, &entref);
}
} // namespace gscr_tell

// =====================================================
// File I/O builtins, paths relative to boiii/scriptdata/
// =====================================================

void gscr_writefile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  const char *data = Scr_GetString(inst, 2);

  if (!path || !data || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }

  const std::filesystem::path full = resolve_path(path);
  const std::filesystem::path parent = full.parent_path();
  if (!parent.empty()) {
    utils::io::create_directory(parent);
  }
  bool append = Scr_GetBoolOptional(inst, 3, false);

  qboolean result =
      qboolean::from(utils::io::write_file(full.string(), data, append));
  push(inst, result);
}

void gscr_readfile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push_string(inst, "");
    return;
  }
  const std::filesystem::path full = resolve_path(path);
  std::string data;
  if (utils::io::read_file(full.string(), &data)) {
    push_string(inst, data.c_str());
  } else {
    push_string(inst, "");
  }
}

void gscr_appendfile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  const char *data = Scr_GetString(inst, 2);
  if (!path || !data || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }
  const std::filesystem::path full = resolve_path(path);
  const std::filesystem::path parent = full.parent_path();
  if (!parent.empty())
    utils::io::create_directory(parent);
  push(inst, utils::io::write_file(full.string(), data, true));
}

void gscr_fileexists(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }
  push(inst, utils::io::file_exists(resolve_path(path).string()));
}

void gscr_removefile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }
  push(inst, utils::io::remove_file(resolve_path(path)));
}

void gscr_removedirectory(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }
  push(inst, utils::io::remove_directory(resolve_path(path), true));
}

void gscr_rm(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }

  bool recurse = Scr_GetBoolOptional(inst, 2, false);

  if (utils::io::directory_exists(resolve_path(path))) {
    push(inst, utils::io::remove_directory(resolve_path(path), recurse));
  } else {
    push(inst, utils::io::remove_file(resolve_path(path)));
  }
}

void gscr_filesize(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }
  push(inst,
       static_cast<int>(utils::io::file_size(resolve_path(path).string())));
}

void gscr_createdirectory(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }

  bool result = utils::io::create_directory(resolve_path(path));
  push(inst, result);
}

void gscr_directoryexists(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 1);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }
  push(inst, utils::io::directory_exists(resolve_path(path)));
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
  for (const std::filesystem::path &f : files) {
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
  std::transform(entries.begin(), entries.end(),
                 std::back_inserter(str_entries),
                 [](const std::filesystem::path &p) -> std::string {
                   return relative_path(p).string();
                 });
  push_array(inst, std::move(str_entries));
}

// =====================================================
// JSON builtins, simple string-based operations
// =====================================================

void gscr_jsonvalid(scriptInstance_t inst) {
  const char *json_str = Scr_GetString(inst, 1);
  if (!json_str) {
    push(inst, 0);
    return;
  }
  rapidjson::Document doc;
  doc.Parse(json_str);
  push(inst, doc.HasParseError() ? 0 : 1);
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
    push(inst, 0);
    return;
  }
  const std::filesystem::path full = resolve_path(path);
  const std::filesystem::path parent = full.parent_path();
  if (!parent.empty())
    utils::io::create_directory(parent);
  push(inst, utils::io::write_file(full.string(), json_str));
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
    push(inst, cmp_result);
  else
    push_string(inst, std::to_string(result).c_str());
}

void gscr_int64_isint(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 1);
  push(inst, (val >= INT32_MIN && val <= INT32_MAX));
}

void gscr_int64_toint(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 1);
  push(inst, static_cast<int>(val));
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
    push(inst, 0);
    return;
  }
  uint8_t *addr = script::find_export_address(script_name, func_name);
  push(inst, reinterpret_cast<int64_t>(addr));
}

void gscr_conststring(scriptInstance_t inst) {
  const uint32_t argc = Scr_GetNumParam(inst);
  if (argc == 0) {
    Scr_ParamError(inst, 1, "No hash argument provided to conststring.");
  } else {
    const ScrString_t hash = static_cast<ScrString_t>(Scr_GetInt(inst, 1));
    push_conststring(inst, hash);
  }
}

// =====================================================
// Player name/tag overrides (server-only)
// =====================================================

std::optional<game::ClientNum_t>
get_self_client_num(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  const game::ClientNum_t client_num =
      static_cast<game::ClientNum_t>(entref->u.entnum);
  if (!game::valid_client_num(client_num)) {
    return std::nullopt;
  }
  return client_num;
}

namespace gscr_setname {
void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  game::ClientNum_t client_num = game::INVALID_CLIENT_INDEX;
  const char *player_name = nullptr;

  const uint32_t argc = game::scr::Scr_GetNumParam(inst);
  if (argc == 1) {
    const std::optional<game::ClientNum_t> self =
        get_self_client_num(inst, entref);
    player_name = game::scr::Scr_GetString(inst, 1);
    if (!self.has_value() || !player_name) {
      Scr_ParamError(inst, 1, "^1[setname] Invalid arguments\n");
      return;
    }
    client_num = static_cast<game::ClientNum_t>(self.value());
  } else {
    client_num = static_cast<game::ClientNum_t>(game::scr::Scr_GetInt(inst, 1));
    player_name = game::scr::Scr_GetString(inst, 2);
    if (!game::valid_client_num(client_num) || !player_name) {
      Scr_ParamError(inst, 1, "^1[setname] Invalid arguments\n");
      return;
    }
  }

  name::set_name_override(client_num, player_name);
  name::sync_name_override_to_clients(client_num);
  name::trigger_client_update(client_num);
}

void func(scriptInstance_t inst) {
  scr_entref_t entref;
  Scr_GetEntityRef(&entref, inst, 0);
  method(inst, &entref);
}
} // namespace gscr_setname

namespace gscr_settag {
void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  game::ClientNum_t client_num = game::INVALID_CLIENT_INDEX;
  const char *tag = nullptr;

  const uint32_t argc = game::scr::Scr_GetNumParam(inst);
  if (argc == 1) {
    const std::optional<game::ClientNum_t> self =
        get_self_client_num(inst, entref);
    tag = game::scr::Scr_GetString(inst, 1);
    if (!self.has_value() || !tag) {
      Scr_ParamError(inst, 1, "^1[settag] Invalid arguments\n");
      return;
    }
    client_num = static_cast<game::ClientNum_t>(self.value());
  } else {
    client_num = static_cast<game::ClientNum_t>(game::scr::Scr_GetInt(inst, 1));
    tag = game::scr::Scr_GetString(inst, 2);
    if (!game::valid_client_num(client_num) || !tag) {
      Scr_ParamError(inst, 1, "^1[settag] Invalid arguments\n");
      return;
    }
  }

  name::set_clan_abbrev_override(client_num, tag);
  name::sync_clan_abbrev_override_to_clients(client_num);
  name::trigger_client_update(client_num);
}

void func(scriptInstance_t inst) {
  scr_entref_t entref;
  Scr_GetEntityRef(&entref, inst, 0);
  method(inst, &entref);
}
} // namespace gscr_settag

namespace gscr_resetname {
void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  game::ClientNum_t client_num = game::INVALID_CLIENT_INDEX;

  const uint32_t argc = game::scr::Scr_GetNumParam(inst);
  if (argc == 0) {
    const std::optional<game::ClientNum_t> self =
        get_self_client_num(inst, entref);
    if (!self.has_value()) {
      Scr_ParamError(inst, 1, "^1[resetname] Invalid arguments\n");
      return;
    }
    client_num = static_cast<game::ClientNum_t>(self.value());
  } else {
    client_num = static_cast<game::ClientNum_t>(game::scr::Scr_GetInt(inst, 1));
    if (!game::valid_client_num(client_num)) {
      Scr_ParamError(inst, 1, "^1[resetname] Invalid arguments\n");
      return;
    }
  }

  name::clear_name_override(client_num);
  name::sync_name_reset_to_clients(client_num);
  name::trigger_client_update(client_num);
}

void func(scriptInstance_t inst) {
  scr_entref_t entref;
  Scr_GetEntityRef(&entref, inst, 0);
  method(inst, &entref);
}
} // namespace gscr_resetname

namespace gscr_resettag {
void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  game::ClientNum_t client_num = game::INVALID_CLIENT_INDEX;

  const uint32_t argc = game::scr::Scr_GetNumParam(inst);
  if (argc == 0) {
    const std::optional<game::ClientNum_t> self =
        get_self_client_num(inst, entref);
    if (!self.has_value()) {
      Scr_ParamError(inst, 1, "^1[resettag] Invalid arguments\n");
      return;
    }
    client_num = static_cast<game::ClientNum_t>(self.value());
  } else {
    client_num = static_cast<game::ClientNum_t>(game::scr::Scr_GetInt(inst, 1));
    if (!game::valid_client_num(client_num)) {
      Scr_ParamError(inst, 1, "^1[resettag] Invalid arguments\n");
      return;
    }
  }

  name::clear_clan_abbrev_override(client_num);
  name::sync_clan_abbrev_reset_to_clients(client_num);
  name::trigger_client_update(client_num);
}

void func(scriptInstance_t inst) {
  scr_entref_t entref;
  Scr_GetEntityRef(&entref, inst, 0);
  method(inst, &entref);
}
} // namespace gscr_resettag

namespace gscr_setclientdvar {
void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  game::ClientNum_t client_num = game::INVALID_CLIENT_INDEX;
  const char *dvar_cmd = nullptr;

  const uint32_t argc = game::scr::Scr_GetNumParam(inst);
  if (argc == 1) {
    const std::optional<game::ClientNum_t> self =
        get_self_client_num(inst, entref);
    dvar_cmd = game::scr::Scr_GetString(inst, 1);
    if (!self.has_value() || !dvar_cmd) {
      Scr_ParamError(inst, 1, "^1[setclientdvar] Invalid arguments\n");
      return;
    }
    client_num = static_cast<game::ClientNum_t>(self.value());
  } else {
    client_num = static_cast<game::ClientNum_t>(game::scr::Scr_GetInt(inst, 1));
    dvar_cmd = game::scr::Scr_GetString(inst, 2);
    if (!game::valid_client_num(client_num) || !dvar_cmd) {
      Scr_ParamError(inst, 1, "^1[setclientdvar] Invalid arguments\n");
      return;
    }
  }

  const std::optional<std::string> dvar_name = extract_dvar_name(dvar_cmd);
  if (dvar_name.has_value()) {
    client_dvar_changes[client_num].insert(*dvar_name);
  }

  game::sv::SV_GameSendServerCommand(client_num, game::net::SV_CMD_CAN_IGNORE_0,
                                     utils::string::va("c \"%s\"", dvar_cmd));
}

void func(scriptInstance_t inst) {
  scr_entref_t entref;
  Scr_GetEntityRef(&entref, inst, 0);
  method(inst, &entref);
}
} // namespace gscr_setclientdvar

utils::hook::detour Scr_GetFunction_hook;
BuiltinFunction Scr_GetFunction_SearchCustom(ScrVarCanonicalName_t canonId,
                                             BuiltinType *type,
                                             int32_t *min_args,
                                             int32_t *max_args) {
  if (custom_builtins::functions.map.contains(canonId)) {
    const BuiltinFunctionDef *def = &custom_builtins::functions.map[canonId];

    *type = def->type;
    *min_args = def->min_args;
    *max_args = def->max_args;

    return def->actionFunc;
  }

  return Scr_GetFunction_hook.invoke<BuiltinFunction>(canonId, type, min_args,
                                                      max_args);
}

utils::hook::detour Scr_GetMethod_hook;
BuiltinMethod Scr_GetMethod_SearchCustom(ScrVarCanonicalName_t canonId,
                                         BuiltinType *type, int32_t *min_args,
                                         int32_t *max_args) {
  if (custom_builtins::methods.map.contains(canonId)) {
    const BuiltinMethodDef *def = &custom_builtins::methods.map[canonId];

    *type = def->type;
    *min_args = def->min_args;
    *max_args = def->max_args;

    return def->actionFunc;
  }

  return Scr_GetMethod_hook.invoke<BuiltinMethod>(canonId, type, min_args,
                                                  max_args);
}

utils::hook::detour Scr_GetFunctionReverseLookup_hook;
ScrVarCanonicalName_t
Scr_GetFunctionReverseLookup_SearchCustom(builtin::BuiltinFunction func) {
  if (custom_builtins::functions.reverse.contains(func)) {
    return custom_builtins::functions.reverse[func];
  }
  return Scr_GetFunctionReverseLookup_hook.invoke<ScrVarCanonicalName_t>(func);
}

utils::hook::detour Scr_GetMethodReverseLookup_hook;
ScrVarCanonicalName_t
Scr_GetMethodReverseLookup_SearchCustom(builtin::BuiltinMethod method) {
  if (custom_builtins::methods.reverse.contains(method)) {
    return custom_builtins::methods.reverse[method];
  }
  return Scr_GetMethodReverseLookup_hook.invoke<ScrVarCanonicalName_t>(method);
}

void builtin_dispatcher(game::scr::scriptInstance_t inst) {

  const int32_t hash = Scr_GetInt(inst, 0);
  if (custom_builtins::functions.map.contains(hash)) {
    custom_builtins::functions.map[hash].actionFunc(inst);
  } else {
    push(inst, false);
  }
}

} // namespace

void add_detour(uint8_t *target_addr, uint8_t *replacement_addr) {
  function_replacements[target_addr] = replacement_addr;
  detours_enabled.store(true, std::memory_order_seq_cst);
}

struct component final : generic_component {
  void post_unpack() override {

    Scr_GetFunctionReverseLookup_hook.create(
        game::scr::builtin::Scr_GetFunctionReverseLookup.get(),
        Scr_GetFunctionReverseLookup_SearchCustom);
    Scr_GetMethodReverseLookup_hook.create(
        game::scr::builtin::Scr_GetMethodReverseLookup.get(),
        Scr_GetMethodReverseLookup_SearchCustom);

    Scr_GetFunction_hook.create(game::scr::builtin::Scr_GetFunction.get(),
                                Scr_GetFunction_SearchCustom);
    Scr_GetMethod_hook.create(game::scr::builtin::Scr_GetMethod.get(),
                              Scr_GetMethod_SearchCustom);

    // Core
    register_builtin("replacefunc", gscr_replacefunc);
    register_builtin("executecommand", gscr_executecommand);
    register_builtin("say", gscr_say);
    register_builtin("tell", gscr_tell::func);
    register_builtin("tell", gscr_tell::method);
    register_builtin("println", gscr_println);
    register_builtin("print", gscr_print);
    register_builtin("printf", gscr_printf);

    // File I/O
    register_builtin("writefile", gscr_writefile);
    register_builtin("readfile", gscr_readfile);
    register_builtin("appendfile", gscr_appendfile);
    register_builtin("fileexists", gscr_fileexists);
    register_builtin("removefile", gscr_removefile);
    register_builtin("removedirectory", gscr_removedirectory);
    register_builtin("rmdir", gscr_removedirectory);
    register_builtin("rm", gscr_rm);
    register_builtin("filesize", gscr_filesize);
    register_builtin("createdirectory", gscr_createdirectory);
    register_builtin("mkdir", gscr_createdirectory);
    register_builtin("directoryexists", gscr_directoryexists);
    register_builtin("listfiles", gscr_listfiles);
    register_builtin("ls", gscr_ls);

    // JSON
    register_builtin("jsonvalid", gscr_jsonvalid);
    register_builtin("jsonparse", gscr_jsonparse);
    register_builtin("jsonset", gscr_jsonset);
    register_builtin("jsondump", gscr_jsondump);

    // Int64
    register_builtin("int64_op", gscr_int64_op);
    register_builtin("int64_isint", gscr_int64_isint);
    register_builtin("int64_toint", gscr_int64_toint);
    register_builtin("int64_min", gscr_int64_min);
    register_builtin("int64_max", gscr_int64_max);
    register_builtin("int64_abs", gscr_int64_abs);
    register_builtin("int64_clamp", gscr_int64_clamp);
    register_builtin("int64_tostring", gscr_int64_tostring);

    // Function lookup
    register_builtin("getfunction", gscr_getfunction);

    // Console commands
    register_builtin("addcommand", gscr_addcommand);
    register_builtin("getcommand", gscr_getcommand);

    // Utility
    register_builtin("clearreplacefuncs", gscr_clearreplacefuncs);

    // Player name/tag overrides (server-only)
    register_builtin("setname", gscr_setname::func);
    register_builtin("setname", gscr_setname::method);
    register_builtin("settag", gscr_settag::func);
    register_builtin("settag", gscr_settag::method);
    register_builtin("resetname", gscr_resetname::func);
    register_builtin("resetname", gscr_resetname::method);
    register_builtin("resettag", gscr_resettag::func);
    register_builtin("resettag", gscr_resettag::method);
    register_builtin("setclientdvar", gscr_setclientdvar::func);
    register_builtin("setclientdvar", gscr_setclientdvar::method);

    register_builtin("conststring", gscr_conststring);

    builtin::BuiltinFunctionDef *bgscr_isprofilebuild_def =
        const_cast<builtin::BuiltinFunctionDef *>(
            &builtin::table::bg::util_functions->IsProfileBuild);
    bgscr_isprofilebuild_def->max_args = 255;
    bgscr_isprofilebuild_def->actionFunc = builtin_dispatcher;

    hook_opcode(0x01D2, VM_OP_SafeCreateLocalVariables_Handler_stub,
                &VM_OP_SafeCreateLocalVariables_Handler_orig);
    hook_opcode(0x000D, VM_OP_CheckClearParams_Handler_stub,
                &VM_OP_CheckClearParams_Handler_orig);

    game_event::on_g_shutdown_game([] {
      function_replacements.clear();

      reset_tracked_client_dvars();
      client_dvar_changes.clear();

      detours_enabled.store(false, std::memory_order_seq_cst);
      clear_script_commands();
      clear_hud_text_state();
      remove_settext_hooks();
    });

    game_event::on_g_init_game([] {
      function_replacements.clear();
      client_dvar_changes.clear();
      detours_enabled.store(false, std::memory_order_seq_cst);
      clear_hud_text_state();
      install_settext_hooks();
    });
  }
};
} // namespace gsc_funcs

REGISTER_COMPONENT(gsc_funcs::component)