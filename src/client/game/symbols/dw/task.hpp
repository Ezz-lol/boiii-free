#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace dw {
namespace task {
WEAK symbol<void(TaskRecord *task)> TaskManager2_ProcessDemonwareTask{
    0x1422B0260};
}
} // namespace dw
} // namespace game