#pragma once

#include <game/structs/net/core.hpp>
#include <game/structs/quake/core.hpp>

namespace game {
namespace net {
namespace msg {

inline constexpr auto MAX_MSG_STRING_LEN = 0x400;
inline constexpr auto MIN_MSG_BUF_LEN = 0x800;

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
  void read_data(void *data, size_t len);
  template <const size_t N> alwaysinline void read_data(uint8_t (&arr)[N]) {
    read_data(reinterpret_cast<void *>(arr), N);
  }
  template <const size_t N>
  alwaysinline void read_data(std::span<uint8_t, N> data) {
    read_data(reinterpret_cast<void *>(data.data()), N);
  }

  alwaysinline void read_data(std::span<uint8_t> data) {
    read_data(reinterpret_cast<void *>(data.data()), data.size());
  }

  alwaysinline std::vector<uint8_t> read_data() {
    std::vector<uint8_t> result{};
    result.reserve(MIN_MSG_BUF_LEN);

    read_data(reinterpret_cast<void *>(result.data()), MIN_MSG_BUF_LEN);

    return result;
  }

  int32_t read_long();
  alwaysinline uint32_t read_ulong() {
    return static_cast<uint32_t>(read_long());
  }

  float read_float();

  int64_t read_int64();
  alwaysinline uint32_t read_uint64() {
    return static_cast<uint64_t>(read_int64());
  }

  int16_t read_short();
  alwaysinline uint32_t read_ushort() {
    return static_cast<uint16_t>(read_short());
  }

  char *read_string(char *string, size_t maxChars);
  template <const size_t N> alwaysinline void read_string(char (&arr)[N]) {
    read_string(arr, N);
  }
  template <const size_t N>
  alwaysinline void read_string(std::span<char, N> data) {
    read_string(data.data(), N);
  }
  alwaysinline void read_string(std::span<char> data) {
    read_string(data.data(), data.size());
  }
  alwaysinline std::string read_string() {
    std::string result{};
    result.reserve(MAX_MSG_STRING_LEN);

    read_string(result.data(), MAX_MSG_STRING_LEN);

    return result;
  }

  char *read_stringLine(char *string, size_t maxChars);
  template <const size_t N> alwaysinline void read_stringLine(char (&arr)[N]) {
    read_stringLine(arr, N);
  }
  template <const size_t N>
  alwaysinline void read_stringLine(std::span<char, N> data) {
    read_stringLine(data.data(), N);
  }

  alwaysinline void read_stringLine(std::span<char> data) {
    read_stringLine(data.data(), data.size());
  }

  alwaysinline std::string read_stringLine() {
    std::string result{};
    result.reserve(MAX_MSG_STRING_LEN);

    read_stringLine(result.data(), MAX_MSG_STRING_LEN);

    return result;
  }

  void write_byte(uint8_t c);
  alwaysinline void write_byte(const int8_t &c) {
    write_byte(static_cast<uint8_t>(c));
  }

  void write_data(const void *data, size_t length);
  template <const size_t N> alwaysinline void write_data(uint8_t (&arr)[N]) {
    write_data(reinterpret_cast<void *>(arr), N);
  }
  template <const size_t N>
  alwaysinline void write_data(std::span<uint8_t, N> data) {
    write_data(reinterpret_cast<void *>(data.data()), N);
  }

  alwaysinline void write_data(std::span<uint8_t> data) {
    write_data(reinterpret_cast<void *>(data.data()), data.size());
  }

  void write_long(int32_t c);
  alwaysinline void write_ulong(uint32_t c) {
    write_long(static_cast<int32_t>(c));
  }

  void write_int64(int64_t c);
  alwaysinline void write_uint64(uint64_t c) {
    write_int64(static_cast<int64_t>(c));
  }

  void write_short(int16_t c);
  alwaysinline void write_ushort(uint16_t c) {
    write_short(static_cast<int16_t>(c));
  }

  void write_string(const std::string_view &s);

  void write_float(float f);

  alwaysinline constexpr void init(uint8_t *d, size_t length) {
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
    maxsize = static_cast<int32_t>(length);
  }
};
ASSERT_SIZE(msg_t, 0x38);

typedef uint8_t min_msg_buf[MIN_MSG_BUF_LEN];

typedef char msgStr_t[MAX_MSG_STRING_LEN];

} // namespace msg
} // namespace net
} // namespace game
