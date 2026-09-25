#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace dw {
namespace task {
WEAK symbol<void(TaskRecord *task)> TaskManager2_ProcessDemonwareTask{
    0x142253740, 0x1422B0260, 0x0};
}
} // namespace dw
} // namespace game
