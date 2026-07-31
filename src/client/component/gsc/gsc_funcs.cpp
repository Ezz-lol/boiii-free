#include <atomic>
#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/utils.hpp>
#include "../game_event.hpp"
#include "../name.hpp"

#include <optional>
#include <string>
#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

#include "../command.hpp"
#include "game/impl/scr/var.hpp"
#include "game/impl/sv/sv.hpp"
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

namespace gsc {
std::unordered_map<uint8_t *, uint8_t *> function_replacements;
std::unordered_map<game::ClientNum_t, std::unordered_set<std::string>>
    client_dvar_changes;
std::atomic_bool detours_enabled = false;
} // namespace gsc

namespace gsc_funcs {
using namespace gsc;
namespace {

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

  std::scoped_lock lock(script_cmd_mutex);
  script_cmd_queue.push_back(std::move(result));
}

void clear_script_commands() {
  std::scoped_lock lock(script_cmd_mutex);
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
            client_num, game::net::SV_CMD_CAN_IGNORE,
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

/*
  Ensure HECmd_SetText reuses config strings where possible, rather than
  registering a new config string each time `SetText` is called,
  quickly overflowing registered config string limit.
*/

// Generating BG_Cache name entry table at compile time
namespace {
using namespace game::bg::cache;
using namespace game::ui::he;
using namespace game::scr::he;
constexpr char hudelem_cfgstr_pool_entry_name_prefix[] =
    "__hudelem_cfgstr_pool_entry_";
constexpr uint8_t hudelem_cfgstr_pool_entry_name_number_max_suffix_len =
    sizeof(uint16_t) * 2 /* characters per byte */;
constexpr uint8_t hudelem_cfgstr_pool_entry_name_len =
    ARRAYSIZE(hudelem_cfgstr_pool_entry_name_prefix) +
    hudelem_cfgstr_pool_entry_name_number_max_suffix_len;
typedef str<hudelem_cfgstr_pool_entry_name_len> HudElemCfgStrPoolEntryName;
constexpr ui::he::HudElementPool<HudElemCfgStrPoolEntryName>
build_hudelem_cfgstr_name_pool(
    ui::he::HudElementPool<HudElemCfgStrPoolEntryName> pool = {}) {
  for (uint16_t i = 0; i < pool.size(); ++i) {
    append_hex<ARRAYSIZE(hudelem_cfgstr_pool_entry_name_prefix), uint16_t>(
        hudelem_cfgstr_pool_entry_name_prefix, i, pool.pool[i]);
  }
  return pool;
}
/*
   Note: 0xE2D is the config string count limit in the engine, so <=1 config
   string per hud element should be fine, given loaded mod and map do not
   collectively register a truly degenerate number of config strings.
*/
static constexpr ui::he::HudElementPool<HudElemCfgStrPoolEntryName>
    HUDELEM_CFGSTR_POOL_ENTRY_NAMES = build_hudelem_cfgstr_name_pool({});

typedef str<ui::he::MAX_HUDELEM_MESSAGE_LEN> HudElemMessage;
struct RegisteredCfgString {
  atomic_optional<int32_t> idx;

  static constexpr BGCacheTypes CACHE_TYPE = BGCacheTypes::LOCSTRING;

  inline void clear() noexcept {
    idx.store(std::nullopt, std::memory_order_release);
  }

  inline void set(int32_t loc_cfgstr_idx) {
    idx.store(loc_cfgstr_idx, std::memory_order_release);
  }

  inline bool has_value() {
    return idx.load(std::memory_order_acquire).has_value();
  }

  // Caller needs to have checked if idx has value prior to call
  inline int32_t get_idx() {
    return idx.load(std::memory_order_acquire).value();
  }

  /*
     Gets config string index relative to complete config string pool - not
     just the subsection allocated for localized strings.

     Safety: Caller needs to have checked if `idx` has value prior to call
  */
  inline int32_t abs_idx() {
    return s_bgCacheTypeInfo->get(CACHE_TYPE).configStringStart + get_idx();
  }
};
static ui::he::HudElementPool<RegisteredCfgString> hudelem_cfgstr_pool = {};

void unregister_clear_hudelem_cfgstr(uint16_t hudElemIdx) {
  RegisteredCfgString *entry = &hudelem_cfgstr_pool[hudElemIdx];
  if (entry->has_value()) {
    ui::he::g_hudelems->get(hudElemIdx).elem.text = 0;
    // TAC-protected on client, so we use a re-implementation to circumvent.
    sv::SV_SetConfigString_Impl(entry->abs_idx(), "");

    s_bgCache->server.dataSet.localizedStrings[entry->get_idx()].reset();

    hudelem_cfgstr_pool[hudElemIdx].clear();
  }
}

namespace hecmd_settext {
static HudElemMessage message_buf = {0};
static HudElemMessage cleaned_message_buf = {0};
inline void clear_message_bufs() {
  memset(message_buf, 0, ARRAYSIZE(message_buf));
  memset(cleaned_message_buf, 0, ARRAYSIZE(cleaned_message_buf));
}

void HECmd_SetText_ReuseCfgString(scriptInstance_t inst, scr_entref_t *entref) {
  if (entref->is_hudelem()) [[likely]] {
    game_hudelem_t *elem = &g_hudelems->get(entref->u.hudElemIndex);

    elem->reset_value();
    const uint32_t argc = Scr_GetNumParam(inst);

    Scr_ConstructMessageString(0, argc - 1, "Hud Elem String", message_buf,
                               MAX_HUDELEM_MESSAGE_LEN);
    com::Com_CleanStringForNetwork(message_buf, cleaned_message_buf,
                                   MAX_HUDELEM_MESSAGE_LEN);

    elem->elem.type = he_type_field_t::TEXT;
    const uint16_t hudElemIdx = entref->u.hudElemIndex;
    RegisteredCfgString *pool_entry = &hudelem_cfgstr_pool[hudElemIdx];

    const bgCacheInstance cache_inst = static_cast<bgCacheInstance>(inst);
    const int32_t localized_cfgstring_index =
        BG_Cache_GetLocStringIndex(cache_inst, cleaned_message_buf);
    if (localized_cfgstring_index > 0) {
      elem->elem.text = localized_cfgstring_index;
    }
    // Not a localized string. Need to register and/or modify the config string
    // value.
    else if (get_sv_running()) {
      if (!pool_entry->has_value()) {
        // Register new config string
        pool_entry->set(
            s_bgCacheTypeInfo->get(BGCacheTypes::LOCSTRING)
                .registerFunc(cache_inst,
                              HUDELEM_CFGSTR_POOL_ENTRY_NAMES[hudElemIdx]));
#ifndef NDEBUG
        trace("[Scr][HECmd_SetText] Registered localized string "
              "configstring for "
              "hudelement 0x%03X with "
              "index 0x%lX",
              hudElemIdx, pool_entry->get_idx());
#endif
      }

      volatile bgCachedGenericData *data =
          &s_bgCache->server.dataSet.localizedStrings[pool_entry->get_idx()];
      data->setName(cleaned_message_buf);
      if (!data->refCount) {
        data->add_ref();
      }

#ifndef NDEBUG
      trace("[Scr][HECmd_SetText] Localized config string entry with "
            "index 0x%lX, "
            "absolute config string index 0x%lX: setting value to \"%s\"",
            pool_entry->get_idx(), pool_entry->abs_idx(), cleaned_message_buf);
#endif
      // TAC-protected on client, so we use a re-implementation to circumvent.
      sv::SV_SetConfigString_Impl(pool_entry->abs_idx(), cleaned_message_buf);
      elem->elem.text = pool_entry->get_idx();
    }
    clear_message_bufs();
  } else [[unlikely]] {
    Scr_ObjectError(inst, "not a hud element");
  }
}
} // namespace hecmd_settext

void unregister_clear_hudelem_cfgstr_pool() {
  if (sv::sv->running()) {
    for (uint16_t hudElemIdx = 0; hudElemIdx < ui::he::HUD_ELEMENT_POOL_SIZE;
         ++hudElemIdx) {
      unregister_clear_hudelem_cfgstr(hudElemIdx);
    }
  }
}

utils::hook::detour HudElem_DestroyAll_hook;

// Unregister and clear all hudelem_cfgstr_pool entries before destroying all
// pool entries
void HudElem_DestroyAll_ClearCfgStrEntry_Invoke() {
  unregister_clear_hudelem_cfgstr_pool();
  return HudElem_DestroyAll_hook.invoke();
}

utils::hook::detour BG_Cache_HandleConfigStringChange_hook;
void BG_Cache_HandleConfigStringChange_ReuseExisting(
    [[maybe_unused]] LocalClientNum_t localClientNum, int32_t index) {
  const char *name = cl::CL_GetConfigString(index);
#ifndef NDEBUG
  trace("[BGCache][%u][%d] Received config string change with index: 0x%lX, "
        "name: \"%s\"",
        +bgCacheInstance::CLIENT, +localClientNum, index,
        readable_ptr(name) ? name : "");
#endif
  const bool is_localized_string =
      index >= s_bgCacheTypeInfo->locstring.configStringStart &&
      index < s_bgCacheTypeInfo->locstring.configStringStart +
                  static_cast<int32_t>(
                      ARRAYSIZE(s_bgCache->client.dataSet.localizedStrings));

  if (is_localized_string) {
    volatile bgCachedGenericData *data =
        &s_bgCache->client.dataSet
             .localizedStrings[index -
                               s_bgCacheTypeInfo->locstring.configStringStart];
    data->setName(name);
    if (!data->refCount) {
      data->add_ref();
    }

    /*
      Registration or modification of a config string with this index causes the
     client to recompute its BG Cache checksum and validate it against the
     server's - this is not a true config string modification.

     In a release profile build (ours), an invalid checksum does not trigger an
     error or corrective behaviour otherwise - it simply logs the mismatch to
     BB, re-computes the checksum, and continues. This recomputation of the
     checksum causes a noticeable, slight drop in performance for the ~1/2 a
     second it is occurring, so it seems preferable to skip this.
    */
  } else if (index != s_bgCacheTypeInfo->debugstring.configStringStart +
                          static_cast<int32_t>(ARRAYSIZE(
                              s_bgCache->client.dataSet.debugStrings))) {
    BG_Cache_HandleConfigStringChange_hook.invoke(localClientNum, index);
  }
}

// HECmd script VM method hooks
inline void apply_hecmd_hooks() {
  BuiltinMethodDef *HECmd_SetText_def = const_cast<BuiltinMethodDef *>(
      &game::scr::builtin::table::hudElem_methods->SetText);
  HECmd_SetText_def->actionFunc = &hecmd_settext::HECmd_SetText_ReuseCfgString;
}

inline void apply_hudelem_hooks() {

  HudElem_DestroyAll_hook.create(game::ui::he::HudElem_DestroyAll,
                                 HudElem_DestroyAll_ClearCfgStrEntry_Invoke);

  // Client-side: don't unnecessarily re-register an entry on value change if
  // the given index was previously registered
  BG_Cache_HandleConfigStringChange_hook.create(
      game::bg::cache::BG_Cache_HandleConfigStringChange,
      BG_Cache_HandleConfigStringChange_ReuseExisting);

  apply_hecmd_hooks();
}
} // namespace

// =====================================================
// Core builtins
// =====================================================

// replacefunc: redirect all calls to target_func to replacement_func
void gscr_replacefunc(scriptInstance_t inst) {
  uint8_t *target_addr =
      reinterpret_cast<uint8_t *>(game::scr::Scr_GetFunc(inst, 0));
  uint8_t *replacement_addr =
      reinterpret_cast<uint8_t *>(game::scr::Scr_GetFunc(inst, 1));

  if (!target_addr || !replacement_addr)
    return;

  function_replacements[target_addr] = replacement_addr;
  detours_enabled.store(true, std::memory_order_release);
}

// clearreplacefuncs: remove all active function replacements
void gscr_clearreplacefuncs([[maybe_unused]] scriptInstance_t inst) {
  function_replacements.clear();
  detours_enabled.store(false, std::memory_order_release);
}

void gscr_println(scriptInstance_t inst) {
  uint32_t argc = Scr_GetNumParam(inst);
  std::string out = "";
  for (uint32_t idx = 0; idx < argc; ++idx) {
    const char *msg = Scr_GetString(inst, idx);
    if (msg && msg[0]) {
      out += msg;
    }
  }
  fprintf(stdout, "[Scr] %s\n", out.c_str());
  fflush(stdout);
  game::com::Com_Printf(0, game::consoleLabel_e::DEFAULT, "%s\n", out.c_str());

#ifndef NDEBUG
  trace("[Scr] %s", out.c_str());
#endif
}

void gscr_print(scriptInstance_t inst) {
  uint32_t argc = Scr_GetNumParam(inst);
  std::string out = "";
  for (uint32_t idx = 0; idx < argc; ++idx) {
    const char *msg = Scr_GetString(inst, idx);
    if (msg && msg[0]) {
      out += msg;
    }
  }
  fprintf(stdout, "[Scr] %s", out.c_str());
  fflush(stdout);
  game::com::Com_Printf(0, game::consoleLabel_e::DEFAULT, "%s", out.c_str());

#ifndef NDEBUG
  trace("[Scr] %s", out.c_str());
#endif
}

void gscr_printf(scriptInstance_t inst) {
  const char *format = Scr_GetString(inst, 0);
  if (!format)
    return;
  std::string buffer;

  int32_t arg_index = 1;
  for (size_t i = 0; format[i] != '\0'; i++) {
    if (format[i] == '%' &&
        arg_index < static_cast<int32_t>(Scr_GetNumParam(inst))) {
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
            - Scr_GetFloat only returns 32-bit float, so we would need to
         add a new function Scr_GetDouble to retrieve 64-bit double
         arguments
         - length modifiers for integers (e.g. %lld, %hhd)
             1. Scr_GetInt only returns 32-bit int, so we would need to add
         new functions Scr_GetInt64 and Scr_GetInt8 to retrieve 64-bit and
                8-bit integer arguments, respectively
              2. We would also need to modify the argument parsing logic to
                 determine which Scr_Get function to call based on the
         length modifier in the format string
          - handle %% for literal % character
        */

        // Either not a specifier or unsupported. Just treat it as a normal
        // % character and continue.
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

#ifndef NDEBUG
  trace("[Scr] %s", buffer.data());
#endif
}

void gscr_executecommand(scriptInstance_t inst) {
  const char *cmd = Scr_GetString(inst, 0);
  if (cmd) {
    game::cbuf::Cbuf_AddText(0, utils::string::va("%s\n", cmd));
  }
}

// addcommand("name") - registers a console command that GSC can read via
// getcommand() and you are free to to whatever you with it once you detect
// the command
void gscr_addcommand(scriptInstance_t inst) {
  const char *name = Scr_GetString(inst, 0);
  if (!name || !name[0])
    return;

  const std::string cmd_name(name);
  const std::string cmd_key = normalize_command_name(cmd_name);
  {
    std::scoped_lock lock(script_cmd_mutex);
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
  std::scoped_lock lock(script_cmd_mutex);
  const uint32_t argc = Scr_GetNumParam(inst);
  if (argc > 0) {
    const char *requested = Scr_GetString(inst, 0);
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
  const char *msg = Scr_GetString(inst, 0);
  if (msg)
    game::sv::SV_GameSendServerCommand(
        game::INVALID_CLIENT_INDEX, game::net::SV_CMD_CAN_IGNORE,
        utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
}

namespace gscr_tell {
void send(scriptInstance_t inst, game::ClientNum_t client_num,
          uint32_t message_index) {
  const char *msg = Scr_GetString(inst, message_index);
  if (game::valid_client_num(client_num) && msg) {
    game::sv::SV_GameSendServerCommand(
        client_num, game::net::SV_CMD_CAN_IGNORE,
        utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
  }
}

// Method form: player tell("Hello");
void method(scriptInstance_t inst, scr_entref_t *entref) {
  send(inst, static_cast<game::ClientNum_t>(entref->u.entnum), 0);
}

// Function form: tell(client_num, "Hello");
void func(scriptInstance_t inst) {
  send(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)), 1);
}
} // namespace gscr_tell

// =====================================================
// File I/O builtins, paths relative to boiii/scriptdata/
// =====================================================

void gscr_writefile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  const char *data = Scr_GetString(inst, 1);

  if (!path || !data || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }

  const std::filesystem::path full = resolve_path(path);
  const std::filesystem::path parent = full.parent_path();
  if (!parent.empty()) {
    utils::io::create_directory(parent);
  }
  bool append = Scr_GetBoolOptional(inst, 2, false);

  qboolean result =
      qboolean::from(utils::io::write_file(full.string(), data, append));
  push(inst, result);
}

void gscr_readfile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
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
  const char *path = Scr_GetString(inst, 0);
  const char *data = Scr_GetString(inst, 1);
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
  const char *path = Scr_GetString(inst, 0);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }
  push(inst, utils::io::file_exists(resolve_path(path).string()));
}

void gscr_removedirectory(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }
  push(inst, utils::io::remove_directory(resolve_path(path), true));
}

void gscr_rm(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }

  bool recurse = Scr_GetBoolOptional(inst, 1, false);

  if (utils::io::directory_exists(resolve_path(path))) {
    push(inst, utils::io::remove_directory(resolve_path(path), recurse));
  } else {
    push(inst, utils::io::remove_file(resolve_path(path)));
  }
}

void gscr_filesize(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }
  push(inst,
       static_cast<int>(utils::io::file_size(resolve_path(path).string())));
}

