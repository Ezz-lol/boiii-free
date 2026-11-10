#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> platformsessiondebuginfo{0x15138AD10,
                                                             0x14836D970};
WEAK symbol<EngineDependentDvarMut> platformsessionlaunchinvitejoinprocessdelay{
    0x0, 0x14836D988};
WEAK symbol<EngineDependentDvarMut> platformsessionpartyprivacy{0x15138AD20,
                                                                0x14836D980};
WEAK symbol<EngineDependentDvarMut> platformsessionshowerrorcodes{0x15138AD18,
                                                                  0x14836D978};
WEAK symbol<EngineDependentDvarMut> orbisgame{0x0, 0x14948F118};
WEAK symbol<EngineDependentDvarMut> durangogame{0x0, 0x14948F120};
WEAK symbol<EngineDependentDvarMut> consolegame{0x0, 0x14948F128};

} // namespace game
