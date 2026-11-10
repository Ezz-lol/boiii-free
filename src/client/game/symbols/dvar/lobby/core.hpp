#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

WEAK symbol<EngineDependentDvarMut> lobbyDebugLogJoins{0x1556CE8D8, 0x148581378};
WEAK symbol<EngineDependentDvarMut> lobbyDebugLogJoinSuccess{0x1556CE8E0, 0x148581380};
WEAK symbol<EngineDependentDvarMut> lobbymsg_prints{0x1556CE8D0, 0x148581370};
WEAK symbol<EngineDependentDvarMut> lobby_showSVDebug{0x1556CE458, 0x148580EF8};
WEAK symbol<EngineDependentDvarMut> ui_lobbyDebugOverlay{0x1556CE398, 0x148580E38};
WEAK symbol<EngineDependentDvarMut> ui_lobbyDebugSessionSQJ{0x1556CE3B8, 0x148580E58};
WEAK symbol<EngineDependentDvarMut> ui_lobbyDebugVis{0x1556CE390, 0x148580E30};
WEAK symbol<EngineDependentDvarMut> ui_lobbyDebugVoice{0x1556CE3C0, 0x148580E60};
WEAK symbol<EngineDependentDvarMut> lobby_enableloadoutdatastreamingingame{0x1556CE788, 0x148581228};
WEAK symbol<EngineDependentDvarMut> lobby_enablepsdatastreamingingame{0x1556CE780, 0x148581220};
WEAK symbol<EngineDependentDvarMut> lobby_enablepsdatastreaminginlobby{0x1556CE798, 0x148581238};
WEAK symbol<EngineDependentDvarMut> lobby_forceoffline{0x1556CE450, 0x148580EF0};

WEAK symbol<EngineDependentDvarMut> lobby_ingamepsdatastreamingbw{0x1556CE7A8, 0x148581248};
WEAK symbol<EngineDependentDvarMut> lobbymapvoteprevmapplaycount{0x1556CE938, 0x1485813D8};
WEAK symbol<EngineDependentDvarMut> lobby_maxlocalplayers{0x1556CE448, 0x148580EE8};
WEAK symbol<EngineDependentDvarMut> lobby_movelobby{0x1556CE7B0, 0x148581250};
WEAK symbol<EngineDependentDvarMut> lobby_ptptimeout{0x1556CE8E8, 0x148581388};
WEAK symbol<EngineDependentDvarMut> lobby_readyuppercentrequired{0x1556CE948, 0x1485813E8};

} // namespace game
