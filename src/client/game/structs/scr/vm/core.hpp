#pragma once

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