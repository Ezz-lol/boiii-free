#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> riotshield_assist_time{
    0x15A7FE050, 0x15A87CFD0, 0x14B3966B8};
WEAK symbol<EngineDependentDvarMut> riotshield_bullet_damage_scale{
    0x15A7FE008, 0x15A87CF88, 0x14B396670};
WEAK symbol<EngineDependentDvarMut> riotshield_damage_score_max{
    0x15A7FE060, 0x15A87CFE0, 0x14B3966C8};
WEAK symbol<EngineDependentDvarMut> riotshield_damage_score_threshold{
    0x15A7FE058, 0x15A87CFD8, 0x14B3966C0};
WEAK symbol<EngineDependentDvarMut> riotshield_deployed_health{
    0x15A7FE028, 0x15A87CFA8, 0x14B396690};
WEAK symbol<EngineDependentDvarMut> riotshield_deploy_limit_radius{
    0x15A7FE000, 0x15A87CF80, 0x14B396668};
WEAK symbol<EngineDependentDvarMut> riotshield_deploy_pitch_max{
    0x15A7FE038, 0x15A87CFB8, 0x14B3966A0};
WEAK symbol<EngineDependentDvarMut> riotshield_deploy_roll_max{
    0x15A7FE040, 0x15A87CFC0, 0x14B3966A8};
WEAK symbol<EngineDependentDvarMut> riotshield_deploy_zdiff_max{
    0x15A7FE048, 0x15A87CFC8, 0x14B3966B0};
WEAK symbol<EngineDependentDvarMut> riotshield_destroyed_cleanup_time{
    0x15A7FE030, 0x15A87CFB0, 0x14B396698};
WEAK symbol<EngineDependentDvarMut> riotshield_explosive_damage_scale{
    0x15A7FE010, 0x15A87CF90, 0x14B396678};
WEAK symbol<EngineDependentDvarMut> riotshield_melee_damage_scale{
    0x15A7FE018, 0x15A87CF98, 0x14B396680};
WEAK symbol<EngineDependentDvarMut> riotshield_projectile_damage_scale{
    0x15A7FE020, 0x15A87CFA0, 0x14B396688};

WEAK symbol<EngineDependentDvarMut> shieldBlastDamageProtection_120{
    0x15A7FE078, 0x15A87CFF8, 0x14B3966E0};
WEAK symbol<EngineDependentDvarMut> shieldBlastDamageProtection_180{
    0x15A7FE080, 0x15A87D000, 0x14B3966E8};
WEAK symbol<EngineDependentDvarMut> shieldBlastDamageProtection_30{
    0x15A7FE068, 0x15A87CFE8, 0x14B3966D0};
WEAK symbol<EngineDependentDvarMut> shieldBlastDamageProtection_60{
    0x15A7FE070, 0x15A87CFF0, 0x14B3966D8};
WEAK symbol<EngineDependentDvarMut> shieldDeployShakeDuration{
    0x15A7FDFF8, 0x15A87CF78, 0x14B396660};
WEAK symbol<EngineDependentDvarMut> shieldDeployShakeScale{
    0x15A7FDFF0, 0x15A87CF70, 0x14B396658};
WEAK symbol<EngineDependentDvarMut> shieldImpactBulletShakeDuration{
    0x15A7FDFB0, 0x15A87CF30, 0x14B396618};
WEAK symbol<EngineDependentDvarMut> shieldImpactBulletShakeScale{
    0x15A7FDFA8, 0x15A87CF28, 0x14B396610};
WEAK symbol<EngineDependentDvarMut> shieldImpactExplosionHighShakeDuration{
    0x15A7FDFE0, 0x15A87CF60, 0x14B396648};
WEAK symbol<EngineDependentDvarMut> shieldImpactExplosionHighShakeScale{
    0x15A7FDFD8, 0x15A87CF58, 0x14B396640};
WEAK symbol<EngineDependentDvarMut> shieldImpactExplosionLowShakeDuration{
    0x15A7FDFD0, 0x15A87CF50, 0x14B396638};
WEAK symbol<EngineDependentDvarMut> shieldImpactExplosionLowShakeScale{
    0x15A7FDFC8, 0x15A87CF48, 0x14B396630};
WEAK symbol<EngineDependentDvarMut> shieldImpactExplosionThreshold{
    0x15A7FDFE8, 0x15A87CF68, 0x14B396650};
WEAK symbol<EngineDependentDvarMut> shieldImpactMissileShakeDuration{
    0x15A7FDFC0, 0x15A87CF40, 0x14B396628};
WEAK symbol<EngineDependentDvarMut> shieldImpactMissileShakeScale{
    0x15A7FDFB8, 0x15A87CF38, 0x14B396620};
WEAK symbol<EngineDependentDvarMut> shieldPlayerBulletProtectionDegrees{
    0x15A7FE088, 0x15A87D008, 0x14B3966F0};
} // namespace game
