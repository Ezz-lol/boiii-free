#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include "auth.hpp"
#include <mmeapi.h>
#include <str.hpp>

namespace game {
namespace live {
namespace user {

XUSER_SIGNIN_STATE LiveUser_GetSignInState_AlwaysSignedIn(
    const ControllerIndex_t controllerIndex) {
  if (!s_userDataForControllerMap->data[controllerIndex]->isActive) {
    return XUSER_SIGNIN_STATE::NotSignedIn;
  }
  return controllerIndex == com::Com_ControllerIndexes_GetPrimary()
             ? XUSER_SIGNIN_STATE::SignedInToLive
             : XUSER_SIGNIN_STATE::SignedInLocally;
}

bool LiveUser_IsSignedInToLive_NotGuest(
    const ControllerIndex_t controllerIndex) {
  return s_userDataForControllerMap->data[controllerIndex]->isActive &&
         controllerIndex == com::Com_ControllerIndexes_GetPrimary();
}

ControllerIndex_t
LiveUser_GetSignedInControllerFromXUID_IgnoreSignInState(const XUID xuid) {
  for (LocalClientNum_t localClientNum = game::LocalClientNum_t::LOCAL_CLIENT_0;
       localClientNum < LOCAL_CLIENT_COUNT; ++localClientNum) {
    const userDataRef data = s_userDataForControllerMap->data[localClientNum];
    if (data->xuid == xuid) {
      return data->controller;
    }
  }

  return ControllerIndex_t::INVALID_CONTROLLER_PORT;
}

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
  if (controllerIndex != CONTROLLER_INDEX_0) {
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

userDataRef LiveUser_GetUserDataForController_Patched(
    const ControllerIndex_t controllerIndex) {
  userDataRef data = s_userDataForControllerMap->data[controllerIndex];
  data->xuid = auth::get_guid(data->controller);
  if (!data->gamertag[0]) {
    LiveUser_UserGetName_ConsoleSuffix(data->controller, data->gamertag,
                                       ARRAYSIZE(data->gamertag));
  }
  data->isUnderAge = false;
  data->isContentRestricted = false;
  data->isChatRestricted = false;
  data->isActive |=
      (controllerIndex == com::Com_ControllerIndexes_GetPrimary());
  if (data->isActive) {
    data->connectionState = CONNECTION_STATE::CONNECTED;
    data->signInState =
        controllerIndex == com::Com_ControllerIndexes_GetPrimary()
            ? XUSER_SIGNIN_STATE::SignedInToLive
            : XUSER_SIGNIN_STATE::SignedInLocally;
  }
  return data;
}

XUID LiveUser_GetLocalXuid_UseAuthGuid(const userData_t *userdata) {
  return auth::get_guid(userdata->controller);
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
  LiveStorage_FetchRequiredFiles_hook.invoke(controllerIndex);
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
    if (!storage::LiveStorage_DoWeHavePlaylists() ||
        !storage::LiveStorage_DoWeHaveFFOTD()) {
      game::live::storage::LiveStorage_FetchRequiredFiles(controllerIndex);
    }
    game::live::storage::Storage_Pump(controllerIndex);

    return false;
  }
  return true;
}

} // namespace live
} // namespace game

namespace live {
void stub_func() { return; }

bool return_true() { return true; }
bool return_false() { return false; }
uint32_t return_zero() { return 0; }

utils::hook::detour LiveUser_IsAnyLocalUserContentRestricted_hook;
utils::hook::detour LiveUser_IsChatRestricted_hook;
utils::hook::detour LiveUser_IsConnected_hook;
utils::hook::detour LiveUser_IsDeviceContentRestricted_hook;
utils::hook::detour LiveUser_IsSignedIn_hook;
utils::hook::detour LiveUser_IsSignedInToLive_hook;
utils::hook::detour LiveUser_IsUserContentRestricted_hook;
utils::hook::detour LiveUser_IsUserSignedInToLive_hook;
utils::hook::detour LiveUser_GetSignInState_hook;
utils::hook::detour LiveUser_GetSignedInControllerFromXUID_hook;
utils::hook::detour LiveUser_GetUserDataForController_hook;
utils::hook::detour LiveUser_UserGetName_hook;
utils::hook::detour LiveUser_GetLocalXuid_hook;

utils::hook::detour LiveConnect_WasPlayerQueueSuccessful_hook;
utils::hook::detour LiveConnect_GetPlayerQueuePosition_hook;
utils::hook::detour LiveConnect_GetPlayerQueueTimeEstimate_hook;
utils::hook::detour LiveConnect_IsPlayerQueued_hook;

utils::hook::detour Live_IsUserSignedInToDemonware_hook;
utils::hook::detour Live_IsDemonwareFetchingDone_hook;
utils::hook::detour LiveConnect_DisableDemonwareConnect_hook;
utils::hook::detour Live_OnDWDisconnect_hook;
utils::hook::detour LiveAntiCheat_ConsoleDetailsReported_hook;

utils::hook::detour LiveSteam_NotVacBanned_hook;

class component final : public client_component {
public:
  void post_unpack() override {
    LiveUser_IsAnyLocalUserContentRestricted_hook.create(
        game::live::user::LiveUser_IsAnyLocalUserContentRestricted.get(),
        return_false);
    LiveUser_IsChatRestricted_hook.create(
        game::live::user::LiveUser_IsChatRestricted.get(), return_false);
    LiveUser_IsConnected_hook.create(
        game::live::user::LiveUser_IsConnected.get(), return_true);
    LiveUser_IsDeviceContentRestricted_hook.create(
        game::live::user::LiveUser_IsDeviceContentRestricted.get(),
        return_false);
    LiveUser_IsSignedIn_hook.create(game::live::user::LiveUser_IsSignedIn.get(),
                                    return_true);
    LiveUser_IsSignedInToLive_hook.create(
        game::live::user::LiveUser_IsSignedInToLive.get(),
        game::live::user::LiveUser_IsSignedInToLive_NotGuest);
    LiveUser_IsUserSignedInToLive_hook.create(
        game::live::user::LiveUser_IsUserSignedInToLive.get(),
        game::live::user::LiveUser_IsSignedInToLive_NotGuest);
    LiveUser_IsUserContentRestricted_hook.create(
        game::live::user::LiveUser_IsUserContentRestricted.get(), return_false);
    LiveUser_GetSignInState_hook.create(
        game::live::user::LiveUser_GetSignInState.get(),
        game::live::user::LiveUser_GetSignInState_AlwaysSignedIn);
    LiveUser_GetSignedInControllerFromXUID_hook.create(
        game::live::user::LiveUser_GetSignedInControllerFromXUID.get(),
        game::live::user::
            LiveUser_GetSignedInControllerFromXUID_IgnoreSignInState);
    LiveUser_GetUserDataForController_hook.create(
        game::live::user::LiveUser_GetUserDataForController.get(),
        game::live::user::LiveUser_GetUserDataForController_Patched);
    LiveUser_UserGetName_hook.create(
        game::live::user::LiveUser_UserGetName.get(),
        game::live::user::LiveUser_UserGetName_ConsoleSuffix);

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
    Live_IsUserSignedInToDemonware_hook.create(
        game::live::Live_IsUserSignedInToDemonware.get(), return_true);
    LiveUser_GetLocalXuid_hook.create(
        game::live::user::LiveUser_GetLocalXuid.get(),
        game::live::user::LiveUser_GetLocalXuid_UseAuthGuid);
    Live_IsDemonwareFetchingDone_hook.create(
        game::live::Live_IsDemonwareFetchingDone.get(),
        game::live::Live_IsDemonwareFetchingDone_FetchIncomplete);
    LiveConnect_DisableDemonwareConnect_hook.create(
        game::live::connect::LiveConnect_DisableDemonwareConnect.get(),
        stub_func);
    Live_OnDWDisconnect_hook.create(game::live::Live_OnDWDisconnect.get(),
                                    stub_func);
    LiveAntiCheat_ConsoleDetailsReported_hook.create(
        game::live::anticheat::LiveAntiCheat_ConsoleDetailsReported.get(),
        return_true);
    game::live::storage::LiveStorage_FetchRequiredFiles_hook.create(
        game::live::storage::LiveStorage_FetchRequiredFiles.get(),
        game::live::storage::LiveStorage_FetchRequiredFiles_Threadsafe);
    game::live::storage::Storage_Pump_hook.create(
        game::live::storage::Storage_Pump.get(),
        game::live::storage::Storage_Pump_Threadsafe);
    LiveSteam_NotVacBanned_hook.create(
        game::live::steam::LiveSteam_NotVacBanned.get(), return_true);
  }
};
} // namespace live
REGISTER_COMPONENT(live::component);