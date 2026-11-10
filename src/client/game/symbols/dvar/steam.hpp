#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> steamLobby_frameEnable{0x1513A4A20, 0x0};
WEAK symbol<EngineDependentDvarMut> steamLobby_frameFailInterval{0x1513A4A38,
                                                                 0x0};
WEAK symbol<EngineDependentDvarMut> steamLobby_frameInterval{0x1513A4A28, 0x0};
WEAK symbol<EngineDependentDvarMut> steamLobby_frameKeepAliveInterval{
    0x1513A4A30, 0x0};
WEAK symbol<EngineDependentDvarMut> steamP2P_Enable{0x1513A4B88, 0x0};
} // namespace game
