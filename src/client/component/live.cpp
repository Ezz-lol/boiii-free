#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>

#include "game/utils.hpp"
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include "auth.hpp"
#include <mmeapi.h>
#include <str.hpp>

namespace game {
namespace live {
namespace user {

constexpr const char *GUEST_PLAYER_NAME_SUFFIX_FMT = "(%d)";
constexpr uint32_t MAX_USERNAME_LEN = 17;
bool LiveUser_UserGetName_ConsoleSuffix(ControllerIndex_t controllerIndex,
                                        char *username, const int bufsize) {
  memset(reinterpret_cast<void *>(username), 0,
         // ORIGINAL:
         // 8 * bufsize
         // PATCH:
         24 /* passed buffer always has length 24 */);
  uint32_t usernameBufLen =
      (std::min)(static_cast<uint32_t>(bufsize), MAX_USERNAME_LEN);

  steam::LiveSteam_GetUserName(username, usernameBufLen, true);
  // steam::LiveSteam_GetUserName(username, usernameBufLen, true);
  if (LiveUser_GetUserDataForController(controllerIndex)->isGuest) {
    const char *guestSuffix = utils::string::va(
        // ORIGINAL:
        // " %d",
        // PATCH:
        GUEST_PLAYER_NAME_SUFFIX_FMT,
        // ORIGINAL:
        // controllerIndex);
        // PATCH:
        controllerIndex + 1);
    I_strcat(username, bufsize, guestSuffix);
  }
  return true;
}

XUID LiveUser_GetLocalXuid_UseAuthGuid(const userData_t *userdata) {
  return auth::get_guid(userdata->controller);
}

const char *
LiveUser_GetClientName_GetOrInit(const ControllerIndex_t controllerIndex) {
  const userDataRef data = s_userDataForControllerMap->data[controllerIndex];
  if (!data->gamertag[0]) {
    LiveUser_UserGetName_ConsoleSuffix(controllerIndex, data->gamertag,
                                       ARRAYSIZE(data->gamertag));
  }
  return data->gamertag;
}
} // namespace user

namespace storage {
std::recursive_mutex Storage_Pump_Lock;
utils::hook::detour Storage_Pump_hook;
void Storage_Pump_Threadsafe(const ControllerIndex_t controllerIndex) {
  std::lock_guard<std::recursive_mutex> lock(Storage_Pump_Lock);
  Storage_Pump_hook.invoke(controllerIndex);
}

std::recursive_mutex LiveStorage_FetchRequiredFiles_Lock;
utils::hook::detour LiveStorage_FetchRequiredFiles_hook;
void LiveStorage_FetchRequiredFiles_Threadsafe(
    const ControllerIndex_t controllerIndex) {
  std::lock_guard<std::recursive_mutex> lock(
      LiveStorage_FetchRequiredFiles_Lock);
  if (!LiveStorage_DoWeHavePlaylists() || !LiveStorage_DoWeHaveFFOTD()) {
    LiveStorage_FetchRequiredFiles_hook.invoke(controllerIndex);
  }
}
} // namespace storage

inline bool
Live_IsMinimalDemonwareFetchingDone(ControllerIndex_t controllerIndex) {
  bool ddls_synced =
      !*inventory::inventory_blocking ||
      !inventory::inventory_blocking->get_bool() ||
      (*live_insyncddlsrequired && !live_insyncddlsrequired->get_bool()) ||
      storage::LiveStorage_AreDDLsInSync();

  return storage::LiveStorage_DoWeHaveFFOTD() &&
         storage::LiveStorage_ValidateFFOTD() &&
         stats::LiveStats_Core_Ready(controllerIndex, eNetworkModes::ONLINE) &&
         stats::LiveStats_Loadouts_Ready(controllerIndex,
                                         eNetworkModes::ONLINE) &&
         storage::LiveStorage_IsTimeSynced() &&
         game::savegame::SaveGame_IsDataReady(controllerIndex,
                                              eNetworkModes::ONLINE) &&
         ddls_synced && storage::LiveStorage_DoWeHavePlaylists() &&
         publisher::variables::LivePublisherVariables_AreVariablesAvailable();
}

bool Live_IsDemonwareFetchingDone_FetchIncomplete(
    ControllerIndex_t controllerIndex) {

  if (!Live_IsMinimalDemonwareFetchingDone(controllerIndex)) {
    game::live::storage::Storage_Pump(controllerIndex);
    game::live::storage::LiveStorage_FetchRequiredFiles(controllerIndex);
    game::live::storage::Storage_Pump(controllerIndex);

    return false;
  }
  return true;
}

utils::hook::detour Live_LocalClient_StorageAndStats_Ready_hook;
bool Live_LocalClient_StorageAndStats_Ready_PumpRequired(
    ControllerIndex_t controllerIndex) {
  bool result =
      Live_LocalClient_StorageAndStats_Ready_hook.invoke<bool>(controllerIndex);
  if (!result) {
    storage::Storage_Pump(controllerIndex);
  }
  return result;
}

} // namespace live
namespace lobby {
namespace active {

void clientinfo_activateallcontent(MutableClientInfo *clientInfo) {
  clientInfo->dlcBits = ContentFlags::allContent();
  clientInfo->easterEggBits = -1;
  clientInfo->isStarterPack = false;
  for (uint32_t i = 0; i < ARRAYSIZE(clientInfo->chunkStatuses); ++i) {
    clientInfo->chunkStatuses[i] = 3;
  }
}
utils::hook::detour LobbyActiveList_SetClientInfo_hook;
void LobbyActiveList_SetClientInfo_ActivateAllContent(
    ActiveClient *activeClient, MutableClientInfo *mutableClientInfo) {

  LobbyActiveList_SetClientInfo_hook.invoke(activeClient, mutableClientInfo);
  clientinfo_activateallcontent(&activeClient->mutableClientInfo);
}
} // namespace active
} // namespace lobby
namespace ui {
namespace lua {
template <const hks::lua_Integer Value>
void Lua_SetTableInt_Always(const char *key,
                            [[maybe_unused]] hks::lua_Integer value,
                            hks::lua_State *luaVM) {
  return Lua_SetTableInt(key, Value, luaVM);
}
template <const bool Value>
void Lua_SetTableBool_Always(const char *key, [[maybe_unused]] bool value,
                             hks::lua_State *luaVM) {
  return Lua_SetTableBool(key, Value, luaVM);
}
} // namespace lua

} // namespace ui
} // namespace game

