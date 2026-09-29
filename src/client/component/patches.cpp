#include <std_include.hpp>

#include "component/path.hpp"
#include "component/dedicated/map_recovery.hpp"
#include "component/script_error.hpp"
#include "scheduler.hpp"
#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>

#include <game/impl/game/game.hpp>
#include <string>
#include <utils/hook.hpp>

#ifndef NDEBUG
#include <game/impl/snd/snd.hpp>
#endif

namespace patches {
game::EngineDependentDvar lobby_min_players;
utils::hook::detour com_error_hook;

utils::hook::detour Sys_Error_hook;
void Sys_Error_LogCaller(const char *fmt, ...) {
  void *callerAddr = _ReturnAddress();
  va_list ap;
  va_start(ap, fmt);
  int32_t len = vsnprintf(nullptr, 0, fmt, ap);
  va_end(ap);
  va_start(ap, fmt);
  std::vector<char> infoBuf(len + 1);
  vsnprintf(infoBuf.data(), infoBuf.size(), fmt, ap);
  va_end(ap);
  const char *msg = infoBuf.data();
  if (msg == nullptr || msg[0] == '\0') {
    msg = "No message provided!";
  }
  fprintf(stderr, "[Sys_Error] Called from 0x%p with message: \"%s\"",
          game::derelocate(callerAddr), msg);
  fflush(stderr);
  game::trace("[Sys_Error] Called from {:p} with message: \"{}\"",
              game::derelocate(callerAddr), msg);
  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT,
                        "[Sys_Error] Called from 0x%p with message: \"%s\"",
                        game::derelocate(callerAddr), msg);
  Sys_Error_hook.invoke("%s", msg);
}

#define MS 1ms
#define SECOND 1000 * MS
#define MINUTE 60 * SECOND
#define HOUR 60 * MINUTE

void com_error_stub(const char *file, int32_t line, game::errorParm code,
                    const char *fmt, ...) {
  void *callerAddr = _ReturnAddress();
  static bool suppress_next_lua_error = false;
  static bool client_script_error_pending = false;

  char buffer[0x1000];
  {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf_s(buffer, _TRUNCATE, fmt, ap);
    va_end(ap);
  }

  if (game::is_server() && script_error::is_script_vm_failure(file, code)) {
    const script_error::report report =
        script_error::build_report(code, buffer, file);
    script_error::print_report(report, map_recovery::on_map_stopped());
    com_error_hook.invoke<void>(file, line, game::errorParm::DROP, "%s",
                                report.summary.c_str());
    return;
  }

  if (!script_error::is_reported(buffer)) {
    const char *log = utils::string::va(
        "[Com_Error] Called from 0x%p with message: \"%s\", code: %d\n",
        game::derelocate(callerAddr),
        buffer[0] ? buffer : "No message provided!",
        static_cast<int32_t>(code));
    fprintf(stderr, "%s\n", log);
    fflush(stderr);
    game::trace("{}", log);
    game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                          game::consoleLabel_e::DEFAULT, "%s\n", log);
  }

  // Suppress cascading Lua error (code 512) after a script error
  if (suppress_next_lua_error && code == game::errorParm::LUA) {
    suppress_next_lua_error = false;
    return;
  }

  const bool is_script_error = strstr(buffer, "script error") != nullptr;
  const bool is_link_error = strstr(buffer, "linking") != nullptr ||
                             strstr(buffer, "Linking") != nullptr;
  const bool is_script_not_found =
      strstr(buffer, "Script file not found") != nullptr;

  if (!game::is_server() &&
      (is_script_error || is_link_error || is_script_not_found)) {
    if (client_script_error_pending) {
      return;
    }

    suppress_next_lua_error = true;
    const script_error::report report =
        script_error::build_report(code, buffer, file);
    script_error::print_report(report, {});

    // No script errors popups for ingame menu , just logs in console since
    // most are harmless csc erros anyway
    if (!game::com::Com_IsInGame()) {
      return;
    }

    client_script_error_pending = true;
    const std::string deferred_error = report.text;
    scheduler::once(
        [deferred_error]() {
          client_script_error_pending = false;
          if (game::com::Com_IsInGame())
            game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "disconnect\n");
          scheduler::once(
              [deferred_error]() {
                game::ui::UI_OpenErrorPopupWithMessage(
                    game::LOCAL_CLIENT_0, game::errorCode::NONE,
                    deferred_error.c_str());
              },
              scheduler::pipeline::main, 500ms);
        },
        scheduler::pipeline::main);
    return;
  }

  if (!is_script_error && !is_link_error && !is_script_not_found &&
      !script_error::is_reported(buffer)) {
    printf("[Com_Error] Code=%d, File=%s, Line=%d, Caller=0x%llX: %s\n",
           static_cast<int32_t>(code), file ? file : "unknown", line,
           reinterpret_cast<unsigned long long>(game::derelocate(callerAddr)),
           buffer);
  }

  if (!game::is_server() && code == game::errorParm::FATAL) {
    std::string deferred_error = std::string(buffer);
    scheduler::once(
        [deferred_error]() {
          game::ui::UI_OpenErrorPopupWithMessage(game::LOCAL_CLIENT_0,
                                                 game::errorCode::NONE,
                                                 deferred_error.c_str());
        },
        scheduler::pipeline::main, 500ms);
  }

  if (strstr(buffer, "Couldn't find the bsp for this map") ||
      strstr(buffer, "Couldn't find the bsp")) {
    const char *message =
        "Missing map BSP detected.\n"
        "You are probably in the main menu or not currently playing a map.";

    printf("[Com_Error] %s Connection error: %s\n", message, buffer);

    std::string msg = std::string(message);

    scheduler::once(
        [msg]() {
          game::ui::UI_OpenErrorPopupWithMessage(
              game::LOCAL_CLIENT_0, game::errorCode::NONE, msg.c_str());
        },
        scheduler::pipeline::main, 500ms);

    return;
  }

  // Removing this skips internal engine error handling,
  // which is preferable to execute if the error is not fatal.
  com_error_hook.invoke<void>(file, line, code, "%s", buffer);
}

