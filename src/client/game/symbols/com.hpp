#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace com {

#define Com_Error(code, fmt, ...)                                              \
  Com_Error_(__FILE__, __LINE__, code, fmt, ##__VA_ARGS__)

// Com
WEAK symbol<void()> Com_Init_Try_Block_Function{0x1421123B0, 0x140504170};
WEAK symbol<void(int channel, unsigned int label, const char *fmt, ...)>
    Com_Printf{0x142148F60, 0x140505630};
WEAK symbol<void(const char *file, int line, int code, const char *fmt, ...)>
    Com_Error_{0x1420F8170, 0x140501470};
WEAK symbol<bool(eModes mode)> Com_SessionMode_IsMode{0x1420F7370, 0x140500940};
WEAK symbol<int()> Com_SessionMode_GetMode{0x1420F6D30, 0x1405002D0};
WEAK symbol<int()> Com_SessionMode_GetGameMode{0x1420F68B0, 0x1404FFE50};
WEAK symbol<void(eNetworkModes networkMode)> Com_SessionMode_SetNetworkMode{
    0x1420F75B0, 0x140500B80};
WEAK symbol<eGameModes(eGameModes gameMode)> Com_SessionMode_SetGameMode{
    0x1420F7570, 0x140500B40};
WEAK symbol<eModes(eModes mode)> Com_SessionMode_SetMode{0x1420F7570,
                                                         0x140500B40};
WEAK symbol<void(const char *gametype, bool loadDefaultSettings)>
    Com_GametypeSettings_SetGametype{0x1420F5980};
WEAK symbol<unsigned int(const char *settingName, bool getDefault)>
    Com_GametypeSettings_GetUInt{0x1420F4E00, 0x1404FE5C0};
WEAK symbol<bool()> Com_IsRunningUILevel{0x142148350, 0x140504BD0};
WEAK symbol<bool()> Com_IsInGame{0x1421482C0, 0x140504B90};
WEAK symbol<void(int localClientNum, eModes fromMode, eModes toMode,
                 uint32_t flags)>
    Com_SwitchMode{0x14214A4D0};
WEAK symbol<const char *(const char *fullpath)> Com_LoadRawTextFile{
    0x1420F61B0};
WEAK symbol<bool(game::LocalClientNum_t localClientNum)>
    Com_LocalClient_IsBeingUsed{0x1420EF990};
WEAK symbol<game::ControllerIndex_t(game::LocalClientNum_t localClientNum)>
    Com_LocalClient_GetControllerIndex{0x1420EF930};

WEAK symbol<game::net::netsrc_t(game::LocalClientNum_t localClientNum)>
    Com_LocalClient_GetNetworkID{0x1420EF950};

} // namespace com
} // namespace game

#endif