#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> chatClientEnabled{0x150BC3968, 0x150C428E8,
                                                      0x0};
WEAK symbol<EngineDependentDvarMut> chatTimeInGame{0x150BC3970, 0x150C428F0,
                                                   0x0};
WEAK symbol<EngineDependentDvarMut> chatTimeLobby{0x150BC3940, 0x150C428C0,
                                                  0x0};
} // namespace game
