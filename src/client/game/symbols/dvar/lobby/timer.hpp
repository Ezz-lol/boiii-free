#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> lobbytimerstartinterval{
    0x1556CE8F0, 0x15574D860, 0x148581390};
WEAK symbol<EngineDependentDvarMut> lobbytimerstatusbegininterval{
    0x1556CE910, 0x15574D880, 0x1485813B0};
WEAK symbol<EngineDependentDvarMut> lobbytimerstatuspostgameinterval{
    0x1556CE920, 0x15574D890, 0x1485813C0};
WEAK symbol<EngineDependentDvarMut> lobbytimerstatusstartinterval{
    0x1556CE918, 0x15574D888, 0x1485813B8};
WEAK symbol<EngineDependentDvarMut> lobbytimerstatusvotinginterval{
    0x1556CE908, 0x15574D878, 0x1485813A8};
WEAK symbol<EngineDependentDvarMut> lobbytimerstatusvotinginterval_arena{
    0x1556CE928, 0x15574D898, 0x1485813C8};
WEAK symbol<EngineDependentDvarMut> lobbytimerzmstartinterval{
    0x1556CE930, 0x15574D8A0, 0x1485813D0};
WEAK symbol<EngineDependentDvarMut> LobbyCPTimerStartInterval{0x1556CE8F8, 0x0,
                                                              0x148581398};
WEAK symbol<EngineDependentDvarMut> lobbycpzmtimerstartinterval{
    0x1556CE900, 0x15574D870, 0x1485813A0};

} // namespace game
