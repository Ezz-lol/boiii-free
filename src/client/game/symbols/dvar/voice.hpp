#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> maxvoicepacketsperframe{
    0x1453559D8, 0x1453D49D8, 0x1428AAEB0};
WEAK symbol<EngineDependentDvarMut> winvoice_loopback{0x157DF7178, 0x0,
                                                      0x14A641218};
WEAK symbol<EngineDependentDvarMut> winvoice_mic_mute{0x157DF7160, 0x0,
                                                      0x14A641200};
WEAK symbol<EngineDependentDvarMut> winvoice_mic_reclevel{
    0x157DF72C0, 0x157E761F0, 0x14A641348};
WEAK symbol<EngineDependentDvarMut> winvoice_mic_scaler{0x157DF7170, 0x0,
                                                        0x14A641210};
WEAK symbol<EngineDependentDvarMut> winvoice_save_voice{0x157DF7168, 0x0,
                                                        0x14A641208};
WEAK symbol<EngineDependentDvarMut> winvoice_voice_level{0x157DF72B8, 0x0,
                                                         0x14A641340};
} // namespace game
