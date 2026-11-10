#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> wind_debug_display{0x0, 0x14B395FA0};
WEAK symbol<EngineDependentDvarMut> wind_global_hi_altitude{0x159BF75E8, 0x14B395F68};
WEAK symbol<EngineDependentDvarMut> wind_global_low_altitude{0x159BF75D8, 0x14B395F58};
WEAK symbol<EngineDependentDvarMut> wind_global_low_strength_percent{0x159BF75E0, 0x14B395F60};
WEAK symbol<EngineDependentDvarMut> wind_global_vector{0x159BF75D0, 0x14B395F50};
WEAK symbol<EngineDependentDvarMut> wind_grass_gust_distance{0x159BF7640, 0x14B395FC0};
WEAK symbol<EngineDependentDvarMut> wind_grass_gustinterval{0x159BF7618, 0x14B395F98};
WEAK symbol<EngineDependentDvarMut> wind_grass_gust_radius{0x159BF7630, 0x14B395FB0};
WEAK symbol<EngineDependentDvarMut> wind_grass_gust_speed{0x159BF7628, 0x14B395FA8};
WEAK symbol<EngineDependentDvarMut> wind_grass_gust_strength{0x159BF7638, 0x14B395FB8};
WEAK symbol<EngineDependentDvarMut> wind_grass_scale{0x159BF75F8, 0x14B395F78};
WEAK symbol<EngineDependentDvarMut> wind_grass_tension{0x159BF7600, 0x14B395F80};
WEAK symbol<EngineDependentDvarMut> wind_leaf_scale{0x159BF75F0, 0x14B395F70};
} // namespace game
