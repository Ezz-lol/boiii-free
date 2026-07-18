#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace cmd {
WEAK symbol<void(scriptInstance_t inst, scr_entref_t *entref)>
    ScrCmd_PlaySoundWithNotify{0x141AEA4D0, 0x1402E66A0};
WEAK symbol<void(scriptInstance_t inst, scr_entref_t *entref)>
    ScrCmd_PlaySoundOnTag{0x141AE6D10, 0x1402E6320};
WEAK symbol<void(scriptInstance_t inst, scr_entref_t *entref)>
    ScrCmd_PlaySoundToPlayer{0x1415FB310, 0x140259EE0};
WEAK symbol<void(scriptInstance_t inst, scr_entref_t *entref)>
    ScrCmd_PlaySoundToTeam{0x141AE8970, 0x1402E6550};
WEAK symbol<void(scriptInstance_t inst, scr_entref_t *entref)> ScrCmd_PlaySound{
    0x141AE6CA0, 0x1402E62B0};
WEAK symbol<void(scriptInstance_t inst, scr_entref_t *entref)>
    ScrCmd_PlaySoundToAllButPlayer{0x1415FB250, 0x140259E00};
} // namespace cmd
} // namespace scr
} // namespace game
