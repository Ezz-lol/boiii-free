#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> session_nonblocking{0x151250880,
                                                        0x148340218};
WEAK symbol<EngineDependentDvarMut> sessionSearchMaxAttempts{0x1513958A0,
                                                             0x148374628};
WEAK symbol<EngineDependentDvarMut> sessiontaskfaildebug{0x15118D790,
                                                         0x1482DEED8};
} // namespace game
