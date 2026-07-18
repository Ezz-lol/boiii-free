#pragma once

#include <game/symbols/sym_include.hpp>


namespace game {
namespace seh {
WEAK symbol<const char *(const char *reference)> SEH_SafeTranslateString{
    0x142279510};
}
} // namespace game
