#pragma once

#include <cstdint>
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

struct half2_pack {
  union {
    half h[2];
    struct {
      int16_t x;
      int16_t y;
    };
    struct {
      uint32_t xy;
    };
  };
};

struct half4_pack {
  union {
    half h[4];
    struct {
      int16_t x;
      int16_t y;
      int16_t z;
      int16_t w;
    };
    struct {
      uint32_t xy;
      uint32_t zw;
    };
    struct {
      uint64_t xyzw;
    };
  };
};

} // namespace fp16
} // namespace game