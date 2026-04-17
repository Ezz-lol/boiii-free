#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace pmem {
// PMem
WEAK symbol<void(const char *name, int allocDir)> PMem_Free{0x141439190,
                                                            0x1405D8B30};
} // namespace pmem
} // namespace game

#endif
