#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace user {
WEAK symbol<game::XUID(game::ControllerIndex_t controllerIndex)>
    LiveUser_GetXuid{0x141EC7310, 0x14047F350};
WEAK symbol<const char *(ControllerIndex_t controllerIndex)>
    LiveUser_GetXuidString{0x141EC6E90};
WEAK symbol<const char *(ControllerIndex_t controllerIndex)>
    LiveUser_GetClientName{0x141EC72E0};
// Initial XUID generation. Value is later fetched by LiveUser_GetXuid
WEAK symbol<bool(int64_t controllerIndex, XUID *xuid)> LiveUser_UserGetXuid{
    0x141EC7650};
WEAK symbol<bool(ControllerIndex_t controllerIndex, char *buf,
                 const int bufsize)>
    LiveUser_UserGetName{0x141EC75D0, 0x14047F4B0};
WEAK symbol<UserDataForControllerMap> s_userDataForControllerMap{0x14340F180};

WEAK symbol<ControllerIndex_t(const XUID xuid)> LiveUser_GetControllerFromXUID{
    0x141EC6DD0};
WEAK symbol<ControllerIndex_t(const XUID xuid)>
    LiveUser_GetSignedInControllerFromXUID{0x141EC6E20};
WEAK symbol<XUSER_SIGNIN_STATE(const ControllerIndex_t controllerIndex)>
    LiveUser_GetSignInState{0x141EC6E00};
WEAK symbol<EUserTier(const ControllerIndex_t controllerIndex)>
    LiveUser_GetTier{0x141EC6E60};
WEAK symbol<userData_t *(const ControllerIndex_t controllerIndex)>
    LiveUser_GetUserDataForController{0x141EC6E80};
WEAK symbol<bool(ControllerIndex_t controllerIndex)> LiveUser_IsActive{
    0x141EC6F00};
WEAK symbol<bool()> LiveUser_IsAnyLocalUserContentRestricted{0x141EC6F20};
WEAK symbol<bool(const ControllerIndex_t controllerIndex)>
    LiveUser_IsChatRestricted{0x141EC6FA0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex)> LiveUser_IsConnected{
    0x141EC6FC0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex)>
    LiveUser_IsDeviceContentRestricted{0x141EC6FE0};
WEAK symbol<bool(ControllerIndex_t controllerIndex)> LiveUser_IsSignedIn{
    0x141EC7030};
WEAK symbol<bool(ControllerIndex_t controllerIndex)> LiveUser_IsSignedInToLive{
    0x141EC7050};
WEAK symbol<bool(const ControllerIndex_t controllerIndex)>
    LiveUser_IsUserContentRestricted{0x141EC7070};
WEAK symbol<bool(ControllerIndex_t controllerIndex)> LiveUser_IsUserGuest{
    0x141E7EC60};
// Same as LiveUser_IsUserGuest, but also sets the sponsorController -
// always set to Com_ControllerIndexes_GetPrimary() return value.
// Name is `LiveUser_IsUserGuest` in the engine - override.
WEAK symbol<bool(ControllerIndex_t controllerIndex,
                 ControllerIndex_t *sponsorController)>
    LiveUser_IsUserGuest2{0x141EC70C0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex)>
    LiveUser_IsUserSignedInToLive{0x141EC7110};
WEAK symbol<bool(const XUID xuid)> LiveUser_IsXUIDLocalPlayer{0x141EC7130};
WEAK symbol<void()> LiveUser_SetPrimaryActive{0x141EC6DC0};
WEAK symbol<void(const ControllerIndex_t controllerIndex, EUserTier tier)>
    LiveUser_SetTier{0x141EC7160};
WEAK symbol<XUID(const userData_t *userdata)> LiveUser_GetLocalXuid{
    0x14214ADD0};

WEAK symbol<bool(ControllerIndex_t controllerIndex,
                 const char **disconnectMessage)>
    Live_UserSignedInToLive{0x141E7EFC0};

} // namespace user

} // namespace live
} // namespace game