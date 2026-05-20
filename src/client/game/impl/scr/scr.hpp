#pragma once
#include "../../../game/game.hpp"
#include <cstdint>

namespace game {
namespace scr {

const char *Scr_TypeName(ScrVarType type);
bool Scr_IsTrue_Impl(scriptInstance_t inst, ScrVarValue_t *value);

} // namespace scr
} // namespace game