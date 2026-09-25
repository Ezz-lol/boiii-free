#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace pcache {
// PCache
WEAK symbol<void(ControllerIndex_t controllerIndex)> PCache_DeleteEntries{
    0x141E80C80, 0x141E8D710, 0x0};
} // namespace pcache
} // namespace game
