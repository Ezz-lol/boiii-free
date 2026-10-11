#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace cg {
namespace voice {
WEAK symbol<db::xasset::MaterialHandle(LocalClientNum_t localClientNum,
                                       ClientNum_t clientNum)>
    CG_IsInGamePlayerListPlayerTalking{0x140A11F30, 0x0, 0x0};
WEAK symbol<db::xasset::MaterialHandle(LocalClientNum_t localClientNum,
                                       ClientNum_t clientNum,
                                       lobby::LobbyType lobbyType)>
    CG_IsPlayerListPlayerTalking{0x140A11F40, 0x0, 0x0};
WEAK symbol<void()> CG_TeamVoiceChat_f{0x1405DE620, 0x1405DE620, 0x1400B47F0};
WEAK symbol<void()> CG_VoiceChat_f{0x1405E32A0, 0x1405E32A0, 0x1400B4B50};
} // namespace voice
} // namespace cg
} // namespace game
