#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {

WEAK symbol<void(const char *comErrorString, uint32_t code)>
    Live_DelayedComError{0x141E0C140};

// Live
WEAK symbol<bool(ControllerIndex_t controllerIndex, int32_t *infoBitmask,
                 bool outputDebugInfo)>
    Live_GetConnectivityInformation{0x141E0C380};

// LiveStats
WEAK symbol<const char *(int32_t controllerIndex)> LiveStats_GetClanTagText{
    0x141E9CE20};

WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 eNetworkModes networkMode)>
    SaveGame_IsDataReady{0x1415E2C90};
WEAK symbol<bool(ControllerIndex_t controllerIndex)>
    Live_LocalClient_StorageAndStats_Ready{0x141E0B520};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 eNetworkModes networkMode)>
    LiveStats_Core_Ready{0x141EA9A30};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 eNetworkModes networkMode)>
    LiveStats_Loadouts_Ready{0x141EAF490};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 const StorageFileType fileType, StorageSlot slot)>
    Storage_IsFileReady{0x142276E30};

WEAK symbol<LiveUserAuthPool> live_clientAuthPool{0x0, 0x14837F408};

WEAK symbol<bool(game::ControllerIndex_t controllerIndex)>
    Live_IsUserSignedInToDemonware{0x141E0D7A0};

// DoubleXP
WEAK symbol<uint32_t(ControllerIndex_t controllerIndex)>
    Live_DoubleXPGetGroupMaskForController{0x141E110D0};
WEAK symbol<uint32_t[2]> g_oldDoubleXPMask{0x1453D8BB0};

} // namespace live
} // namespace game
