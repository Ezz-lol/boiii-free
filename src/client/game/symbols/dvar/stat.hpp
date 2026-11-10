#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> statsCheckXuidBeforeStatsUpload{0x1515C0CC0, 0x148382948};
WEAK symbol<EngineDependentDvarMut> statsErrorBackup{0x1515C0CD0, 0x148382958};
WEAK symbol<EngineDependentDvarMut> statsErrorNormal{0x1515C0CC8, 0x148382950};
WEAK symbol<EngineDependentDvarMut> statsErrorOtherPlayer{0x1515C0CE0, 0x148382968};
WEAK symbol<EngineDependentDvarMut> statsErrorStable{0x1515C0CD8, 0x148382960};
WEAK symbol<EngineDependentDvarMut> statsLocationFatal{0x1515C0CB8, 0x148382940};
WEAK symbol<EngineDependentDvarMut> stats_version_check{0x151640E20, 0x148382870};
WEAK symbol<EngineDependentDvarMut> statsVersionCP{0x1513A4668, 0x14837E358};
WEAK symbol<EngineDependentDvarMut> statsVersionFR{0x1513A4678, 0x14837E368};
WEAK symbol<EngineDependentDvarMut> statsVersionMP{0x1513A4660, 0x14837E350};
WEAK symbol<EngineDependentDvarMut> statsVersionZM{0x1513A4670, 0x14837E360};
WEAK symbol<EngineDependentDvarMut> stat_version{0x151640E28, 0x1483829A0};
WEAK symbol<EngineDependentDvarMut> maxStatsBackupInterval{0x1515C0C30, 0x1483828B8};
WEAK symbol<EngineDependentDvarMut> zero_stats_check{0x1515C0C28, 0x1483828B0};
WEAK symbol<EngineDependentDvarMut> waitOnStatsTimeout{0x1515C0C58, 0x1483828E0};

} // namespace game
