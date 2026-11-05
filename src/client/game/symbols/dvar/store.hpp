#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> store_item_viewed_timer{0x15188CE18,
                                                            0x15190BD88, 0x0};
WEAK symbol<EngineDependentDvarMut> storeMapPackMaskToStats{
    0x1515C0CB0, 0x15163FC20, 0x148382938};

} // namespace game
