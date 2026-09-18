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
};

typedef vec2<half> half2_pack;
typedef vec4<half> half4_pack;

} // namespace fp16
} // namespace game