void gscr_createdirectory(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }

  bool result = utils::io::create_directory(resolve_path(path));
  push(inst, result);
}

void gscr_directoryexists(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (!path || !is_safe_path(path)) {
    push(inst, 0);
    return;
  }
  push(inst, utils::io::directory_exists(resolve_path(path)));
}

void gscr_listfiles(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
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
 Lists files in a directory, optionally recursively and including
 directories. Returns an array of file/directory paths.
*/
void gscr_ls(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (!path || !is_safe_path(path)) {
    push(inst);
    return;
  }
  fprintf(stderr, "ls: called with path %s\n", path);
  fflush(stderr);

  bool recurse = Scr_GetBoolOptional(inst, 1, false);
  bool include_directories = Scr_GetBoolOptional(inst, 2, false);

  const std::filesystem::path full = resolve_path(path);
  if (!utils::io::directory_exists(full)) {
    push(inst);
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
  push(inst, std::move(str_entries));
}

// =====================================================
// JSON builtins, simple string-based operations
// =====================================================

void gscr_jsonvalid(scriptInstance_t inst) {
  const char *json_str = Scr_GetString(inst, 0);
  if (!json_str) {
    push(inst, 0);
    return;
  }
  rapidjson::Document doc;
  doc.Parse(json_str);
  push(inst, doc.HasParseError());
}

// jsonparse(json_string, key), returns value as string
void gscr_jsonparse(scriptInstance_t inst) {
  const char *json_str = Scr_GetString(inst, 0);
  const char *key = Scr_GetString(inst, 1);
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
  const char *json_str = Scr_GetString(inst, 0);
  const char *key = Scr_GetString(inst, 1);
  const char *val_str = Scr_GetString(inst, 2);
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
  const char *path = Scr_GetString(inst, 0);
  const char *json_str = Scr_GetString(inst, 1);
  if (!path || !json_str || !is_safe_path(path)) {
    push(inst, false);
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

int64_t parse_int64_arg(scriptInstance_t inst, uint32_t index) {
  const char *str = Scr_GetString(inst, index);
  if (str && str[0])
    return std::strtoll(str, nullptr, 0);
  return static_cast<int64_t>(Scr_GetInt(inst, index));
}

void gscr_int64_op(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 0);
  const char *op = Scr_GetString(inst, 1);
  const int64_t b = parse_int64_arg(inst, 2);

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
  const int64_t val = parse_int64_arg(inst, 0);
  push(inst, (val >= INT32_MIN && val <= INT32_MAX));
}

void gscr_int64_toint(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 0);
  push(inst, static_cast<int>(val));
}

void gscr_int64_min(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 0);
  const int64_t b = parse_int64_arg(inst, 1);
  push_string(inst, std::to_string(std::min(a, b)).c_str());
}

void gscr_int64_max(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 0);
  const int64_t b = parse_int64_arg(inst, 1);
  push_string(inst, std::to_string(std::max(a, b)).c_str());
}

