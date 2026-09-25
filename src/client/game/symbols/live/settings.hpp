#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace settings {
WEAK symbol<CachedSettingsProfileKeys> s_codeCachedProfileKeys{
    0x1432BF430, 0x14333E430, 0x0};

WEAK symbol<SettingsGlob> s_settingsGlob{0x14A00B5CC, 0x14A08A5CC, 0x0};
} // namespace settings
} // namespace live
} // namespace game
