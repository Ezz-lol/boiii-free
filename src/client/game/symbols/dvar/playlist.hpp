#pragma once

#include <game/symbols/sym_include.hpp>

namespace game { // namespace game
WEAK symbol<EngineDependentDvarMut> playlist_allowdevmaps{
    0x15796EAF0, 0x1579ED9F0, 0x14A31A410};
WEAK symbol<EngineDependentDvarMut> playlist_autoeventsenabled{0x0, 0x1579EDA30,
                                                               0x14A31A450};
WEAK symbol<EngineDependentDvarMut> playlist_excludegametype{
    0x15796EB00, 0x1579EDA00, 0x14A31A420};
WEAK symbol<EngineDependentDvarMut> playlist_excludegametypemap{
    0x15796EB08, 0x1579EDA08, 0x14A31A428};
WEAK symbol<EngineDependentDvarMut> playlist_excludemap{
    0x15796EAF8, 0x1579ED9F8, 0x14A31A418};
WEAK symbol<EngineDependentDvarMut> playlist_linearmapcycle{
    0x15796EB10, 0x1579EDA10, 0x14A31A430};
WEAK symbol<EngineDependentDvarMut> playlist_linearmapcyclerandom{
    0x15796EB18, 0x1579EDA18, 0x14A31A438};
} // namespace game
