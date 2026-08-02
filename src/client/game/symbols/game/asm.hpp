#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace anim {
using namespace game::ai;
WEAK symbol<bool(const AIBody *aiBody, int32_t entityNum,
                 LocalClientNum_t localClientNum)>
    G_ASM_AimLayerShouldBeActive{0x0, 0x14023F5B0};
WEAK symbol<void(int32_t entityNum)> G_ASM_AnimUpdated{0x0, 0x14023F6C0};
WEAK symbol<void(int32_t entityNum, LocalClientNum_t localClientNum,
                 vec3_t *headingVec)>
    G_ASM_GetAIHeadingVec{0x0, 0x14023FA30};
WEAK symbol<void(int32_t entityNum, LocalClientNum_t localClientNum,
                 vec3_t *moveVec)>
    G_ASM_GetAIMoveVec{0x0, 0x14023FA70};
WEAK symbol<void(int32_t entityNum, LocalClientNum_t localClientNum,
                 float *idleBlendRatio, const float idleBlendDropoff)>
    G_ASM_GetAIVelocityRatio{0x0, 0x14023FB10};
WEAK symbol<bool(AIBody *aiBody, int32_t entityNum,
                 LocalClientNum_t localClientNum)>
    G_ASM_GetAimAnimations{0x0, 0x14023FBC0};
WEAK symbol<int32_t(int32_t entityNum, LocalClientNum_t localClientNum)>
    G_ASM_GetEnemyEntNum{0x0, 0x14023FC90};
WEAK symbol<bool(AIBody *aiBody, int32_t entityNum,
                 LocalClientNum_t localClientNum)>
    G_ASM_GetParametricAnimationGroup{0x0, 0x14023FCE0};
WEAK symbol<void(int32_t entityNum, LocalClientNum_t localClientNum,
                 vec3_t *viewOrigin)>
    G_ASM_GetPlayerViewOrigin{0x0, 0x14023FDC0};
WEAK symbol<bool(AIBody *aiBody, int32_t entityNum,
                 LocalClientNum_t localClientNum)>
    G_ASM_GetPrimaryDeltaAnimation{0x0, 0x14023FDE0};
WEAK symbol<bool(AIBody *aiBody, int32_t entityNum,
                 LocalClientNum_t localClientNum, scr::ScrString_t tableName,
                 scr::ScrString_t astEvalFunction, int32_t *outTableIndex,
                 int32_t *outRowNum, int32_t *outAnimIndex,
                 const bool dumpDebugInfo)>
    G_ASM_GetSingleAnimation{0x0, 0x14023FF20};
WEAK symbol<void(int32_t attackerEntNum, LocalClientNum_t localClientNum,
                 int32_t shootAtEntNum, vec3_t *targetPos)>
    G_ASM_GetTargetPos{0x0, 0x140240000};
WEAK symbol<void(AIBody *aiBody, int32_t entityNum)> G_ASM_PackState{
    0x0, 0x140240290};
WEAK symbol<asmStatus_t(AIBody *aiBody, int32_t entityNum)> G_ASM_PostUpdate{
    0x0, 0x140240420};
WEAK symbol<asmStatus_t(AIBody *aiBody, int32_t entityNum)> G_ASM_PreUpdate{
    0x0, 0x140240570};
WEAK symbol<void()> G_ASM_InitMocomps{0x0, 0x1402445D0};
WEAK symbol<int(AIBody *aiBody, int32_t entityNum,
                LocalClientNum_t localClientNum)>
    G_ASM_GetBurstCount{0x0, 0x140245090};
WEAK symbol<int(AIBody *aiBody, int32_t entityNum,
                LocalClientNum_t localClientNum)>
    G_ASM_GetFireInterval{0x0, 0x1402450E0};
WEAK
    symbol<AIWeaponFireType(int32_t entityNum, LocalClientNum_t localClientNum)>
        G_ASM_GetFireMode{0x0, 0x140245130};
WEAK symbol<bool(AIBody *aiBody, int32_t entityNum,
                 LocalClientNum_t localClientNum)>
    G_ASM_GetShootAnimations{0x0, 0x140245150};
WEAK symbol<void(int32_t entityNum, bool shouldSet)> G_ASM_SetFiringFlag{
    0x0, 0x140245240};
WEAK symbol<bool(const AIBody *const aiBody, int32_t entityNum,
                 LocalClientNum_t localClientNum)>
    G_ASM_ShootLayerShouldBeActive{0x0, 0x1402452D0};
} // namespace anim
} // namespace game