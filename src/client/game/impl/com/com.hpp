#pragma once

#include <game/game.hpp>

namespace game {
namespace com {

int32_t Com_GetBuildIntField_Impl(BuildIntField field);
const char *Com_GetBuildVersion_Impl();
const char *Com_GetBuildID_Impl();
const char *Com_GetBuildStringField_Impl(BuildStringField field);
} // namespace com
} // namespace game