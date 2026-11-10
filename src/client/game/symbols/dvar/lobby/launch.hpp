#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> lobbylaunch_fadetoblackdelay{0x1556CE868,
                                                                 0x148581308};
WEAK symbol<EngineDependentDvarMut> lobbylaunch_fadetoblackdelayonconnect{
    0x1556CE878, 0x148581318};
WEAK symbol<EngineDependentDvarMut> lobbylaunch_gamelaunchdelay{0x1556CE870,
                                                                0x148581310};
WEAK symbol<EngineDependentDvarMut> lobbylaunch_gamelaunchdelayonconnect{
    0x1556CE880, 0x148581320};
WEAK symbol<EngineDependentDvarMut> lobbylaunch_waitforclientackdelay{
    0x1556CE888, 0x148581328};
WEAK symbol<EngineDependentDvarMut> lobby_beginplay{0x1556CE940, 0x1485813E0};

} // namespace game
