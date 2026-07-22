#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace cmd {
namespace he {

WEAK symbol<void(scriptInstance_t inst, scr_entref_t *entref)> HECmd_SetText{
    0x141990700, 0x1402A0BA0};
}
} // namespace cmd
} // namespace scr
} // namespace game