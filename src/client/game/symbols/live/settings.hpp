#pragma once

#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace live {
namespace settings {
WEAK symbol<CachedSettingsProfileKeys> s_codeCachedProfileKeys{0x14333E430};

WEAK symbol<SettingsGlob> s_settingsGlob{0x14A08A5CC};
} // namespace settings
} // namespace live
} // namespace game

#endif