#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> M_Filter{0x0, 0x0, 0x1428B0BB8};
WEAK symbol<EngineDependentDvarMut> m_forward{0x0, 0x0, 0x1428B0BA8};
WEAK symbol<EngineDependentDvarMut> M_MouseAcceleration{0x0, 0x0, 0x1428B0BC8};
WEAK symbol<EngineDependentDvarMut> M_MouseFilter{0x0, 0x0, 0x1428B0BC0};
WEAK symbol<EngineDependentDvarMut> m_mousesensitivity{0x0, 0x0, 0x1428B0BD0};
WEAK symbol<EngineDependentDvarMut> m_pitch{0x0, 0x1453DC140, 0x1428B0B98};
WEAK symbol<EngineDependentDvarMut> m_side{0x0, 0x0, 0x1428B0BB0};
WEAK symbol<EngineDependentDvarMut> M_Yaw{0x0, 0x0, 0x1428B0BA0};
} // namespace game
