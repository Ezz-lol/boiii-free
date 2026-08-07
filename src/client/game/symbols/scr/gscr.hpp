#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace gscr {
WEAK symbol<void(scriptInstance_t inst, scr_entref_t entref)>
    GScr_SetWorldFogActiveBank{0x0, 0x1402DF580};

WEAK symbol<void(scriptInstance_t inst, scr_entref_t entref)>
    GScr_GetBGBTokensRemaining{0x141A8CAB0, 0x1402D2310};
WEAK symbol<void(scriptInstance_t inst)> GScr_PIXBeginEvent{0x0, 0x1402DA730};
WEAK symbol<void(scriptInstance_t inst)> GScr_PIXEndEvent{0x0, 0x140515B60};
WEAK symbol<void()> GScr_LoadConsts{0x141C353B0, 0x14032E640};
// Client only
WEAK symbol<void(scriptInstance_t inst)> GScr_BBPrint{0x1422A0940};

WEAK symbol<int32_t(const char *pszMenu)> GScr_GetScriptMenuIndex{0x141A93CA0,
                                                                  0x1402D4590};

} // namespace gscr
} // namespace scr
} // namespace game