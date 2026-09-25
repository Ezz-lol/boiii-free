#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {

WEAK symbol<void(const char *comErrorString, uint32_t code)>
    Live_DelayedComError{0x141DFF6B0, 0x141E0C140, 0x0};

// Live
WEAK symbol<bool(ControllerIndex_t controllerIndex, int32_t *infoBitmask,
                 bool outputDebugInfo)>
    Live_GetConnectivityInformation{0x141DFF8F0, 0x141E0C380, 0x0};

// LiveStats
WEAK symbol<const char *(int32_t controllerIndex)> LiveStats_GetClanTagText{
    0x141E90390, 0x141E9CE20, 0x0};

WEAK symbol<bool(ControllerIndex_t controllerIndex)>
    Live_LocalClient_StorageAndStats_Ready{0x141DFEA90, 0x141E0B520, 0x0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 const StorageFileType fileType, StorageSlot slot)>
    Storage_IsFileReady{0x14221A300, 0x142276E30, 0x0};

WEAK symbol<LiveUserAuthPool> live_clientAuthPool{0x0, 0x0, 0x14837F408};

WEAK symbol<bool(game::ControllerIndex_t controllerIndex)>
    Live_IsUserSignedInToDemonware{0x141E00D10, 0x141E0D7A0, 0x0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex)>
    Live_IsDemonwareFetchingDone{0x141E00C80, 0x141E0D710, 0x0};
WEAK symbol<void(const ControllerIndex_t controllerIndex)> Live_OnDWDisconnect{
    0x141E71B70, 0x141E7E600, 0x0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex)>
    Live_BlockUntilSignedInToDemonware{0x141DFEBB0, 0x141E0B640, 0x0};

// DoubleXP
WEAK symbol<uint32_t(ControllerIndex_t controllerIndex)>
    Live_DoubleXPGetGroupMaskForController{0x141E04640, 0x141E110D0, 0x0};
WEAK symbol<uint32_t[2]> g_oldDoubleXPMask{0x145359BB0, 0x1453D8BB0, 0x0};

WEAK symbol<EngineDependentDvar> live_insyncddlsrequired{0x15118D818,
                                                         0x15120C798, 0x0};

} // namespace live
} // namespace game
