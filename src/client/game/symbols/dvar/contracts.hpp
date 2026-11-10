#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> contracts_daily_duration{0x15118F7A0,
                                                             0x1482E0AE0};
WEAK symbol<EngineDependentDvarMut> contracts_enabled_mp{0x15118F780,
                                                         0x1482E0AC0};
WEAK symbol<EngineDependentDvarMut> contracts_now{0x15118F788, 0x1482E0AC8};
WEAK symbol<EngineDependentDvarMut> contracts_rows_in_set{0x0, 0x1482E0AD0};
WEAK symbol<EngineDependentDvarMut> contracts_start_time{0x15118F798,
                                                         0x1482E0AD8};
} // namespace game
