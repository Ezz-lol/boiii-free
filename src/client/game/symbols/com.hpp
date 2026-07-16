#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace com {

#define Com_Error(code, fmt, ...)                                              \
  Com_Error_(__FILE__, __LINE__, code, fmt, ##__VA_ARGS__)

// Com
WEAK symbol<void()> Com_Init_Try_Block_Function{0x1421123B0, 0x140504170};
WEAK symbol<void(int channel, consoleLabel_e label, const char *fmt, ...)>
    Com_Printf{0x142148F60, 0x140505630};
WEAK symbol<void(const char *file, int line, errorParm code, const char *fmt,
                 ...)>
    Com_Error_{0x1420F8170, 0x140501470};
WEAK symbol<bool(eModes mode)> Com_SessionMode_IsMode{0x1420F7370, 0x140500940};

WEAK symbol<game::eModes()> _Com_SessionMode_GetMode{0x1420F6D30, 0x1405002D0};
// To allow for forward declaration for use in struct methods
inline eModes Com_SessionMode_GetMode() { return _Com_SessionMode_GetMode(); }
WEAK symbol<int()> Com_SessionMode_GetGameMode{0x1420F68B0, 0x1404FFE50};
WEAK symbol<void(eNetworkModes networkMode)> Com_SessionMode_SetNetworkMode{
    0x1420F75B0, 0x140500B80};
WEAK symbol<void(eGameModes mode)> Com_SessionMode_SetGameMode{0x1420F7570,
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
WEAK symbol<void(const char *level)> Com_LoadLevelFastFiles{0x1421484E0,
                                                            0x140504C30};
WEAK symbol<void()> Com_UnloadLevelFastFiles{0x14214AAC0, 0x140506E40};
WEAK symbol<void()> Com_TryUnloadLevelFastFiles{0x14214BAC0, 0x140507CF0};

WEAK symbol<bool(game::LocalClientNum_t localClientNum)>
    Com_LocalClient_IsBeingUsed{0x1420EF990};
WEAK symbol<game::ControllerIndex_t(game::LocalClientNum_t localClientNum)>
    Com_LocalClient_GetControllerIndex{0x1420EF930};
WEAK symbol<game::net::netsrc_t(game::LocalClientNum_t localClientNum)>
    Com_LocalClient_GetNetworkID{0x1420EF950};
WEAK symbol<void *(live::LiveAllocTypes type, void *ptr, const size_t size)>
    Com_LiveRealloc{0x14214BE50};
WEAK symbol<void(live::LiveAllocTypes type, void *ptr)> Com_LiveDeallocate{
    0x14214BD80};
WEAK symbol<qboolean> com_errorEntered{0x1568ED8B8};
WEAK symbol<const char *()> Com_SessionMode_GetAbbreviationForCurrentMode{
    0x1420F6790};
WEAK symbol<LocalClientNum_t(ControllerIndex_t controllerIndex)>
    Com_ControllerIndex_GetLocalClientNum{0x1420EF7C0};
WEAK symbol<void()> Com_Frame_Try_Block_Function{0x1420F8E00, 0x1405020E0};
WEAK symbol<ControllerIndex_t()> Com_ControllerIndexes_GetPrimary{0x1420EF880};

} // namespace com
} // namespace game

#endif