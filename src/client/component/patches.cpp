#include <std_include.hpp>

#include "component/error_help.hpp"
#include "component/exception.hpp"
#include "component/game_event.hpp"
#include "component/network.hpp"
#include "component/party.hpp"
#include "component/path.hpp"
#include "component/script_error.hpp"

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
  if (!game::is_server()) {
    exception::show_fatal_error(msg);
  }
  Sys_Error_hook.invoke("%s", error_help::strip_colors(msg).c_str());
}

#define MS 1ms
#define SECOND 1000 * MS
#define MINUTE 60 * SECOND
#define HOUR 60 * MINUTE

void com_error_stub(const char *file, int32_t line, game::errorParm code,
                    const char *fmt, ...) {
  void *callerAddr = _ReturnAddress();

  char buffer[0x1000];
  {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf_s(buffer, _TRUNCATE, fmt, ap);
    va_end(ap);
  }
#ifndef NDEBUG
  if (code != game::errorParm::SOFTRESTART &&
      code != game::errorParm::SOFTRESTART_KEEPDW &&
      code != game::errorParm::SOFTRESTART_SILENT) {
#endif

    const char *log = utils::string::va(
        "[Com][Error] Called from 0x%p with message: \"%s\", code: %d\n",
        game::derelocate(callerAddr),
        buffer[0] ? buffer : "No message provided!",
        static_cast<int32_t>(code));
    fprintf(stderr, "%s\n", log);
    fflush(stderr);
    game::trace("{}", log);
    game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                          game::consoleLabel_e::DEFAULT, "%s\n", log);

    if (!game::is_server() && code == game::errorParm::FATAL) {
      exception::show_fatal_error(buffer);
    }

    if (game::is_server() &&
        (code == game::errorParm::FATAL || code == game::errorParm::DROP)) {
      const std::string_view reason =
          std::string_view(buffer).substr(buffer[0] == '\x15');
      game::foreach_connected_client([&](game::sv::client_s &client) {
        network::send(client.address, "serverError", std::string(reason));
      });
    }
#ifndef NDEBUG
  }
#endif

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

thread_local const uint32_t *client_stats = nullptr;
utils::hook::detour active_network_mode_hook;
utils::hook::detour to_active_network_mode_hook;
utils::hook::detour read_client_stats_hook;

std::optional<game::eNetworkModes> client_stats_network_mode() {
  if (!client_stats) {
    return std::nullopt;
  }
  using game::StorageFileType;
  bool sent_online = false;
  bool sent_offline = false;
  for (uint32_t slot = 0; slot < 32; ++slot) {
    const game::live::storage::StorageFileDesc &file =
        (*game::live::storage::s_storageFileDescs)[slot];
    if ((client_stats[0] & (1u << slot)) &&
        file.fileType >= StorageFileType::CP_STATS_ONLINE &&
        file.fileType <= StorageFileType::FR_STATS_OFFLINE) {
      (file.isOnline ? sent_online : sent_offline) = true;
    }
  }
  if (sent_online == sent_offline) {
    return std::nullopt;
  }
  return sent_online ? game::eNetworkModes::ONLINE
                     : game::eNetworkModes::OFFLINE;
}

game::eNetworkModes active_network_mode_stub() {
  return client_stats_network_mode().value_or(
      active_network_mode_hook.invoke<game::eNetworkModes>());
}

game::eNetworkModes
to_active_network_mode_stub(const game::eNetworkModes network_mode) {
  return client_stats_network_mode().value_or(
      to_active_network_mode_hook.invoke<game::eNetworkModes>(network_mode));
}

bool read_client_stats_stub(const game::ClientNum_t client, uint32_t *buffer,
                            const int32_t key, const int32_t max_size,
                            void *state, void *context) {
  client_stats = buffer;
  const bool result = read_client_stats_hook.invoke<bool>(
      client, buffer, key, max_size, state, context);
  client_stats = nullptr;
  return result;
}

struct sent_configstrings {
  int32_t reliable_sequence{};
  std::unordered_map<std::string, std::string> values;
};

std::mutex sent_configstrings_mutex;
std::unordered_map<const game::sv::client_s *, sent_configstrings>
    sent_configstrings_by_client;
utils::hook::detour add_server_command_hook;

bool is_repeated_configstring(const game::sv::client_s *client,
                              const std::string_view command) {
  const size_t space = command.find(' ', 2);
  if (!command.starts_with("2 ") || space == std::string_view::npos) {
    return false;
  }

  std::lock_guard lock(sent_configstrings_mutex);
  sent_configstrings &sent = sent_configstrings_by_client[client];
  if (client->reliableSequence < sent.reliable_sequence) {
    sent.values.clear();
  }
  sent.reliable_sequence = client->reliableSequence;

  const std::string value(command.substr(space + 1));
  const auto [entry, added] =
      sent.values.try_emplace(std::string(command.substr(2, space - 2)), value);
  if (!added && entry->second == value) {
    return true;
  }
  entry->second = value;
  return false;
}

void add_server_command_stub(game::sv::client_s *client,
                             const game::net::svscmd_type type,
                             const char *command) {
  if (client && command && is_repeated_configstring(client, command)) {
    return;
  }
  add_server_command_hook.invoke<void>(client, type, command);
}

void clear_sent_configstrings() {
  std::lock_guard lock(sent_configstrings_mutex);
  sent_configstrings_by_client.clear();
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

utils::hook::detour AddRow_hook;
void AddRow_Trace(game::lobby::debug::DebugSystem system, const char *str) {
  const void *returnAddr = _ReturnAddress();
  if (str && str[0]) {
    game::trace("[Lobby][Debug][{}][@{:p}] {}",
                game::lobby::debug::serialize(system), returnAddr, str);
  }

  return AddRow_hook.invoke(system, str);
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
  DEFINE_COMPONENT_NAME("patches");

  void post_unpack() override {
    G_RegisterSoundWait_hook.create(game::G_RegisterSoundWait.get(),
                                    game::G_RegisterSoundWait_Impl);
#ifndef NDEBUG
    SND_HashName_hook.create(game::snd::SND_HashName.get(),
                             game::snd::SND_HashName_Impl);
#endif
    // Clientfield Mismatch -> recoverable ERR_DROP
    com_error_hook.create(game::com::Com_Error_, com_error_stub);
    if (game::is_client()) {
      network::on("serverError",
                  [](const game::net::netadr_t &from,
                     const network::data_view &data,
                     const game::LocalClientNum_t localClientNum) {
                    if (from == party::get_connected_server(localClientNum)) {
                      script_error::show_error_popup(
                          std::string(data.begin(), data.end()));
                    }
                  });
    }
    if (game::is_new_client()) {
      active_network_mode_hook.create(
          game::com::Com_SessionMode_GetActiveNetworkMode,
          active_network_mode_stub);
      to_active_network_mode_hook.create(
          game::com::Com_SessionMode_ToActiveNetworkMode,
          to_active_network_mode_stub);
      read_client_stats_hook.create(
          game::live::stats::LiveStats_ReadClientStats, read_client_stats_stub);
      add_server_command_hook.create(game::sv::SV_AddServerCommand,
                                     add_server_command_stub);
      game_event::on_any(clear_sent_configstrings);
    }
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
    AddRow_hook.create(game::lobby::debug::AddRow, AddRow_Trace);
#endif
  }
};
} // namespace patches

REGISTER_COMPONENT(patches::component)
