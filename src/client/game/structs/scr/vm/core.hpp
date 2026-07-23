#pragma once

#include <game/structs/scr/core.hpp>
#include <game/structs/scr/var.hpp>
#include <game/structs/macros.hpp>

namespace game {
namespace scr {
namespace vm {
struct function_stack_t {
  uint8_t *pos;
  var::ScrVarValue_t *top;
  var::ScrVarIndex_t threadId;
  uint32_t localVarCount;
  var::ScrVarValue_t *startTop;
};
ASSERT_SIZE(function_stack_t, 0x20);

struct function_frame_t {
  function_stack_t fs;
  bool nested;
};
ASSERT_SIZE(function_frame_t, 0x28);

#pragma pack(push, 1)
struct ScrVmPub_t {
  uint8_t _pad0[0x10];
  int32_t function_count;
  uint8_t _pad10[4];
  function_frame_t *function_frame;
  uint8_t _pad18[0x8A40 - 0x20];
};
ASSERT_OFFSET(ScrVmPub_t, function_count, 0x10);
ASSERT_OFFSET(ScrVmPub_t, function_frame, 0x18);
ASSERT_SIZE(ScrVmPub_t, 0x8A40);
#pragma pack(pop)

typedef ScrPool<ScrVmPub_t> ScrVmPubPool;

#pragma pack(push, 1)
struct ScrVmContext_t {
  var::ScrVarIndex_t fieldValueId;
  bool fieldValueRemoveOk;
  uint8_t _padding05[3];
  var::ScrVarIndex_t objectId;
  uint8_t _padding0C[4];
  uint8_t *lastGoodPos;
  var::ScrVarValue_t *lastGoodTop;
};
ASSERT_SIZE(ScrVmContext_t, 0x20);
#pragma pack(pop)

} // namespace vm
} // namespace scr
} // namespace game