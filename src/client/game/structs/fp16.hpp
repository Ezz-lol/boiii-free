#pragma once

#include <cstdint>

#include <game/structs/quake/vec.hpp>

namespace game {
namespace fp16 {
union half {
  uint16_t u;
  struct {
    uint16_t Mantissa : 10;
    uint16_t Exponent : 5;
    uint16_t Sign : 1;
  };

  inline std::string serialize() const noexcept {
    return std::format("{{ \"u\": 0x{:04X}, \"Sign\": 0x{:01X}, \"Exponent\": "
                       "{:01X}, \"Mantissa\": 0x{:02X} }}",
                       u, Sign, Exponent, Mantissa);
  }
};

typedef vec2<half> half2_pack;
typedef vec4<half> half4_pack;

} // namespace fp16
} // namespace game
