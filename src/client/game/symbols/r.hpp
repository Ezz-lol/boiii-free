#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace r {
WEAK symbol<void(const GfxWindowParms *wndParms)> R_StoreWindowSettings{
    0x141CB17C0, 0x141CBDB90, 0x140386C30};
WEAK symbol<void()> R_StreamPopDisable{0x141D039E0, 0x141D0FDB0, 0x1403BB430};
WEAK symbol<void()> R_StreamPushDisable{0x141D03A00, 0x141D0FDD0, 0x1403BB470};

/*
  I am not at all confident about the name, signature, functionality, etc,
  behind this function.

  Its entire callstack is brand new, PC-specific, and contains very complex
  logic related to GPU command submission and result handling.

  TODO: completely reverse engineer this function and callstack.
*/
WEAK symbol<void(void *a1, void *a2, uint32_t a3, int32_t a4, int32_t a5,
                 void *a6, int32_t a7, int32_t a8, int32_t a9, int32_t a10,
                 int32_t a11, int32_t a12, int32_t a13, int32_t a14)>
    R_CopyTextureRegionMips{0x141CAB1F0, 0x141CB75C0, 0x140383950};

struct StreamGuard {
  StreamGuard() { R_StreamPushDisable(); }
  ~StreamGuard() { R_StreamPopDisable(); }
};

WEAK symbol<vidConfig_t> vidConfig{0x14F464CB0, 0x14F4E3C30, 0x147D0A8B0};

} // namespace r
} // namespace game