void gscr_int64_abs(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 0);
  push_string(inst, std::to_string(a < 0 ? -a : a).c_str());
}

void gscr_int64_clamp(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 0);
  const int64_t lo = parse_int64_arg(inst, 1);
  const int64_t hi = parse_int64_arg(inst, 2);
  push_string(inst, std::to_string(std::clamp(val, lo, hi)).c_str());
}

void gscr_int64_tostring(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 0);
  push_string(inst, std::to_string(val).c_str());
}

void gscr_getfunction(scriptInstance_t inst) {
  const char *script_name = Scr_GetString(inst, 0);
  const char *func_name = Scr_GetString(inst, 1);
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
    Scr_ParamError(inst, 0, "No hash argument provided to conststring.");
  } else {
    const ScrString_t hash = static_cast<ScrString_t>(Scr_GetInt(inst, 0));
    push_conststring(inst, hash);
  }
}

void gscr_isstruct(scriptInstance_t inst) {
  const uint32_t argc = Scr_GetNumParam(inst);
  if (argc == 0) {
    Scr_ParamError(inst, 0,
                   "No argument provided to isstruct. syntax: isstruct(var)");
  } else {
    push(inst, Scr_GetType(inst, 0) == var::ScrVarType::POINTER &&
                   Scr_GetPointerType(inst, 0) == var::ScrVarType::STRUCT);
  }
}

