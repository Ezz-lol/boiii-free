#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace dw {
namespace task {
WEAK symbol<void(TaskRecord *task)> TaskManager2_ProcessDemonwareTask{
    0x1422B0260};
}
} // namespace dw
} // namespace game

#endif