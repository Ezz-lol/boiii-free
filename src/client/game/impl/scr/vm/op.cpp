#include <std_include.hpp>
#include "op.hpp"

namespace game {
namespace scr {
namespace vm {
namespace op {
void VM_OP_GetTime_Handler_Impl(scriptInstance_t inst,
                                volatile function_stack_t *fs,
                                volatile ScrVmContext_t *vmc,
                                volatile bool *terminate) {
  using namespace game::level::cl;
  using namespace game::cl;

  switch (inst) {
  case SCRIPTINSTANCE_SERVER: {
    fs->top[1].type = var::ScrVarType::INT;
    fs->top[1].u.intValue = *level::level_time;
    break;
  }
  case SCRIPTINSTANCE_CLIENT: {
    if (*primaryLocalClientNum < *cl_maxLocalClients) {
      fs->top[1].type = var::ScrVarType::INT;
      fs->top[1].u.intValue = game::cg::cgArray[*primaryLocalClientNum].time;
    } else {
      goto unreachable;
    }
    break;
  }
  unreachable:
  default: {
    fs->top[1].type = var::ScrVarType::UNDEFINED;
    fs->top[1].u.uintptrValue = 0;
    break;
  }
  }
  // Incrementing volatile pointer is deprecated in C++ 20
  fs->top += 1;
}
} // namespace op
} // namespace vm
} // namespace scr
} // namespace game