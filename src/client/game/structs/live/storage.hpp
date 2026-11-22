#pragma once

#include <game/structs/dw/core.hpp>
#include <game/structs/dw/task/fs.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace storage {

enum class OnlinePublisherFilenameTable : uint32_t {
  PLAYLISTS = 0x0,
  MOTD = 0x1,
  FEATURED_CARDS = 0x2,
  EXPERIMENTS = 0x3,
  TABLE_COUNT = 0x4,
};

struct onlinePubData {
  const char *fileNameType;
  char fileName[128];
  uint8_t *buffer;
  int bufferSize;
  bool isFileCompressed;
  const dw::TaskDefinition *taskDef;
  dw::task::fs::dwFileOperationInfo *fileOps;
  bool dataFetchCompleted;
  bool multipleLanguages;
  fastcallPtr_t<void(const ControllerIndex_t controllerIndex, void *)>
      successCallback;
  fastcallPtr_t<dw::taskCompleteResults(const ControllerIndex_t controllerIndex,
                                        void *)>
      failureCallback;
};

union OnlinePubDataPool {
  struct {
    onlinePubData playlists;
    onlinePubData motd;
    onlinePubData featured_cards;
    onlinePubData experiments;
  };
  onlinePubData data[4];
};
ASSERT_SIZE(OnlinePubDataPool, 0x300);

struct StorageFileDesc {
  StorageFileType fileType;
  const char *name;
  uint8_t unk10[0x10];
  int32_t isOnline;
  uint8_t unk24[0x54];
};
ASSERT_SIZE(StorageFileDesc, 0x78);

// Fileshare transfer parameters, only the fields used by the theater films.
struct FileShareStreamUpload {
  uint8_t unk0[0x60];
  // Pulls the next recorded film bytes, returns 0 when none are pending.
  uint32_t (*streamData)(char *buffer, uint32_t size);
  void (*onSuccess)(ControllerIndex_t controllerIndex, uint64_t fileId);
};
ASSERT_OFFSET(FileShareStreamUpload, streamData, 0x60);

struct FileShareStreamDownload {
  uint64_t fileId;
  uint32_t fileSize;
  uint8_t unkC[0x1C];
  // Requested byte range, rangeEnd 0 means up to the end of the file.
  uint32_t rangeStart;
  uint32_t rangeEnd;
  uint8_t unk30[0x8];
  void (*onData)(const char *buffer, uint32_t size, uint32_t received,
                 uint32_t fileSize);
  void (*onComplete)(ControllerIndex_t controllerIndex);
  void (*onFailure)(ControllerIndex_t controllerIndex);
};
ASSERT_OFFSET(FileShareStreamDownload, rangeStart, 0x28);
ASSERT_OFFSET(FileShareStreamDownload, onData, 0x38);
} // namespace storage
} // namespace live
} // namespace game
