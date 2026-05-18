#pragma once

#include <cstdint>
namespace game {
namespace io {
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

} // namespace io
} // namespace game