#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace seh {
WEAK symbol<const char *(const char *reference)> SEH_SafeTranslateString{
    0x14221C9F0, 0x142279510, 0x0};
WEAK symbol<int32_t()> SEH_GetCurrentLanguage{0x14221BA40, 0x0, 0x0};
WEAK symbol<bool(int32_t iLanguage)> SEH_IsLanguageReversed{0x14221BE90, 0x0,
                                                            0x0};
} // namespace seh
} // namespace game
