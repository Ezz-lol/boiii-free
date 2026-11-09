#include <std_include.hpp>

#include "game/game.hpp"

namespace game {
namespace net {
namespace msg {

// '’' in Windows-1252 and ISO-8859-1 charsets
constexpr uint8_t RIGHT_SINGLE_QUOTE_WIN_CHAR = 0x92;
alwaysinline char sanitize_char(uint8_t raw_byte) {
  switch (raw_byte) {
  case RIGHT_SINGLE_QUOTE_WIN_CHAR:
    return '\'';
  case '%':
    return '.';
  default:
    return static_cast<char>(raw_byte);
  }
}

uint8_t msg_t::read_byte() {
  if (readcount >= cursize + splitSize) {
    overflowed = qtrue;
    return 0;
  }

  uint8_t result =
      (readcount < cursize) ? data[readcount] : splitData[readcount - cursize];
  readcount++;
  return result;
}

void msg_t::read_data(void *dest, size_t len) {
  if (len > 0) {
    if (readcount + len > cursize + splitSize) {
      overflowed = qtrue;
      std::memset(dest, -1, len);
    } else {
      char *dest_ptr = static_cast<char *>(dest);
      size_t remaining = len;

      while (remaining > 0) {
        if (readcount < cursize) {
          size_t chunk =
              std::min(remaining, static_cast<size_t>(cursize - readcount));
          std::memcpy(dest_ptr, data + readcount, chunk);
          readcount += static_cast<int32_t>(chunk);
          dest_ptr += chunk;
          remaining -= chunk;
        } else {
          size_t chunk = std::min<size_t>(
              remaining,
              static_cast<size_t>((cursize + splitSize) - readcount));
          std::memcpy(dest_ptr, splitData + (readcount - cursize), chunk);
          readcount += static_cast<int32_t>(chunk);
          dest_ptr += chunk;
          remaining -= chunk;
        }
      }
    }
  }
}

int32_t msg_t::read_long() {
  if (readcount + sizeof(int32_t) > cursize + splitSize) {
    overflowed = qtrue;
    return -1;
  }

  uint8_t b0 = read_byte();
  uint8_t b1 = read_byte();
  uint8_t b2 = read_byte();
  uint8_t b3 = read_byte();

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  uint32_t val = static_cast<uint32_t>(b0) | (static_cast<uint32_t>(b1) << 8) |
                 // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                 (static_cast<uint32_t>(b2) << 16) |
                 // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
                 (static_cast<uint32_t>(b3) << 24);

  return static_cast<int32_t>(letoh(val));
}

float msg_t::read_float() {
  int32_t val = read_long();
  if (val == -1) {
    overflowed = qtrue;
    return -1.0f;
  }
  return std::bit_cast<float>(val);
}

int64_t msg_t::read_int64() {
  if (readcount + sizeof(int64_t) > cursize + splitSize) {
    overflowed = qtrue;
    return 0;
  }

  uint8_t b0 = read_byte();
  uint8_t b1 = read_byte();
  uint8_t b2 = read_byte();
  uint8_t b3 = read_byte();
  uint8_t b4 = read_byte();
  uint8_t b5 = read_byte();
  uint8_t b6 = read_byte();
  uint8_t b7 = read_byte();

  uint64_t l =
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      static_cast<uint64_t>(b0) | (static_cast<uint64_t>(b1) << 8) |
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      (static_cast<uint64_t>(b2) << 16) | (static_cast<uint64_t>(b3) << 24) |
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      (static_cast<uint64_t>(b4) << 32) | (static_cast<uint64_t>(b5) << 40) |
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      (static_cast<uint64_t>(b6) << 48) | (static_cast<uint64_t>(b7) << 56);

  return static_cast<int64_t>(letoh(l));
}

int16_t msg_t::read_short() {
  if (readcount + sizeof(int16_t) > cursize + splitSize) {
    overflowed = qtrue;
    return -1;
  }

  uint8_t b0 = read_byte();
  uint8_t b1 = read_byte();

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  uint16_t val = static_cast<uint16_t>(b0) | (static_cast<uint16_t>(b1) << 8);
  return static_cast<int16_t>(letoh(val));
}

char *msg_t::read_string(char *string, size_t maxChars) {
  if (maxChars > 0) {
    uint32_t i = 0;
    while (true) {
      uint8_t b = read_byte();
      uint8_t decoded = b == std::numeric_limits<uint8_t>::max() ? '\0' : b;

      if (i < maxChars) {
        string[i] = sanitize_char(decoded);
      }

      if (decoded == '\0') {
        break;
      }
      i++;
    }
    string[maxChars - 1] = '\0';
  }
  return string;
}

char *msg_t::read_stringLine(char *string, size_t maxChars) {
  if (maxChars > 0) {
    uint32_t i = 0;
    while (true) {
      uint8_t b = read_byte();
      uint8_t decoded = b == '\n' ? '\0' : b;

      if (i < maxChars) {
        string[i] = sanitize_char(decoded);
      }

      if (decoded == '\0') {
        break;
      }
      i++;
    }
    string[maxChars - 1] = '\0';
  }
  return string;
}

void msg_t::write_byte(uint8_t c) {
  if (cursize >= maxsize) {
    overflowed = qtrue;
  } else {
    data[cursize++] = c;
  }
}

void msg_t::write_data(const void *src_data, size_t length) {
  if (length > 0) {
    if (cursize + length > maxsize) {
      overflowed = qtrue;
    } else {
      std::memcpy(data + cursize, src_data, length);
      cursize += static_cast<int32_t>(length);
    }
  }
}

void msg_t::write_long(int32_t c) {
  if (cursize + sizeof(int32_t) > maxsize) {
    overflowed = qtrue;
  } else {
    *reinterpret_cast<uint32_t *>(data + cursize) =
        htole(static_cast<uint32_t>(c));
    cursize += sizeof(int32_t);
  }
}

void msg_t::write_int64(int64_t c) {
  if (cursize + sizeof(int64_t) > maxsize) {
    overflowed = qtrue;
  } else {
    *reinterpret_cast<uint64_t *>(data + cursize) =
        htole(static_cast<uint64_t>(c));
    cursize += sizeof(int64_t);
  }
}

void msg_t::write_short(int16_t c) {
  if (cursize + sizeof(int16_t) > maxsize) {
    overflowed = qtrue;
  } else {
    *reinterpret_cast<uint16_t *>(data + cursize) =
        htole(static_cast<uint16_t>(c));
    cursize += sizeof(int16_t);
  }
}

void msg_t::write_string(const std::string_view &s) {
  if (s.length() < MAX_MSG_STRING_LEN) {
    msgStr_t src;
    size_t len = s.length();

    for (size_t i = 0; i < len; ++i) {
      src[i] = sanitize_char(s[i]);
    }
    src[len] = '\0';
    int32_t total_len = static_cast<int32_t>(len + 1);

    if (cursize + total_len > maxsize) {
      overflowed = qtrue;
    } else {
      std::memcpy(data + cursize, src, total_len);
      cursize += total_len;
    }
  } else {
    if (cursize + sizeof(char) > maxsize) {
      overflowed = qtrue;
    } else {
      data[cursize++] = '\0';
    }
  }
}

void msg_t::write_float(float f) {
  if (cursize + sizeof(float) > maxsize) {
    overflowed = qtrue;
  } else {
    write_long(std::bit_cast<int32_t>(f));
  }
}

} // namespace msg
} // namespace net
} // namespace game
