#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> storage_backoff{0x15788EAA0, 0x14A2D25D0};
WEAK symbol<EngineDependentDvarMut> storage_maxfailures{0x15788EA98,
                                                        0x14A2D25C8};
WEAK symbol<EngineDependentDvarMut> storage_offline{0x15788EA90, 0x14A2D25C0};
} // namespace game
