#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace seh {
WEAK symbol<const char *(const char *reference)> SEH_SafeTranslateString{
    0x14221C9F0, 0x142279510, 0x0};
}
} // namespace game
