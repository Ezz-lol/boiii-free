#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> hudelempausedbrightness{0x144949648,
                                                            0x1421DAD00};
WEAK symbol<EngineDependentDvarMut> hud_enable{0x144C98D90, 0x0};
WEAK symbol<EngineDependentDvarMut> hud_fade_ammoDisplay{0x144C98CB0, 0x0};
WEAK symbol<EngineDependentDvarMut> hud_fade_stance{0x144C98CB8, 0x0};
WEAK symbol<EngineDependentDvarMut> hud_healthOverlay_phaseEnd_fromAlpha{
    0x144C98D20, 0x0};
WEAK symbol<EngineDependentDvarMut> hud_healthOverlay_phaseEnd_toAlpha{
    0x144C98D10, 0x0};
WEAK symbol<EngineDependentDvarMut> hud_healthOverlay_phaseOne_pulseDuration{
    0x144C98CE8, 0x0};
WEAK symbol<EngineDependentDvarMut> hud_healthOverlay_phaseOne_toAlphaAdd{
    0x144C98CE0, 0x0};
WEAK symbol<EngineDependentDvarMut> hud_healthOverlay_phaseThree_pulseDuration{
    0x144C98D08, 0x0};
WEAK symbol<EngineDependentDvarMut>
    hud_healthOverlay_phaseThree_toAlphaMultiplier{0x144C98D00, 0x0};
WEAK symbol<EngineDependentDvarMut> hud_healthOverlay_phaseTwo_pulseDuration{
    0x144C98CF8, 0x0};
WEAK symbol<EngineDependentDvarMut>
    hud_healthOverlay_phaseTwo_toAlphaMultiplier{0x144C98CF0, 0x0};
WEAK symbol<EngineDependentDvarMut> hud_healthOverlay_pulseStart{0x144C98CD0,
                                                                 0x0};
WEAK symbol<EngineDependentDvarMut> hud_healthOverlay_pulseStop{0x144C98CD8,
                                                                0x0};
WEAK symbol<EngineDependentDvarMut> hud_healthOverlay_regenPauseTime{
    0x144C98CC8, 0x0};
WEAK symbol<EngineDependentDvarMut> hud_health_startpulse_injured{0x144C98CC0,
                                                                  0x0};
} // namespace game
