#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

WEAK symbol<EngineDependentDvarMut> lobbyDebugLogJoins{0x1556CE8D8, 0x15574D848,
                                                       0x148581378};
WEAK symbol<EngineDependentDvarMut> lobbyDebugLogJoinSuccess{
    0x1556CE8E0, 0x15574D850, 0x148581380};
WEAK symbol<EngineDependentDvarMut> lobbymsg_prints{0x1556CE8D0, 0x15574D840,
                                                    0x148581370};
WEAK symbol<EngineDependentDvarMut> lobby_showSVDebug{0x1556CE458, 0x15574D3C8,
                                                      0x148580EF8};
WEAK symbol<EngineDependentDvarMut> ui_lobbyDebugOverlay{
    0x1556CE398, 0x15574D308, 0x148580E38};
WEAK symbol<EngineDependentDvarMut> ui_lobbyDebugSessionSQJ{
    0x1556CE3B8, 0x15574D328, 0x148580E58};
WEAK symbol<EngineDependentDvarMut> ui_lobbyDebugVis{0x1556CE390, 0x15574D300,
                                                     0x148580E30};
WEAK symbol<EngineDependentDvarMut> ui_lobbyDebugVoice{0x1556CE3C0, 0x15574D330,
                                                       0x148580E60};
} // namespace game
