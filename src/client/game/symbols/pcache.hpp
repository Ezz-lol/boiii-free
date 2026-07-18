#pragma once

#include <game/symbols/sym_include.hpp>


namespace game {
namespace pcache {
// PCache
WEAK symbol<void(ControllerIndex_t controllerIndex)> PCache_DeleteEntries{
    0x141E8D710};
} // namespace pcache
} // namespace game

