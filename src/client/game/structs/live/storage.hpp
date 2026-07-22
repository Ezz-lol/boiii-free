#pragma once

#include "../dw/core.hpp"
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
  dw::dwFileOperationInfo *fileOps;
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
} // namespace storage
} // namespace live
} // namespace game