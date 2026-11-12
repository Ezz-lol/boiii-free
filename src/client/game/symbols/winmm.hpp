#pragma once

#include "game/symbols/sym_include.hpp"
namespace game {
namespace winmm {
WEAK symbol<
    stdcallPtr_t<MMRESULT(HMIXEROBJ hmxobj, LPMIXERLINEA pmxl, DWORD fdwInfo)>>
    mixerGetLineInfoA{0x15AA6A28C, 0x14B4BD788};
WEAK symbol<stdcallPtr_t<UINT()>> waveInGetNumDevs{0x15AA6A2B4, 0x14B4BD760};
WEAK symbol<stdcallPtr_t<UINT()>> mixerGetNumDevs{0x15AA6A2BC, 0x14B4BD768};
} // namespace winmm
} // namespace game
