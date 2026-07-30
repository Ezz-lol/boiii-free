#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace r {
WEAK symbol<void(const GfxWindowParms *wndParms)> R_StoreWindowSettings{
    0x141CBDB90, 0x140386C30};
WEAK symbol<vidConfig_t> vidConfig{0x14F4E3C30, 0x147D0A8B0};
} // namespace r
} // namespace game