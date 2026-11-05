#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut>
    lobbymigrate_considerstreamingforhostmigration{0x1556CE830, 0x15574D7A0,
                                                   0x1485812D0};
WEAK symbol<EngineDependentDvarMut> lobbymigrate_dedicatedonly{
    0x1556CE828, 0x15574D798, 0x1485812C8};
WEAK symbol<EngineDependentDvarMut> lobbymigrate_enabled{
    0x1556CE810, 0x15574D780, 0x1485812B0};
WEAK symbol<EngineDependentDvarMut> lobbymigrate_enabledlan{
    0x1556CE818, 0x15574D788, 0x1485812B8};
WEAK symbol<EngineDependentDvarMut> lobbymigrate_hostwaitms{
    0x1556CE7D8, 0x15574D748, 0x148581278};
WEAK symbol<EngineDependentDvarMut> lobbymigrate_maxpacketwaittime{
    0x1556CE7C8, 0x15574D738, 0x148581268};
WEAK symbol<EngineDependentDvarMut> lobbymigrate_migratetobestgamehostenabled{
    0x1556CE820, 0x15574D790, 0x1485812C0};
WEAK symbol<EngineDependentDvarMut> lobbymigrate_minscorediff{
    0x1556CE7F8, 0x15574D768, 0x148581298};
WEAK symbol<EngineDependentDvarMut> lobbymigrate_nomineewaitms{
    0x1556CE7D0, 0x15574D740, 0x148581270};
WEAK symbol<EngineDependentDvarMut> lobbymigrate_usestddev{
    0x1556CE800, 0x15574D770, 0x1485812A0};
WEAK symbol<EngineDependentDvarMut> lobbymigrate_wirelesslatencyincrease{
    0x1556CE808, 0x15574D778, 0x1485812A8};
} // namespace game