void gscr_ismenucached(scriptInstance_t inst) {
  const uint32_t argc = Scr_GetNumParam(inst);
  if (argc == 0) {
    Scr_ParamError(inst, 0,
                   "No argument provided to ismenucached. syntax: "
                   "ismenucached(\"MenuName\")");
  } else {
    const var::ScrVarType_t type = Scr_GetType(inst, 0);
    if (var::ScrVar_StringLike(type)) {
      using namespace game::bg::cache;
      push(inst,
           BG_Cache_IsCachedScriptMenuIndex(static_cast<bgCacheInstance>(inst),
                                            Scr_GetString(inst, 0)));
    } else {
      Scr_ParamError(
          inst, 0,
          "Argument of type %s provided to ismenucached is not a string or "
          "localized string.",
          Scr_TypeName(type));
    }
  }
}

// =====================================================
// Player name/tag overrides (server-only)
// =====================================================

namespace gscr_setname {
void set(scriptInstance_t inst, game::ClientNum_t client_num,
         uint32_t name_index) {
  const char *player_name = game::scr::Scr_GetString(inst, name_index);
  if (!game::valid_client_num(client_num) || !player_name) {
    Scr_ParamError(inst, name_index, "^1[setname] Invalid arguments\n");
    return;
  }
  name::set_name_override(client_num, player_name);
  name::sync_name_override_to_clients(client_num);
  name::trigger_client_update(client_num);
}

void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  // player setname("PlayerOne");
  set(inst, static_cast<game::ClientNum_t>(entref->u.entnum), 0);
}

