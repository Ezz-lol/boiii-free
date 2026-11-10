#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

WEAK symbol<EngineDependentDvarMut> inventory_blocking{0x15114E3E8, 0x0};
WEAK symbol<EngineDependentDvarMut> inventory_enabled{0x15114E3A0, 0x0};
WEAK symbol<EngineDependentDvarMut> inventory_fetch_cooloff{0x15114E3C0, 0x0};
WEAK symbol<EngineDependentDvarMut> inventory_itemsPerPage{0x15114E3B8, 0x0};
WEAK symbol<EngineDependentDvarMut> inventory_maxPages{0x15114E3B0, 0x0};
WEAK symbol<EngineDependentDvarMut> inventory_retry_delay{0x15114E3C8, 0x0};
WEAK symbol<EngineDependentDvarMut> inventory_retry_max{0x15114E3D0, 0x0};
WEAK symbol<EngineDependentDvarMut> inventory_test_button_visible{0x15114E3A8,
                                                                  0x0};
} // namespace game
