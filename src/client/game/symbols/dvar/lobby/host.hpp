#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> lobby_hostcontenttimeout{
    0x1556CE778, 0x15574D6E8, 0x148581218};
WEAK symbol<EngineDependentDvarMut> lobby_hostintervalheartbeat{
    0x1556CE770, 0x15574D6E0, 0x148581210};
WEAK symbol<EngineDependentDvarMut> lobby_hostintervalstate{
    0x1556CE768, 0x15574D6D8, 0x148581208};
WEAK symbol<EngineDependentDvarMut> lobby_hostjoinrequestreject{
    0x1556CE7B8, 0x15574D728, 0x148581258};
WEAK symbol<EngineDependentDvarMut> lobby_hostpsfragmentretryinterval{
    0x1556CE790, 0x15574D700, 0x148581230};
WEAK symbol<EngineDependentDvarMut> lobby_hosttimeout{0x1556CE760, 0x15574D6D0,
                                                      0x148581200};
} // namespace game