void func(scriptInstance_t inst) {
  // setname(client_num, "PlayerOne");
  set(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)), 1);
}
} // namespace gscr_setname

namespace gscr_settag {
void set(scriptInstance_t inst, game::ClientNum_t client_num,
         uint32_t tag_index) {
  const char *tag = game::scr::Scr_GetString(inst, tag_index);
  if (!game::valid_client_num(client_num) || !tag) {
    Scr_ParamError(inst, tag_index, "^1[settag] Invalid arguments\n");
    return;
  }
  name::set_clan_abbrev_override(client_num, tag);
  name::sync_clan_abbrev_override_to_clients(client_num);
  name::trigger_client_update(client_num);
}

void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  // player settag("DEV");
  set(inst, static_cast<game::ClientNum_t>(entref->u.entnum), 0);
}

void func(scriptInstance_t inst) {
  // settag(client_num, "DEV");
  set(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)), 1);
}
} // namespace gscr_settag

namespace gscr_resetname {
void reset(scriptInstance_t inst, game::ClientNum_t client_num) {
  if (!game::valid_client_num(client_num)) {
    Scr_ParamError(inst, 0, "^1[resetname] Invalid arguments\n");
    return;
  }
  name::clear_name_override(client_num);
  name::sync_name_reset_to_clients(client_num);
  name::trigger_client_update(client_num);
}

void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  // player resetname();
  reset(inst, static_cast<game::ClientNum_t>(entref->u.entnum));
}

