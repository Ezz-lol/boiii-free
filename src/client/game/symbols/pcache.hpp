#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace pcache {
// PCache
WEAK symbol<void(ControllerIndex_t controllerIndex)> PCache_DeleteEntries{
    0x141E8D710};
} // namespace pcache
} // namespace game

#endif
