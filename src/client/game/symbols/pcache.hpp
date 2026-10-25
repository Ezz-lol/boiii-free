#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace pcache {
// PCache
WEAK symbol<void(ControllerIndex_t controllerIndex)> PCache_DeleteEntries{
    0x141E80C80, 0x141E8D710, 0x0};
// Finds or creates a player's cached component. Hold CriticalSection::PCACHE
// while using it.
WEAK symbol<live::pcache::PCacheComponent *(
    ControllerIndex_t controllerIndex, XUID xuid,
    live::pcache::pcacheComponent_t type)>
    PCache_GetComponent{0x141E80CA0, 0x141E8D730, 0x0};
WEAK symbol<void(ControllerIndex_t controllerIndex, XUID xuid)>
    PCache_InvalidatePlayerEmblem{0x141E81580, 0x141E8E010, 0x0};
} // namespace pcache
} // namespace game
