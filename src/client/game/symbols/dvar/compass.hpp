#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> compass{0x144785448, 0x144804448,
                                            0x14201EF88};
WEAK symbol<EngineDependentDvarMut> compassDrawHackerPing{
    0x144785490, 0x144804490, 0x14201F028};
WEAK symbol<EngineDependentDvarMut> compassDrawLastStandIcon{
    0x144785498, 0x144804498, 0x14201F030};
WEAK symbol<EngineDependentDvarMut> compassGridAlign{0x1447854B8, 0x1448044B8,
                                                     0x14201F050};
WEAK symbol<EngineDependentDvarMut> compassGridCols{0x1447854B0, 0x1448044B0,
                                                    0x14201F048};
WEAK symbol<EngineDependentDvarMut> compassGridEnabled{0x1447854A0, 0x1448044A0,
                                                       0x14201F038};
WEAK symbol<EngineDependentDvarMut> compassGridRows{0x1447854A8, 0x1448044A8,
                                                    0x14201F040};
WEAK symbol<EngineDependentDvarMut> compassmaxrange{0x144785510, 0x144804510,
                                                    0x14201EF40};
WEAK symbol<EngineDependentDvarMut> compassObjectiveHeight{
    0x144785458, 0x144804458, 0x14201EFF0};
WEAK symbol<EngineDependentDvarMut> compassObjectiveIconHeight{
    0x144785468, 0x144804468, 0x14201F000};
WEAK symbol<EngineDependentDvarMut> compassObjectiveIconWidth{
    0x144785460, 0x144804460, 0x14201EFF8};
WEAK symbol<EngineDependentDvarMut> compassObjectiveMaxHeight{
    0x144785488, 0x144804488, 0x14201F020};
WEAK symbol<EngineDependentDvarMut> compassObjectiveMinDistRange{
    0x144785478, 0x144804478, 0x14201F010};
WEAK symbol<EngineDependentDvarMut> compassObjectiveMinHeight{
    0x144785480, 0x144804480, 0x14201F018};
WEAK symbol<EngineDependentDvarMut> compassObjectiveNearbyDist{
    0x144785470, 0x144804470, 0x14201F008};
WEAK symbol<EngineDependentDvarMut> compassObjectiveWidth{
    0x144785450, 0x144804450, 0x14201EFE8};
} // namespace game
