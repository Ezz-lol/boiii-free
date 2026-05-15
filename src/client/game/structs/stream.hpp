#pragma once

#include <cstdint>
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

#pragma pack(push, 1)
struct stream_fh {
  char name[256];
  stream_platform_handle h;
  uint8_t unknown106[4];
  bool shouldOpen;
  uint8_t _unknown109[1];
  bool shouldClose;
  bool error;
  int flags;
  bool inUse;
  bool synchronous;
  uint8_t unknown[38];
  int64_t readOffset;
  int64_t easyOffset;
  int64_t fileSize;
  int64_t fakeGlobalOffset;
};

#pragma pack(pop)
} // namespace stream
} // namespace game