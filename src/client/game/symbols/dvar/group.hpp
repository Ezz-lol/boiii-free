#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

WEAK symbol<EngineDependentDvarMut> groupcountsvisible{0x1511E12C0,
                                                       0x148326C60};
WEAK symbol<EngineDependentDvarMut> groupdownloadinterval{0x1511E12B8,
                                                          0x148326C58};
WEAK symbol<EngineDependentDvarMut> groups_admins_refresh_time{0x150FCC420,
                                                               0x0};
WEAK symbol<EngineDependentDvarMut> groups_applications_refresh_time{
    0x150FCC430, 0x0};
WEAK symbol<EngineDependentDvarMut> groups_enabled{0x150FCC410, 0x0};
WEAK symbol<EngineDependentDvarMut> groups_invites_refresh_time{0x150FCC428,
                                                                0x0};
WEAK symbol<EngineDependentDvarMut> groups_presence_refresh_time{0x150FCC450,
                                                                 0x0};
WEAK symbol<EngineDependentDvarMut> groups_quickjoin_players_refresh_time{
    0x150FCC438, 0x0};
WEAK symbol<EngineDependentDvarMut> groups_self_groups_refresh_time{0x150FCC418,
                                                                    0x0};
WEAK symbol<EngineDependentDvarMut> groups_service_failure_backoff_time{
    0x150FCC440, 0x0};
WEAK symbol<EngineDependentDvarMut> groups_wait_on_dw{0x150FCC448, 0x0};
WEAK symbol<EngineDependentDvarMut> groupuploadinterval{0x1511E12B0,
                                                        0x148326C50};
WEAK symbol<EngineDependentDvarMut> groupzerocountsvisible{0x1511E12C8,
                                                           0x148326C68};
} // namespace game
