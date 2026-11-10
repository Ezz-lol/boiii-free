#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> metPlayerInGameOnly{0x1511D1C80, 0x14831F120};
WEAK symbol<EngineDependentDvarMut> metPlayerListCleanupInterval{0x1511D1CA0, 0x14831F140};
WEAK symbol<EngineDependentDvarMut> metPlayerListRemoveFriends{0x1511D1C88, 0x14831F128};
WEAK symbol<EngineDependentDvarMut> metPlayerListUpdateInterval{0x1511D1C90, 0x14831F130};
WEAK symbol<EngineDependentDvarMut> metPlayerListUploadInterval{0x1511D1C98, 0x14831F138};
WEAK symbol<EngineDependentDvarMut> metPlayerPlatformGamertag{0x1511D1CC0, 0x14831F160};
WEAK symbol<EngineDependentDvarMut> metPlayerPlatformPresence{0x1511D1CB8, 0x14831F158};
WEAK symbol<EngineDependentDvarMut> maxMetPlayerListCount{0x1511D1CA8, 0x14831F148};

} // namespace game
