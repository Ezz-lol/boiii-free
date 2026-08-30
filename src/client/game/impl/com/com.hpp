#pragma once

#include <game/game.hpp>

namespace game {
namespace com {
const char *Com_GetBuildStringField_Impl(BuildStringField field);
int32_t Com_GetBuildIntField_Impl(BuildIntField field);
} // namespace com
} // namespace game