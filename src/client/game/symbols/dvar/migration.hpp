#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> migration_blocksperframe{0x0, 0x0,
                                                             0x14A19D818};
WEAK symbol<EngineDependentDvarMut> migration_forcehost{0x0, 0x0, 0x14A19D838};
WEAK symbol<EngineDependentDvarMut> migration_limit{0x0, 0x0, 0x14A19D828};
WEAK symbol<EngineDependentDvarMut> migration_minclientcount{0x0, 0x0,
                                                             0x14A19D810};
WEAK symbol<EngineDependentDvarMut> migration_msgtimeout{0x0, 0x0, 0x14A19D820};
WEAK symbol<EngineDependentDvarMut> migration_soak{0x0, 0x0, 0x14A19D840};
WEAK symbol<EngineDependentDvarMut> migration_timebetween{0x0, 0x0,
                                                          0x14A19D830};
} // namespace game
