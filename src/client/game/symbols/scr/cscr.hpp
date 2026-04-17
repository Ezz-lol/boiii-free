#pragma once
#ifdef GAME_HPP

#include "../macros.hpp"

namespace game {
namespace scr {
namespace cscr {
WEAK symbol<void(scriptInstance_t inst)> CScr_SetExposureActiveBank{
    0x0, 0x140092F80};
WEAK symbol<void(scriptInstance_t inst)> CScr_SetLitFogBank{0x0, 0x140093FE0};
WEAK symbol<void(scriptInstance_t inst)> CScr_SetLutVolumeActiveBank{
    0x0, 0x140094120};
WEAK symbol<void(scriptInstance_t inst)> CScr_SetPBGActiveBank{0x0,
                                                               0x140094360};
WEAK symbol<void(scriptInstance_t inst)> CScr_SetWorldFogActiveBank{
    0x0, 0x140094960};
} // namespace cscr
} // namespace scr
} // namespace game

#endif
