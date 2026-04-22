#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

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
WEAK symbol<game::XUID(game::ControllerIndex_t controllerIndex)>
    LiveUser_GetXuid{0x141EC7310};

} // namespace live
} // namespace game

#endif