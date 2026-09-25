#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace storage {

WEAK symbol<ddl::DDLContext *(const ControllerIndex_t controllerIndex,
                              const StorageFileType fileType, StorageSlot slot)>
    Storage_GetDDLContext{0x142219F80, 0x142276AB0, 0x0};
WEAK symbol<const ddl::DDLState *(const StorageFileType fileType)>
    Storage_GetDDLRootState{0x14221A050, 0x142276B80, 0x14054B710};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 const StorageFileType fileType, StorageSlot slot)>
    Storage_Write{0x14221AE30, 0x142277960, 0x14054C120};

WEAK symbol<int32_t(const ControllerIndex_t controllerIndex,
                    uint8_t *transferBuffer, int32_t transferBufferSize)>
    Storage_SerializeTransferData{0x14221ABE0, 0x142277710, 0x14054BED0};

WEAK symbol<bool()> LiveStorage_DoWeHaveFFOTD{0x141EB6EC0, 0x141EC3950, 0x0};
WEAK symbol<bool()> LiveStorage_ValidateFFOTD{0x141EB8020, 0x141EC4AB0, 0x0};

WEAK symbol<bool()> LiveStorage_DoWeHavePlaylists{0x141EB6EF0, 0x141EC3980,
                                                  0x0};
WEAK symbol<bool()> LiveStorage_IsTimeSynced{0x141EB2F60, 0x141EBF9F0, 0x0};
WEAK symbol<bool()> LiveStorage_AreDDLsInSync{0x141EB1410, 0x141EBDEA0, 0x0};

WEAK symbol<void(const ControllerIndex_t controllerIndex)>
    LiveStorage_FetchRequiredFiles{0x141EB1DA0, 0x141EBE830, 0x0};

WEAK symbol<bool> s_liveStorageInitialized{0x1515C0D18, 0x15163FC88, 0x0};
WEAK symbol<OnlinePubDataPool> s_onlinePubData{0x14338FC50, 0x14340EC40, 0x0};
WEAK symbol<void(const ControllerIndex_t)> Storage_Pump{0x14221A680,
                                                        0x1422771B0, 0x0};

} // namespace storage

} // namespace live
} // namespace game
