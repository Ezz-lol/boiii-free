#pragma once

#include <game/structs/scr/core.hpp>
#include <game/structs/scr/var.hpp>
#include <game/structs/macros.hpp>

namespace game {
namespace scr {
namespace vm {

// Verified
PACKED(struct function_stack_t {
  uint8_t *pos;
  volatile var::ScrVarValue_t *top;
  var::ScrVarIndex_t threadId;
  uint32_t localVarCount;
  volatile var::ScrVarValue_t *startTop;
});
ASSERT_OFFSET(function_stack_t, localVarCount, 0x14);
ASSERT_SIZE(function_stack_t, 0x20);

// Verified
PACKED(struct function_frame_t {
  function_stack_t fs;
  bool nested;
  uint8_t _padding21[7];
});
ASSERT_SIZE(function_frame_t, 0x28);

PACKED(struct ScrVmContext_t {
  var::ScrVarIndex_t fieldValueId;
  bool fieldValueRemoveOk;
  uint8_t _padding05[3];
  var::ScrVarIndex_t objectId;
  uint8_t _padding0C[4];
  uint8_t *lastGoodPos;
  volatile var::ScrVarValue_t *lastGoodTop;
});
ASSERT_SIZE(ScrVmContext_t, 0x20);

// Verified
PACKED(struct scrVmPub_t {
  uint32_t *localVars;
  var::ScrVarValue_t *maxstack;
  int32_t function_count;
  uint8_t _padding10[4]; // Probably just padding
  function_frame_t *function_frame;
  var::ScrVarValue_t *top;
  bool debugCode;
  bool abort_on_error;
  bool terminal_error;
  bool block_execution;
  uint8_t _padding2C[4];
  uint32_t callNesting;
  uint32_t inparamcount;
  uint32_t outparamcount;
  uint8_t _padding3C[4];
  function_frame_t function_frame_start[64];
  var::ScrVarValue_t stack[2048];
});

ASSERT_OFFSET(scrVmPub_t, localVars, 0x00);
ASSERT_OFFSET(scrVmPub_t, maxstack, 0x08);
ASSERT_OFFSET(scrVmPub_t, function_count, 0x10);
ASSERT_OFFSET(scrVmPub_t, function_frame, 0x18);
ASSERT_OFFSET(scrVmPub_t, top, 0x20);
ASSERT_OFFSET(scrVmPub_t, debugCode, 0x28);
ASSERT_OFFSET(scrVmPub_t, abort_on_error, 0x29);
ASSERT_OFFSET(scrVmPub_t, terminal_error, 0x2A);
ASSERT_OFFSET(scrVmPub_t, block_execution, 0x2B);
ASSERT_OFFSET(scrVmPub_t, callNesting, 0x30);
ASSERT_OFFSET(scrVmPub_t, inparamcount, 0x34);
ASSERT_OFFSET(scrVmPub_t, outparamcount, 0x38);
ASSERT_OFFSET(scrVmPub_t, function_frame_start, 0x40);
ASSERT_SIZE(scrVmPub_t, 0x8A40);

typedef ScrPool<scrVmPub_t> ScrVmPubPool;

PACKED(struct scrVmGlob_t {
  var::ScrVarValue_t eval_stack[2];
  const char *dialog_error_message;
  int32_t loading;
  int32_t starttime;
  var::ScrVarIndex_t localVarsStack[4096];
});
typedef ScrPool<scrVmGlob_t> ScrVmGlobPool;

PACKED(union VectorConstant {
  uint8_t value;
  struct {
    // Defined from Least Significant Bit (LSB) to Most Significant Bit (MSB)
    uint8_t z_neg : 1; // Bit 0 (0x01)
    uint8_t z_pos : 1; // Bit 1 (0x02)

    uint8_t y_neg : 1; // Bit 2 (0x04)
    uint8_t y_pos : 1; // Bit 3 (0x08)

    uint8_t x_neg : 1; // Bit 4 (0x10)
    uint8_t x_pos : 1; // Bit 5 (0x20)

    uint8_t unused : 2; // Bits 6 & 7 (0x40, 0x80)
  };

  inline constexpr vec3_t unpack() const noexcept {
    vec3_t result;

    if (x_pos) {
      result.x = 1.0f;
    } else if (x_neg) {
      result.x = -1.0f;
    }

    if (y_pos) {
      result.y = 1.0f;
    } else if (y_neg) {
      result.y = -1.0f;
    }

    if (z_pos) {
      result.z = 1.0f;
    } else if (z_neg) {
      result.z = -1.0f;
    }

    return result;
  }

  inline static constexpr bool can_pack(float component) {
    return component == 1.0f || component == 0.0f || component == -1.0f;
  }

  inline static constexpr bool can_pack(float x, float y, float z) {
    return can_pack(x) && can_pack(y) && can_pack(z);
  }

  inline static constexpr bool can_pack(const vec3_t &vec) {
    return can_pack(vec.x, vec.y, vec.z);
  }

  inline static constexpr VectorConstant pack(float x, float y, float z) {
    VectorConstant result{0};

    if (x == 1.0f) {
      result.x_pos = 1;
    } else if (x == -1.0f) {
      result.x_neg = 1;
    }

    if (y == 1.0f) {
      result.y_pos = 1;
    } else if (y == -1.0f) {
      result.y_neg = 1;
    }

    if (z == 1.0f) {
      result.z_pos = 1;
    } else if (z == -1.0f) {
      result.z_neg = 1;
    }

    return result;
  }

  inline static constexpr VectorConstant pack(const vec3_t &vec) {
    return pack(vec.x, vec.y, vec.z);
  }

  inline constexpr operator uint8_t() const noexcept { return value; }
});

} // namespace vm
} // namespace scr
} // namespace game