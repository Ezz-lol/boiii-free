#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

WEAK symbol<EngineDependentDvarMut> fileshareAllowDownload{0x150BCF048,
                                                           0x1482944F8};
WEAK symbol<EngineDependentDvarMut> fileshareAllowDownloadingOthersFiles{
    0x150BCF068, 0x148294518};
WEAK symbol<EngineDependentDvarMut> fileshareAllowEmblemDownload{0x150BCF060,
                                                                 0x148294510};
WEAK symbol<EngineDependentDvarMut> fileshareAllowPaintjobDownload{0x150BCF050,
                                                                   0x148294500};
WEAK symbol<EngineDependentDvarMut> fileshareAllowVariantDownload{0x150BCF058,
                                                                  0x148294508};
WEAK symbol<EngineDependentDvarMut> fileshare_enabled{0x150BCEFD0, 0x148294480};
WEAK symbol<EngineDependentDvarMut> fileshareRetry{0x150BCF038, 0x1482944E8};
WEAK symbol<EngineDependentDvarMut> fileshareShowFailure{0x150BCF040,
                                                         0x1482944F0};
WEAK symbol<EngineDependentDvarMut> fileshare_tier{0x150BCEFD8, 0x148294488};
} // namespace game