void scr_get_num_expected_players() {
  int32_t expected_players = game::lobby::LobbyHost_GetClientCount(
      game::lobby::LobbyType::GAME, game::lobby::LobbyClientType::ALL);

  const game::eModes mode = game::com::Com_SessionMode_GetMode();
  if ((mode == game::eModes::ZOMBIES || mode == game::eModes::CAMPAIGN)) {
    const int32_t min_players = lobby_min_players.get_int();
    if (min_players > 0) {
      expected_players = min_players;
    }
  }

  const int32_t num_expected_players = std::max(1, expected_players);
  game::scr::Scr_AddInt(game::scr::SCRIPTINSTANCE_SERVER, num_expected_players);
}

void sv_execute_client_messages_stub(game::sv::client_s *client,
                                     game::net::msg::msg_t *msg) {
  if ((client->reliableSequence - client->reliableAcknowledge) < 0) {
    client->reliableAcknowledge = client->reliableSequence;
    game::sv::SV_DropClient(client, "EXE_LOSTRELIABLECOMMANDS", true, true);
    return;
  }

  game::sv::SV_ExecuteClientMessage(client, msg);
}

utils::hook::detour Sys_WaitForSingleObject_Safe_hook;
void Sys_WaitForSingleObject_Safe(HANDLE *event) {
  if (event != nullptr) {
    Sys_WaitForSingleObject_Safe_hook.invoke(event);
  }
}

#ifndef NDEBUG
utils::hook::detour PhysPrint_hook;
void PhysPrint_AllOutputs(const char *fmt, ...) {
  void *callerAddr = _ReturnAddress();
  va_list ap;
  va_start(ap, fmt);
  int32_t len = vsnprintf(nullptr, 0, fmt, ap);
  va_end(ap);
  va_start(ap, fmt);
  std::vector<char> infoBuf(len + 1);
  vsnprintf(infoBuf.data(), infoBuf.size(), fmt, ap);
  va_end(ap);
  const char *msg = infoBuf.data();
  if (msg == nullptr) {
    msg = "";
  }

  const char *formatted_msg =
      utils::string::va("[Phys][Print(0x%p)]%s%s", game::derelocate(callerAddr),
                        msg[0] ? " " : "", msg);
  fprintf(stdout, "%s\n", formatted_msg);
  fflush(stdout);

  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT, "%s\n", formatted_msg);
  game::trace("{}", formatted_msg);
}
#endif

utils::hook::detour G_RegisterSoundWait_hook;
#ifndef NDEBUG
utils::hook::detour SND_HashName_hook;
#endif

utils::hook::detour tlAtomicMutex_Lock_hook;

utils::hook::detour fsopen_hook;
FILE *fsopen_adjustpath(const char *FileName, const char *Mode,
                        int32_t ShFlag) {
  if (!FileName) {
    return nullptr;
  }
  std::filesystem::path path = FileName;
  path = path::normalize(path);
  const std::string path_str = path.generic_string();
  return fsopen_hook.invoke<FILE *>(path_str.c_str(), Mode, ShFlag);
}

utils::hook::detour wfsopen_hook;
FILE *wfsopen_adjustpath(const wchar_t *FileName, const wchar_t *Mode,
                         int32_t ShFlag) {
  if (!FileName) {
    return nullptr;
  }
  std::filesystem::path path = FileName;
  path = path::normalize(path);
  const std::wstring path_str = path.native();
  return wfsopen_hook.invoke<FILE *>(path_str.data(), Mode, ShFlag);
}

utils::hook::detour mkdir_hook;
int64_t mkdir_adjustpath(const wchar_t *path) {
  if (path) {
    const std::filesystem::path adjusted =
        path::normalize(std::filesystem::path(path));
    const std::wstring adjusted_str = adjusted.native();
    return mkdir_hook.invoke<int64_t>(adjusted_str.c_str());
  }

  return mkdir_hook.invoke<int64_t>(path);
}

