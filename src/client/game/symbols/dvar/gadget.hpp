#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> gadgetActivateDuringWeaponChange{
    0x1441D5DF0, 0x144254DF0, 0x141E56DB0};
WEAK symbol<EngineDependentDvarMut> gadgetEnabled{0x1441D5DA0, 0x144254DA0,
                                                  0x141E56D60};
WEAK symbol<EngineDependentDvarMut> gadgetFlickerDurationDefault{
    0x1441D5DA8, 0x144254DA8, 0x141E56D68};
WEAK symbol<EngineDependentDvarMut> gadgetFlickerWhizbyMaxDistance{
    0x1441D5DB0, 0x144254DB0, 0x141E56D70};
WEAK symbol<EngineDependentDvarMut> gadgetPowerInitEmpty{
    0x1441D5DC8, 0x144254DC8, 0x141E56D88};
WEAK symbol<EngineDependentDvarMut> gadgetPowerOverchargePerkScoreFactor{
    0x1441D5DE8, 0x144254DE8, 0x141E56DA8};
WEAK symbol<EngineDependentDvarMut> gadgetPowerOverchargePerkTimeFactor{
    0x1441D5DE0, 0x144254DE0, 0x141E56DA0};
WEAK symbol<EngineDependentDvarMut> gadgetPowerOverrideFactor{
    0x1441D5DB8, 0x144254DB8, 0x141E56D78};
WEAK symbol<EngineDependentDvarMut> gadgetPowerUnlimited{
    0x1441D5DC0, 0x144254DC0, 0x141E56D80};
WEAK symbol<EngineDependentDvarMut> gadgetPulseOrbShareEnemy{
    0x1441D5DD0, 0x144254DD0, 0x141E56D90};
WEAK symbol<EngineDependentDvarMut> gadgetPulseOrbShareFriendly{
    0x1441D5DD8, 0x144254DD8, 0x141E56D98};
WEAK symbol<EngineDependentDvarMut> gadgetThiefKillFullPowerMultiplier{
    0x0, 0x144254E20, 0x141E56DE0};
WEAK symbol<EngineDependentDvarMut> gadgetThiefKillPowerGain{
    0x1441D5E18, 0x144254E18, 0x141E56DD8};
WEAK symbol<EngineDependentDvarMut> gadgetThiefRechargeRate{
    0x1441D5E08, 0x144254E08, 0x141E56DC8};
WEAK symbol<EngineDependentDvarMut> gadgetThiefShutdownFullCharge{
    0x1441D5E00, 0x144254E00, 0x141E56DC0};
} // namespace game