void func(scriptInstance_t inst) {
  // resetname(client_num);
  reset(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)));
}
} // namespace gscr_resetname

namespace gscr_resettag {
void reset(scriptInstance_t inst, game::ClientNum_t client_num) {
  if (!game::valid_client_num(client_num)) {
    Scr_ParamError(inst, 0, "^1[resettag] Invalid arguments\n");
    return;
  }
  name::clear_clan_abbrev_override(client_num);
  name::sync_clan_abbrev_reset_to_clients(client_num);
  name::trigger_client_update(client_num);
}

void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  // player resettag();
  reset(inst, static_cast<game::ClientNum_t>(entref->u.entnum));
}

void func(scriptInstance_t inst) {
  // resettag(client_num);
  reset(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)));
}
} // namespace gscr_resettag

namespace gscr_setclientdvar {
void set(scriptInstance_t inst, game::ClientNum_t client_num,
         uint32_t command_index) {
  const char *dvar_cmd = game::scr::Scr_GetString(inst, command_index);
  if (!game::valid_client_num(client_num) || !dvar_cmd) {
    Scr_ParamError(inst, command_index,
                   "^1[setclientdvar] Invalid arguments\n");
    return;
  }

  const std::optional<std::string> dvar_name =
      dvar_cmd ? extract_dvar_name(dvar_cmd) : std::nullopt;
  if (dvar_name.has_value()) {
    client_dvar_changes[client_num].insert(*dvar_name);
  }

  game::sv::SV_GameSendServerCommand(client_num, game::net::SV_CMD_CAN_IGNORE,
                                     utils::string::va("c \"%s\"", dvar_cmd));
}

void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  // player setclientdvar("cg_fov 120");
  set(inst, static_cast<game::ClientNum_t>(entref->u.entnum), 0);
}

