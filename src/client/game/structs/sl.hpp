#pragma once

#include <cstdint>
#include <game/structs/macros.hpp>
namespace game {
namespace sl {
PACKED(struct RefString {
  union {
    struct {
      uint32_t magic;
      uint16_t refCount;
      uint8_t user;
      uint8_t byteLen;
    };
    struct {
      uint32_t pad;
      volatile int32_t data;
    };
  };
  char str[1];
  uint8_t _padding09[3];
});

} // namespace sl
} // namespace game