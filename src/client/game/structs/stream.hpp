#pragma once

#include <cstdint>
#include <structs/str.hpp>
#include <game/structs/macros.hpp>

namespace game {
namespace stream {

enum class stream_status : int32_t {
  STREAM_STATUS_INVALID = 0x0,
  STREAM_STATUS_QUEUED = 0x1,
  STREAM_STATUS_INPROGRESS = 0x2,
  STREAM_STATUS_CANCELLING = 0x3,
  STREAM_STATUS_CANCELLED = 0x4,
  STREAM_STATUS_DEVICE_REMOVED = 0x5,
  STREAM_STATUS_READFAILED = 0x6,
  STREAM_STATUS_EOF = 0x7,
  STREAM_STATUS_FINISHED = 0x8,
  STREAM_STATUS_USER1 = 0x9,
  STREAM_STATUS_COUNT = 0xA,
};

typedef int32_t stream_fileid;
typedef int32_t stream_id;
typedef stream_fileid stream_platform_handle;

// This is a guess. Engine never defines this, and I have not yet
// investigated what type(s) are reinterpret casted to `FileSysHandle`
// in the engine.
typedef FILE DB_IFileSysFile;

typedef DB_IFileSysFile *FileSysHandle;

// Verified
struct stream_fh {
  char name[256];
  FileSysHandle h;
  bool inUse;
  bool shouldOpen;
  bool shouldClose;
  bool error;
  int32_t flags;
  int64_t readOffset;
  int64_t easyOffset;
  int64_t fileSize;
  int64_t fakeGlobalOffset;
  uint8_t cipherKey[32];
  uint8_t cipherIV[8];
};
ASSERT_SIZE(stream_fh, 0x158);
} // namespace stream
} // namespace game