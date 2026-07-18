#pragma once

#include "core.hpp"
#include "../quake/core.hpp"

namespace game {
namespace net {
namespace msg {
struct msg_t {
  qboolean overflowed;
  qboolean readOnly;
  uint8_t *data;
  uint8_t *splitData;
  int32_t maxsize;
  int32_t cursize;
  int32_t splitSize;
  int32_t readcount;
  int32_t bit;
  int32_t lastEntityRef;
  qboolean flush;
  netsrc_t targetLocalNetID;
};
ASSERT_SIZE(msg_t, 0x38);
typedef uint8_t min_msg_buf[2048];

} // namespace msg
} // namespace net
} // namespace game