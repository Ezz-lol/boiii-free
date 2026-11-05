#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> lobby_clientcontenttimeout{
    0x1556CE848, 0x15574D7B8, 0x1485812E8};
WEAK symbol<EngineDependentDvarMut> lobby_clientendcoolofftime{
    0x1556CE850, 0x15574D7C0, 0x1485812F0};
WEAK symbol<EngineDependentDvarMut> lobby_clientendfakesendloss{
    0x1556CE858, 0x15574D7C8, 0x1485812F8};
WEAK symbol<EngineDependentDvarMut> lobby_clientloadingintouicheck{
    0x1556CE860, 0x15574D7D0, 0x148581300};
WEAK symbol<EngineDependentDvarMut> lobby_clienttimeout{
    0x1556CE838, 0x15574D7A8, 0x1485812D8};
WEAK symbol<EngineDependentDvarMut> lobby_clientwarntimeout{
    0x1556CE840, 0x15574D7B0, 0x1485812E0};
} // namespace game