namespace live {
void stub_func() { return; }

bool return_true() { return true; }
bool return_false() { return false; }
uint32_t return_zero() { return 0; }
game::ContentFlags return_all_content() {
  return game::ContentFlags::allContent();
}

utils::hook::detour LiveUser_UserGetName_hook;
utils::hook::detour LiveUser_GetLocalXuid_hook;
utils::hook::detour LiveUser_GetClientName_hook;

utils::hook::detour LiveConnect_WasPlayerQueueSuccessful_hook;
utils::hook::detour LiveConnect_GetPlayerQueuePosition_hook;
utils::hook::detour LiveConnect_GetPlayerQueueTimeEstimate_hook;
utils::hook::detour LiveConnect_IsPlayerQueued_hook;

utils::hook::detour Live_IsDemonwareFetchingDone_hook;
utils::hook::detour Live_OnDWDisconnect_hook;

utils::hook::detour LiveAntiCheat_ConsoleDetailsReported_hook;

utils::hook::detour LiveSteam_NotVacBanned_hook;

class component final : public client_component {
public:
  void post_unpack() override {
    LiveUser_UserGetName_hook.create(
        game::live::user::LiveUser_UserGetName.get(),
        game::live::user::LiveUser_UserGetName_ConsoleSuffix);
    LiveUser_GetClientName_hook.create(
        game::live::user::LiveUser_GetClientName.get(),
        game::live::user::LiveUser_GetClientName_GetOrInit);
    LiveConnect_GetPlayerQueueTimeEstimate_hook.create(
        game::live::connect::LiveConnect_GetPlayerQueueTimeEstimate.get(),
        return_zero);
    LiveConnect_GetPlayerQueuePosition_hook.create(
        game::live::connect::LiveConnect_GetPlayerQueuePosition.get(),
        return_zero);
    LiveConnect_WasPlayerQueueSuccessful_hook.create(
        game::live::connect::LiveConnect_WasPlayerQueueSuccessful.get(),
        return_true);
    LiveConnect_IsPlayerQueued_hook.create(
        game::live::connect::LiveConnect_IsPlayerQueued.get(), return_false);
    LiveUser_GetLocalXuid_hook.create(
        game::live::user::LiveUser_GetLocalXuid.get(),
        game::live::user::LiveUser_GetLocalXuid_UseAuthGuid);
    Live_OnDWDisconnect_hook.create(game::live::Live_OnDWDisconnect.get(),
                                    stub_func);
    LiveAntiCheat_ConsoleDetailsReported_hook.create(
        game::live::anticheat::LiveAntiCheat_ConsoleDetailsReported.get(),
        return_true);
    Live_IsDemonwareFetchingDone_hook.create(
        game::live::Live_IsDemonwareFetchingDone.get(),
        game::live::Live_IsDemonwareFetchingDone_FetchIncomplete);
    game::live::storage::LiveStorage_FetchRequiredFiles_hook.create(
        game::live::storage::LiveStorage_FetchRequiredFiles.get(),
        game::live::storage::LiveStorage_FetchRequiredFiles_Threadsafe);
    game::live::storage::Storage_Pump_hook.create(
        game::live::storage::Storage_Pump.get(),
        game::live::storage::Storage_Pump_Threadsafe);
    LiveSteam_NotVacBanned_hook.create(
        game::live::steam::LiveSteam_NotVacBanned.get(), return_true);
    game::live::Live_LocalClient_StorageAndStats_Ready_hook.create(
        game::live::Live_LocalClient_StorageAndStats_Ready.get(),
        game::live::Live_LocalClient_StorageAndStats_Ready_PumpRequired);
  }
};
} // namespace live
REGISTER_COMPONENT(live::component);