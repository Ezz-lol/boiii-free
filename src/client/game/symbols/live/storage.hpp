#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace storage {

WEAK symbol<int32_t(const ControllerIndex_t controllerIndex,
                    uint8_t *transferBuffer, int32_t transferBufferSize)>
    Storage_SerializeTransferData{0x142277710};

WEAK symbol<bool()> LiveStorage_DoWeHaveFFOTD{0x141EC3950};
WEAK symbol<bool()> LiveStorage_ValidateFFOTD{0x141EC4AB0};

WEAK symbol<bool()> LiveStorage_DoWeHavePlaylists{0x141EC3980};
WEAK symbol<bool()> LiveStorage_IsTimeSynced{0x141EBF9F0};
WEAK symbol<bool()> LiveStorage_AreDDLsInSync{0x141EBDEA0};

WEAK symbol<void(const ControllerIndex_t controllerIndex)>
    LiveStorage_FetchRequiredFiles{0x141EBE830};

WEAK symbol<bool> s_liveStorageInitialized{0x15163FC88};
WEAK symbol<OnlinePubDataPool> s_onlinePubData{0x14340EC40};
WEAK symbol<void(const ControllerIndex_t)> Storage_Pump{0x1422771B0};

} // namespace storage

} // namespace live
} // namespace game