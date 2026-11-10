#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> gunsmith_enabled{0x15114E370, 0x1482DECF0};
WEAK symbol<EngineDependentDvarMut> gunsmith_enable_toggle_variant{0x15114E380, 0x1482DED00};
WEAK symbol<EngineDependentDvarMut> gunsmith_totalvariants{0x15114E368, 0x1482DECE8};
WEAK symbol<EngineDependentDvarMut> gunsmithVersion{0x15114E378, 0x1482DECF8};
} // namespace game
