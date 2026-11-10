#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace storage {
WEAK symbol<StorageFileDesc[static_cast<size_t>(StorageFileType::FILE_COUNT)]>
    s_storageFileDescs{0x1433BBE10, 0x0};

WEAK symbol<ddl::DDLContext *(const ControllerIndex_t controllerIndex,
                              const StorageFileType fileType, StorageSlot slot)>
    Storage_GetDDLContext{0x142219F80, 0x0};
WEAK symbol<const ddl::DDLState *(const StorageFileType fileType)>
    Storage_GetDDLRootState{0x14221A050, 0x14054B710};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 const StorageFileType fileType, StorageSlot slot)>
    Storage_Write{0x14221AE30, 0x14054C120};

WEAK symbol<int32_t(const ControllerIndex_t controllerIndex,
                    uint8_t *transferBuffer, int32_t transferBufferSize)>
    Storage_SerializeTransferData{0x14221ABE0, 0x14054BED0};

WEAK symbol<bool()> LiveStorage_DoWeHaveFFOTD{0x141EB6EC0, 0x0};
WEAK symbol<bool()> LiveStorage_ValidateFFOTD{0x141EB8020, 0x0};

WEAK symbol<bool()> LiveStorage_DoWeHavePlaylists{0x141EB6EF0, 0x0};
WEAK symbol<bool()> LiveStorage_IsTimeSynced{0x141EB2F60, 0x0};
WEAK symbol<bool()> LiveStorage_AreDDLsInSync{0x141EB1410, 0x0};

WEAK symbol<void(const ControllerIndex_t controllerIndex)>
    LiveStorage_FetchRequiredFiles{0x141EB1DA0, 0x0};

WEAK symbol<bool> s_liveStorageInitialized{0x1515C0D18, 0x0};
WEAK symbol<OnlinePubDataPool> s_onlinePubData{0x14338FC50, 0x0};
WEAK symbol<void(const ControllerIndex_t)> Storage_Pump{0x14221A680, 0x0};
WEAK symbol<dw::TaskRecord *(const ControllerIndex_t controllerIndex,
                             dw::task::fs::dwFileOperationInfo *fileInfo)>
    LiveStorage_ReadDWFile{0x141EB3210, 0x0};
WEAK symbol<dw::TaskRecord *(const ControllerIndex_t controllerIndex,
                             dw::task::fs::dwFileOperationInfo *fileInfo)>
    LiveStorage_WriteDWUserFile{0x141EB4A50, 0x0};
WEAK symbol<int32_t()> LiveStorage_GetFFOTDVersion{0x141EB7680, 0x14047E580};

} // namespace storage

} // namespace live
} // namespace game
