#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> language{0x15789FE58, 0x14A2DB018};

WEAK symbol<EngineDependentDvarMut> dvar_loc_availableLanguages{0x0,
                                                                0x14A2DB048};
WEAK symbol<EngineDependentDvarMut> dvar_loc_forceenglish{0x0, 0x14A2DB058};
WEAK symbol<EngineDependentDvarMut> dvar_loc_language{0x15789FE48, 0x14A2DB050};
WEAK symbol<EngineDependentDvarMut> dvar_loc_languagesaved{0x0, 0x14A2DB040};
WEAK symbol<EngineDependentDvarMut> dvar_loc_systemlanguage{0x0, 0x14A2DB020};
WEAK symbol<EngineDependentDvarMut> dvar_loc_warnings{0x15789FE70, 0x14A2DB030};
WEAK symbol<EngineDependentDvarMut> dvar_loc_warningsaserrors{0x15789FE78,
                                                              0x14A2DB038};
} // namespace game
