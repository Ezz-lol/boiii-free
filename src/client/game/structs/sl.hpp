#pragma once

#include <cstdint>

#include <game/structs/macros.hpp>

namespace game {
namespace sl {
PACKED(struct alignas(4) RefString {
  union {
    struct {
      volatile uint32_t refCount : 16;
      volatile uint32_t user : 8;
      volatile uint32_t byteLen : 8;
    };
    volatile uint32_t data;
  };
  char str[1];
  uint8_t _padding05[3];

  uint32_t dec_ref() volatile;

  uint32_t inc_ref() volatile;
});
ASSERT_SIZE(RefString, 0x8);

} // namespace sl
} // namespace game