utils::hook::detour stat64_hook;
int32_t stat64_adjustpath(const char *fileName, struct _stat64 *stat) {
  if (fileName) {
    const std::filesystem::path adjusted = path::normalize(fileName);
    const std::string adjusted_str = adjusted.generic_string();
    return stat64_hook.invoke<int32_t>(adjusted_str.c_str(), stat);
  }

  return stat64_hook.invoke<int32_t>(fileName, stat);
}

utils::hook::detour stat64i32_hook;
int32_t stat64i32_adjustpath(const char *fileName, struct _stat64i32 *stat) {
  if (fileName) {
    const std::filesystem::path adjusted = path::normalize(fileName);
    const std::string adjusted_str = adjusted.generic_string();
    return stat64i32_hook.invoke<int32_t>(adjusted_str.c_str(), stat);
  }

  return stat64i32_hook.invoke<int32_t>(fileName, stat);
}

inline void patch_os_fs_apis() {
  fsopen_hook.create(game::fs::fsopen, fsopen_adjustpath);
  wfsopen_hook.create(game::fs::wfsopen, wfsopen_adjustpath);
  mkdir_hook.create(game::fs::__mkdir, mkdir_adjustpath);
  stat64_hook.create(game::fs::stat64, stat64_adjustpath);
  stat64i32_hook.create(game::fs::stat64i32, stat64i32_adjustpath);
}

constexpr size_t PATH_BUFFER_LEN = 0x100;
utils::hook::detour FS_BuildOSPath_hook;
void FS_BuildOSPath_adjustpath(const char *base, const char *game,
                               const char *qpath, char *ospath) {
  FS_BuildOSPath_hook.invoke(base, game, qpath, ospath);

  if (ospath) {
    const std::filesystem::path adjusted = path::normalize(ospath);
    const std::string adjusted_str = adjusted.generic_string();
    strscpy(ospath, adjusted_str.c_str(), PATH_BUFFER_LEN);
  }
}

inline void patch_sys_path_builders() {
  FS_BuildOSPath_hook.create(game::fs::FS_BuildOSPath,
                             FS_BuildOSPath_adjustpath);
}

inline void patch_fs_functions() {
  patch_os_fs_apis();
  patch_sys_path_builders();
}

struct component final : generic_component {
#ifndef NDEBUG
  std::string name() override { return "patches"; }
#endif

  void post_unpack() override {
    G_RegisterSoundWait_hook.create(game::G_RegisterSoundWait.get(),
                                    game::G_RegisterSoundWait_Impl);
#ifndef NDEBUG
    SND_HashName_hook.create(game::snd::SND_HashName.get(),
                             game::snd::SND_HashName_Impl);
#endif
    // Clientfield Mismatch -> recoverable ERR_DROP
    com_error_hook.create(game::com::Com_Error_, com_error_stub);
    Sys_Error_hook.create(game::sys::Sys_Error, Sys_Error_LogCaller);

    tlAtomicMutex_Lock_hook.create(game::tlAtomicMutex::syms::Lock.get(),
                                   game::tlAtomicMutex::Lock);

    /*
       Fix memory access exception in Sys_WaitForSingleObject during mapswitch.
       Root cause is difficult to narrow down due to a callstack obfuscated by
       arxan. We circumvent this by ensuring the passed handle pointer is
       non-null before calling the function.
    */
    Sys_WaitForSingleObject_Safe_hook.create(
        game::sys::Sys_WaitForSingleObject.get(), Sys_WaitForSingleObject_Safe);

    // print hexadecimal xuids in chat game log command
    utils::hook::set<char>(game::select(0x142F5A332, 0x142FD9362, 0x140E16FA2),
                           'x');

    // change 4 character min name limit to 3 characters
    utils::hook::set<uint8_t>(
        game::select(0x1421f0f23, 0x14224DA53, 0x140531143), 3);
    utils::hook::set<uint8_t>(
        game::select(0x1421f1084, 0x14224DBB4, 0x1405312A8), 3);
    utils::hook::set<uint8_t>(
        game::select(0x1421f145c, 0x14224DF8C, 0x1405316DC), 3);

    // make sure reliableAck is not negative or too big
    utils::hook::call(game::select(0x1421F7D6C, 0x14225489C, 0x140537C4C),
                      sv_execute_client_messages_stub);

    lobby_min_players = game::register_dvar_int("lobby_min_players", 0, 0, 8,
                                                game::DVAR_NONE, "");

    utils::hook::jump(game::select(0x141A6F920, 0x141A7BCF0, 0x1402CB900),
                      scr_get_num_expected_players, true);

    patch_fs_functions();

#ifndef NDEBUG
    PhysPrint_hook.create(game::phys::PhysPrint, PhysPrint_AllOutputs);
#endif
  } // namespace patches
};
} // namespace patches

REGISTER_COMPONENT(patches::component)
