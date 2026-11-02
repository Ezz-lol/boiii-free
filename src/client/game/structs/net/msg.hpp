#pragma once

#include <game/structs/net/core.hpp>
#include <game/structs/quake/core.hpp>

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

  uint8_t read_byte();
  void read_data(void *data, int32_t len);
  int32_t read_long();
  float read_float();
  int64_t read_int64();
  int16_t read_short();
  char *read_string(char *string, uint32_t maxChars);
  char *read_stringLine(char *string, uint32_t maxChars);
  void write_byte(uint8_t c);
  void write_data(const void *data, int32_t length);
  void write_long(int32_t c);
  void write_int64(int64_t c);
  void write_short(int16_t c);
  void write_string(const std::string_view &s);
  void write_float(float f);

  inline constexpr void init(uint8_t *d, int32_t length) {
    overflowed = qfalse;
    readOnly = qfalse;
    splitData = nullptr;
    cursize = 0;
    splitSize = 0;
    readcount = 0;
    bit = 0;
    lastEntityRef = 0;
    flush = qfalse;
    targetLocalNetID = netsrc_t::CLIENT1;

    data = d;
    maxsize = length;
  }
};
ASSERT_SIZE(msg_t, 0x38);

inline constexpr auto MIN_MSG_BUF_LEN = 0x800;
typedef uint8_t min_msg_buf[MIN_MSG_BUF_LEN];

inline constexpr auto MAX_MSG_STRING_LEN = 0x400;
typedef char msgStr_t[MAX_MSG_STRING_LEN];

} // namespace msg
} // namespace net
} // namespace game
