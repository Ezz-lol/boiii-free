#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> upload_bps_dedicated{0x15686FF00, 0x14948EE18};
WEAK symbol<EngineDependentDvarMut> upload_bps_demosystem{0x15686FEF0, 0x14948EE08};
WEAK symbol<EngineDependentDvarMut> upload_bps_perplayer{0x15686FEF8, 0x14948EE10};
} // namespace game