void func(scriptInstance_t inst) {
  // setclientdvar(client_num, "cg_fov 120");
  set(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)), 1);
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
Scr_GetFunctionReverseLookup_SearchCustom(BuiltinFunction func) {
  if (custom_builtins::functions.reverse.contains(func)) {
    return custom_builtins::functions.reverse[func];
  }
  return Scr_GetFunctionReverseLookup_hook.invoke<ScrVarCanonicalName_t>(func);
}

utils::hook::detour Scr_GetMethodReverseLookup_hook;
ScrVarCanonicalName_t
Scr_GetMethodReverseLookup_SearchCustom(BuiltinMethod method) {
  if (custom_builtins::methods.reverse.contains(method)) {
    return custom_builtins::methods.reverse[method];
  }
  return Scr_GetMethodReverseLookup_hook.invoke<ScrVarCanonicalName_t>(method);
}

} // namespace

void add_detour(uint8_t *target_addr, uint8_t *replacement_addr) {
  function_replacements[target_addr] = replacement_addr;
  detours_enabled.store(true, std::memory_order_release);
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
    register_builtin("replacefunc", gscr_replacefunc, 2);
    register_builtin("executecommand", gscr_executecommand, 1);
    register_builtin("say", gscr_say, 1);
    register_builtin("tell", gscr_tell::func, 2);
    register_builtin("tell", gscr_tell::method, 1);
    register_variadic_builtin("println", gscr_println, 0);
    register_variadic_builtin("print", gscr_print, 0);
    register_variadic_builtin("printf", gscr_printf, 1);

    // File I/O
    register_builtin("writefile", gscr_writefile, 2, 3);
    register_builtin("readfile", gscr_readfile, 1);
    register_builtin("appendfile", gscr_appendfile, 2);
    register_builtin("fileexists", gscr_fileexists, 1);
    register_builtin({"removefile", "rm"}, gscr_rm, 1, 2);
    register_builtin({"rmdir", "removedirectory"}, gscr_removedirectory, 1);
    register_builtin("filesize", gscr_filesize, 1);
    register_builtin({"mkdir", "createdirectory"}, gscr_createdirectory, 1);
    register_builtin("directoryexists", gscr_directoryexists, 1);

    register_builtin(
        "listfiles",
        deprecate<gscr_listfiles, "listfiles", "ls",
                  "is being phased out in favor of `ls`. `ls` returns an "
                  "array of paths rather than a line-delimited "
                  "list of paths, returned as one string.">,
        1);
    register_builtin("ls", gscr_ls, 1, 3);

    // JSON
    register_builtin("jsonvalid", gscr_jsonvalid, 1);
    register_builtin("jsonparse", gscr_jsonparse, 2);
    register_builtin("jsonset", gscr_jsonset, 3);
    register_builtin("jsondump", gscr_jsondump, 2);

    // Int64
    register_builtin("int64_op", gscr_int64_op, 3);
    register_builtin("int64_isint", gscr_int64_isint, 1);
    register_builtin("int64_toint", gscr_int64_toint, 1);
    register_builtin("int64_min", gscr_int64_min, 2);
    register_builtin("int64_max", gscr_int64_max, 2);
    register_builtin("int64_abs", gscr_int64_abs, 1);
    register_builtin("int64_clamp", gscr_int64_clamp, 3);
    register_builtin("int64_tostring", gscr_int64_tostring, 1);

    // Function lookup
    register_builtin("getfunction", gscr_getfunction, 2);

    // Console commands
    register_builtin("addcommand", gscr_addcommand, 1, 2);
    register_builtin("getcommand", gscr_getcommand, 0, 1);

    // Utility
    register_builtin("clearreplacefuncs", gscr_clearreplacefuncs, 0);

    // Player name/tag overrides (server-only)
    register_builtin("setname", gscr_setname::func, 2);
    register_builtin("setname", gscr_setname::method, 1);
    register_builtin("settag", gscr_settag::func, 2);
    register_builtin("settag", gscr_settag::method, 1);
    register_builtin("resetname", gscr_resetname::func, 1);
    register_builtin("resetname", gscr_resetname::method, 0);
    register_builtin("resettag", gscr_resettag::func, 1);
    register_builtin("resettag", gscr_resettag::method, 0);
    register_builtin("setclientdvar", gscr_setclientdvar::func, 2);
    register_builtin("setclientdvar", gscr_setclientdvar::method, 1);

    register_builtin("conststring", gscr_conststring, 1);
    register_builtin("isstruct", gscr_isstruct, 1);
    register_builtin("ismenucached", gscr_ismenucached, 1);

    apply_hudelem_hooks();

    game_event::on_g_shutdown_game([] {
      function_replacements.clear();
      reset_tracked_client_dvars();
      client_dvar_changes.clear();
      detours_enabled.store(false, std::memory_order_release);
      clear_script_commands();
    });

    game_event::on_g_init_game([] {
      function_replacements.clear();
      client_dvar_changes.clear();
      detours_enabled.store(false, std::memory_order_release);
      unregister_clear_hudelem_cfgstr_pool();
    });
  }
};
} // namespace gsc_funcs

REGISTER_COMPONENT(gsc_funcs::component)