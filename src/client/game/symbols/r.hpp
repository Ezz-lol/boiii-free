#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace r {
WEAK symbol<void(const GfxWindowParms *wndParms)> R_StoreWindowSettings{
    0x141CBDB90, 0x140386C30};
WEAK symbol<void()> R_StreamPopDisable{0x141D0FDB0, 0x1403BB430};
WEAK symbol<void()> R_StreamPushDisable{0x141D0FDD0, 0x1403BB470};

struct StreamGuard {
  StreamGuard() { R_StreamPushDisable(); }
  ~StreamGuard() { R_StreamPopDisable(); }
};

WEAK symbol<vidConfig_t> vidConfig{0x14F4E3C30, 0x147D0A8B0};

} // namespace r
} // namespace game