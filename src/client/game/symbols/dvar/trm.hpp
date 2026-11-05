#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> traverse_mode{0x159BAB8F0, 0x159C2A870,
                                                  0x14B34A3A8};
WEAK symbol<EngineDependentDvarMut> trm_enable{0x1441DBF40, 0x14425AF40,
                                               0x141E5CF00};
WEAK symbol<EngineDependentDvarMut> trm_enabled{0x159BAB888, 0x159C2A808,
                                                0x14B34A378};
} // namespace game
