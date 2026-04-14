#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "../game_event.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

#include "../command.hpp"

namespace script {
int64_t find_export_address(const std::string &script_name,
                            const std::string &func_name);
}

namespace gsc_funcs {
namespace {
using vm_opcode_handler_t = void(__fastcall *)(int32_t inst, int64_t *fs_0,
                                               int64_t vmc, bool *terminate);
using ScrVm_GetInt_t = int64_t(__fastcall *)(unsigned int inst,
                                             unsigned int index);
using ScrVm_GetString_t = const char *(__fastcall *)(unsigned int inst,
                                                     unsigned int index);
using ScrVm_AddString_t = void(__fastcall *)(unsigned int inst,
                                             const char *value);

ScrVm_GetInt_t ScrVm_GetInt = nullptr;
ScrVm_GetString_t ScrVm_GetString = nullptr;
ScrVm_AddString_t ScrVm_AddString = nullptr;

std::unordered_map<int32_t, void (*)(int)> custom_builtins;
std::unordered_map<int64_t, int64_t> function_replacements;
bool detours_enabled = false;

vm_opcode_handler_t orig_SafeCreateLocalVariables = nullptr;
vm_opcode_handler_t orig_CheckClearParams = nullptr;
vm_opcode_handler_t orig_ScriptMethodCallPointer = nullptr;   // 0x0077
vm_opcode_handler_t orig_ScriptFunctionCallPointer = nullptr; // 0x003A

thread_local bool return_value_set = false;

constexpr uint32_t fnv1a(const char *str) {
  uint32_t hash = 0x811c9dc5;
  while (*str) {
    hash ^= static_cast<uint8_t>(*str++);
    hash *= 0x01000193;
  }
  return hash;
}

void push_string(int inst, const char *val) {
  if (ScrVm_AddString)
    ScrVm_AddString(static_cast<unsigned int>(inst), val);
  return_value_set = true;
}

void push_int(int inst, int val) {
  game::Scr_AddInt(static_cast<game::scr::scriptInstance_t>(inst), val);
  return_value_set = true;
}

// =====================================================
// Script console commands (addcommand/getcommand)
// =====================================================

std::mutex script_cmd_mutex;
std::vector<std::string> script_cmd_names;
std::deque<std::string> script_cmd_queue;

void script_cmd_handler(const command::params &params) {
  std::string result;
  for (auto i = 0; i < params.size(); i++) {
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
    auto *entry = reinterpret_cast<int64_t *>(tables[i] + opcode * 8);
    if (!*out_orig)
      *out_orig = reinterpret_cast<vm_opcode_handler_t>(*entry);
    if (*entry == reinterpret_cast<int64_t>(*out_orig))
      *entry = reinterpret_cast<int64_t>(hook);
  }
}

void builtin_dispatcher(game::scr::scriptInstance_t inst) {
  if (!ScrVm_GetInt)
    return;

  return_value_set = false;

  try {
    const auto hash =
        static_cast<int32_t>(ScrVm_GetInt(static_cast<unsigned int>(inst), 0));
    const auto it = custom_builtins.find(hash);
    if (it != custom_builtins.end())
      it->second(static_cast<int>(inst));
  } catch (const std::exception &e) {
    printf("^1[builtin] Exception: %s\n", e.what());
  } catch (...) {
    printf("^1[builtin] Unknown exception\n");
  }

  if (!return_value_set)
    game::Scr_AddInt(inst, 0);
}

// =====================================================
// Core builtins
// =====================================================

// replacefunc: redirect all calls to target_func to replacement_func
void gscr_replacefunc(int inst) {
  const auto target_script =
      ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  const auto target_func = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
  const auto replace_script =
      ScrVm_GetString(static_cast<unsigned int>(inst), 3);
  const auto replace_func = ScrVm_GetString(static_cast<unsigned int>(inst), 4);

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
void gscr_clearreplacefuncs(int /*inst*/) {
  if (!function_replacements.empty())
    printf("[clearreplacefuncs] Clearing %zu replacement(s)\n",
           function_replacements.size());
  function_replacements.clear();
  detours_enabled = false;
}

void gscr_println(int inst) {
  const auto msg = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  printf("%s\n", msg ? msg : "");
}

void gscr_executecommand(int inst) {
  const auto cmd = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  if (cmd)
    game::Cbuf_AddText(0, utils::string::va("%s\n", cmd));
}

// addcommand("name") - registers a console command that GSC can read via
// getcommand() and you are free to to whatever you with it once you detect the
// command
void gscr_addcommand(int inst) {
  const auto name = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
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
void gscr_getcommand(int inst) {
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
void gscr_say(int inst) {
  const auto msg = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  if (msg)
    game::SV_GameSendServerCommand(
        -1, game::net::SV_CMD_CAN_IGNORE_0,
        utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
}

// tell: send a private chat message to a specific client
// GSC: player tell("Hello");
void gscr_tell(int inst) {
  const auto client_num =
      static_cast<int>(ScrVm_GetInt(static_cast<unsigned int>(inst), 1));
  const auto msg = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
  if (client_num >= 0 && client_num < 18 && msg)
    game::SV_GameSendServerCommand(
        client_num, game::net::SV_CMD_CAN_IGNORE_0,
        utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
}

// =====================================================
// File I/O builtins, paths relative to boiii/scriptdata/
// =====================================================

void gscr_writefile(int inst) {
  const auto path = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  const auto data = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
  if (!path || !data || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  const auto full = resolve_path(path);
  const auto parent = full.parent_path();
  if (!parent.empty())
    utils::io::create_directory(parent);
  push_int(inst, utils::io::write_file(full.string(), data) ? 1 : 0);
}

void gscr_readfile(int inst) {
  const auto path = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  if (!path || !is_safe_path(path)) {
    push_string(inst, "");
    return;
  }
  const auto full = resolve_path(path);
  std::string data;
  if (utils::io::read_file(full.string(), &data))
    push_string(inst, data.c_str());
  else
    push_string(inst, "");
}

void gscr_appendfile(int inst) {
  const auto path = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  const auto data = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
  if (!path || !data || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  const auto full = resolve_path(path);
  const auto parent = full.parent_path();
  if (!parent.empty())
    utils::io::create_directory(parent);
  push_int(inst, utils::io::write_file(full.string(), data, true) ? 1 : 0);
}

void gscr_fileexists(int inst) {
  const auto path = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  push_int(inst, utils::io::file_exists(resolve_path(path).string()) ? 1 : 0);
}

void gscr_removefile(int inst) {
  const auto path = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  push_int(inst, utils::io::remove_file(resolve_path(path)) ? 1 : 0);
}

void gscr_filesize(int inst) {
  const auto path = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  push_int(inst,
           static_cast<int>(utils::io::file_size(resolve_path(path).string())));
}

void gscr_createdirectory(int inst) {
  const auto path = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  push_int(inst, utils::io::create_directory(resolve_path(path)) ? 1 : 0);
}

void gscr_directoryexists(int inst) {
  const auto path = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  if (!path || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  push_int(inst, utils::io::directory_exists(resolve_path(path)) ? 1 : 0);
}

void gscr_listfiles(int inst) {
  const auto path = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  if (!path || !is_safe_path(path)) {
    push_string(inst, "");
    return;
  }
  const auto full = resolve_path(path);
  if (!utils::io::directory_exists(full)) {
    push_string(inst, "");
    return;
  }
  const auto files = utils::io::list_files(full);
  std::string result;
  for (const auto &f : files) {
    if (!result.empty())
      result += ",";
    result += f.filename().string();
  }
  push_string(inst, result.c_str());
}

// =====================================================
// JSON builtins, simple string-based operations
// =====================================================

void gscr_jsonvalid(int inst) {
  const auto json_str = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  if (!json_str) {
    push_int(inst, 0);
    return;
  }
  rapidjson::Document doc;
  doc.Parse(json_str);
  push_int(inst, doc.HasParseError() ? 0 : 1);
}

// jsonparse(json_string, key), returns value as string
void gscr_jsonparse(int inst) {
  const auto json_str = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  const auto key = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
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
void gscr_jsonset(int inst) {
  const auto json_str = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  const auto key = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
  const auto val_str = ScrVm_GetString(static_cast<unsigned int>(inst), 3);
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
void gscr_jsondump(int inst) {
  const auto path = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  const auto json_str = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
  if (!path || !json_str || !is_safe_path(path)) {
    push_int(inst, 0);
    return;
  }
  const auto full = resolve_path(path);
  const auto parent = full.parent_path();
  if (!parent.empty())
    utils::io::create_directory(parent);
  push_int(inst, utils::io::write_file(full.string(), json_str) ? 1 : 0);
}

// =====================================================
// Int64 builtins, string-based 64-bit arithmetic
// =====================================================

int64_t parse_int64_arg(unsigned int inst, unsigned int index) {
  const auto str = ScrVm_GetString(inst, index);
  if (str && str[0])
    return std::strtoll(str, nullptr, 0);
  return ScrVm_GetInt(inst, index);
}

void gscr_int64_op(int inst) {
  const auto a = parse_int64_arg(static_cast<unsigned int>(inst), 1);
  const auto op = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
  const auto b = parse_int64_arg(static_cast<unsigned int>(inst), 3);

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

void gscr_int64_isint(int inst) {
  const auto val = parse_int64_arg(static_cast<unsigned int>(inst), 1);
  push_int(inst, (val >= INT32_MIN && val <= INT32_MAX) ? 1 : 0);
}

void gscr_int64_toint(int inst) {
  const auto val = parse_int64_arg(static_cast<unsigned int>(inst), 1);
  push_int(inst, static_cast<int>(val));
}

void gscr_int64_min(int inst) {
  const auto a = parse_int64_arg(static_cast<unsigned int>(inst), 1);
  const auto b = parse_int64_arg(static_cast<unsigned int>(inst), 2);
  push_string(inst, std::to_string(std::min(a, b)).c_str());
}

void gscr_int64_max(int inst) {
  const auto a = parse_int64_arg(static_cast<unsigned int>(inst), 1);
  const auto b = parse_int64_arg(static_cast<unsigned int>(inst), 2);
  push_string(inst, std::to_string(std::max(a, b)).c_str());
}

void gscr_int64_abs(int inst) {
  const auto a = parse_int64_arg(static_cast<unsigned int>(inst), 1);
  push_string(inst, std::to_string(a < 0 ? -a : a).c_str());
}

void gscr_int64_clamp(int inst) {
  const auto val = parse_int64_arg(static_cast<unsigned int>(inst), 1);
  const auto lo = parse_int64_arg(static_cast<unsigned int>(inst), 2);
  const auto hi = parse_int64_arg(static_cast<unsigned int>(inst), 3);
  push_string(inst, std::to_string(std::clamp(val, lo, hi)).c_str());
}

void gscr_int64_tostring(int inst) {
  const auto val = parse_int64_arg(static_cast<unsigned int>(inst), 1);
  push_string(inst, std::to_string(val).c_str());
}

void gscr_getfunction(int inst) {
  const auto script_name = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
  const auto func_name = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
  if (!script_name || !func_name) {
    push_int(inst, 0);
    return;
  }
  const auto addr = script::find_export_address(script_name, func_name);
  push_int(inst, addr ? 1 : 0);
}
} // namespace

void add_detour(int64_t target_addr, int64_t replacement_addr) {
  function_replacements[target_addr] = replacement_addr;
  detours_enabled = true;
}

struct component final : generic_component {
  void post_unpack() override {
    ScrVm_GetInt = reinterpret_cast<ScrVm_GetInt_t>(
        game::select(0x1412EB7F0, 0x1401711E0));
    ScrVm_GetString = reinterpret_cast<ScrVm_GetString_t>(
        game::select(0x1412EBAA0, 0x140171490));
    ScrVm_AddString = reinterpret_cast<ScrVm_AddString_t>(
        game::select(0x1412E9A30, 0x14016F320));

    // Core
    custom_builtins[static_cast<int32_t>(fnv1a("replacefunc"))] =
        gscr_replacefunc;
    custom_builtins[static_cast<int32_t>(fnv1a("executecommand"))] =
        gscr_executecommand;
    custom_builtins[static_cast<int32_t>(fnv1a("say"))] = gscr_say;
    custom_builtins[static_cast<int32_t>(fnv1a("tell"))] = gscr_tell;
    custom_builtins[static_cast<int32_t>(fnv1a("println"))] = gscr_println;
    custom_builtins[static_cast<int32_t>(fnv1a("print"))] = gscr_println;
    custom_builtins[static_cast<int32_t>(fnv1a("printf"))] = gscr_println;

    // File I/O
    custom_builtins[static_cast<int32_t>(fnv1a("writefile"))] = gscr_writefile;
    custom_builtins[static_cast<int32_t>(fnv1a("readfile"))] = gscr_readfile;
    custom_builtins[static_cast<int32_t>(fnv1a("appendfile"))] =
        gscr_appendfile;
    custom_builtins[static_cast<int32_t>(fnv1a("fileexists"))] =
        gscr_fileexists;
    custom_builtins[static_cast<int32_t>(fnv1a("removefile"))] =
        gscr_removefile;
    custom_builtins[static_cast<int32_t>(fnv1a("filesize"))] = gscr_filesize;
    custom_builtins[static_cast<int32_t>(fnv1a("createdirectory"))] =
        gscr_createdirectory;
    custom_builtins[static_cast<int32_t>(fnv1a("directoryexists"))] =
        gscr_directoryexists;
    custom_builtins[static_cast<int32_t>(fnv1a("listfiles"))] = gscr_listfiles;

    // JSON
    custom_builtins[static_cast<int32_t>(fnv1a("jsonvalid"))] = gscr_jsonvalid;
    custom_builtins[static_cast<int32_t>(fnv1a("jsonparse"))] = gscr_jsonparse;
    custom_builtins[static_cast<int32_t>(fnv1a("jsonset"))] = gscr_jsonset;
    custom_builtins[static_cast<int32_t>(fnv1a("jsondump"))] = gscr_jsondump;

    // Int64
    custom_builtins[static_cast<int32_t>(fnv1a("int64_op"))] = gscr_int64_op;
    custom_builtins[static_cast<int32_t>(fnv1a("int64_isint"))] =
        gscr_int64_isint;
    custom_builtins[static_cast<int32_t>(fnv1a("int64_toint"))] =
        gscr_int64_toint;
    custom_builtins[static_cast<int32_t>(fnv1a("int64_min"))] = gscr_int64_min;
    custom_builtins[static_cast<int32_t>(fnv1a("int64_max"))] = gscr_int64_max;
    custom_builtins[static_cast<int32_t>(fnv1a("int64_abs"))] = gscr_int64_abs;
    custom_builtins[static_cast<int32_t>(fnv1a("int64_clamp"))] =
        gscr_int64_clamp;
    custom_builtins[static_cast<int32_t>(fnv1a("int64_tostring"))] =
        gscr_int64_tostring;

    // Function lookup
    custom_builtins[static_cast<int32_t>(fnv1a("getfunction"))] =
        gscr_getfunction;

    // Console commands
    custom_builtins[static_cast<int32_t>(fnv1a("addcommand"))] =
        gscr_addcommand;
    custom_builtins[static_cast<int32_t>(fnv1a("getcommand"))] =
        gscr_getcommand;

    // Utility
    custom_builtins[static_cast<int32_t>(fnv1a("clearreplacefuncs"))] =
        gscr_clearreplacefuncs;

    auto *builtin_def = reinterpret_cast<game::BuiltinFunctionDef *>(
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