#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lua {
namespace lui {
WEAK symbol<bool()> UI_IsInitialized{0x14268C590, 0x0, 0x0};
WEAK symbol<db::xasset::MaterialHandle> Lua_DefaultMaterial{0x156262C88, 0x0,
                                                            0x0};
} // namespace lui
} // namespace lua
} // namespace game
