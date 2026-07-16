#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>

#include <utils/hook.hpp>

#include <mmeapi.h>

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

userDataRef LiveUser_GetUserDataForController_Patched(
    const ControllerIndex_t controllerIndex) {
  userDataRef data = s_userDataForControllerMap->data[controllerIndex];
  data->isUnderAge = false;
  data->isContentRestricted = false;
  data->isChatRestricted = false;
  if (data->isActive) {
    data->connectionState = CONNECTION_STATE::CONNECTION_STATE_CONNECTED;
    data->signInState =
        controllerIndex == com::Com_ControllerIndexes_GetPrimary()
            ? XUSER_SIGNIN_STATE::SignedInToLive
            : XUSER_SIGNIN_STATE::SignedInLocally;
  }
  return data;
}

} // namespace user
} // namespace live
} // namespace game
namespace liveuser {

void stub_func() { return; }

bool return_true() { return true; }
bool return_false() { return false; }

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
  }
};
} // namespace liveuser
REGISTER_COMPONENT(liveuser::component);