#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
// e.g. BG_ASM_
namespace bg {
WEAK symbol<bool(eModes mode, ControllerIndex_t controllerIndex,
                 int32_t itemIndex)>
    BG_UnlockablesIsItemPurchased{0x1426304B0, 0x1426A9620, 0x0};
WEAK symbol<bool(eModes mode, ControllerIndex_t controllerIndex,
                 int32_t itemIndex, int32_t attachmentNum)>
    BG_UnlockablesIsItemAttachmentLocked{0x14262F760, 0x1426A88D0, 0x0};
WEAK symbol<bool(eModes mode, ControllerIndex_t controllerIndex,
                 int32_t itemIndex, int32_t attachmentSlotIndex)>
    BG_UnlockablesIsAttachmentSlotLocked{0x14262F560, 0x1426A86D0, 0x0};
WEAK symbol<bool(eModes mode, ControllerIndex_t controllerIndex,
                 int32_t itemIndex, int32_t optionIndex)>
    BG_UnlockablesItemOptionLocked{0x142631550, 0x1426AA6C0, 0x0};
WEAK symbol<bool(eModes mode, ControllerIndex_t controllerIndex,
                 emblemChallengeLookup_t *challengeLookup, bool otherPlayer)>
    BG_UnlockablesEmblemOrBackingLockedByChallenge{0x14262A970, 0x1426A3AE0,
                                                   0x0};
WEAK symbol<bool(eModes mode, ControllerIndex_t controllerIndex, uint16_t index,
                 int32_t itemIndex)>
    BG_UnlockedGetChallengeUnlockedForIndex{0x142636480, 0x1426AF5F0, 0x0};
WEAK symbol<bool(eModes mode, ControllerIndex_t controllerIndex,
                 uint32_t characterIndex, CharacterItemType itemType,
                 int32_t itemIndex)>
    BG_UnlockablesCharacterCustomizationItemLocked{0x142628EC0, 0x1426A2030,
                                                   0x0};
WEAK symbol<int32_t(eModes mode, ControllerIndex_t controllerIndex)>
    BG_UnlockablesGetCustomClassCount{0x14262C790, 0x1426A5900, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex,
                 BGEmblemBackgroundID backgroundId)>
    BG_EmblemIsEntitlementBackgroundGranted{0x1425EE3B0, 0x142667520, 0x0};
WEAK symbol<bool(const vec3_t *pos, const vec3_t *mapCenter)> BG_ValidateOrigin{
    0x142609370, 0x1426824E0, 0x1406A9BB0};
WEAK symbol<void(const level::trajectory_t *tr, int32_t atTime, vec3_t *result)>
    BG_EvaluateTrajectory{0x142601DA0, 0x14267AF10, 0x1406A3DE0};
WEAK symbol<void(const level::trajectory_t *tr, int32_t atTime, vec3_t *result)>
    BG_EvaluateTrajectoryDelta{0x142601A70, 0x14267ABE0, 0x1406A3AB0};
WEAK symbol<db::xasset::CharacterBodyType *(
    eModes mode, const uint32_t playerRoleTemplateIndex)>
    BG_GetCharacterBodyType{0x1400ADD00, 0x400ADD00, 0x140047EB0};

WEAK symbol<qboolean(int32_t wheel0, int32_t wheel1, int32_t wheel2)>
    BG_VehicleWheelsCollinear{0x142650E20, 0x0, 0x1406D09C0};
WEAK symbol<void(move::pmove_t *pm, const move::pml_t *pml)>
    BG_VehicleStrafeMove{0x1426504E0, 0x0, 0x1406D0080};
WEAK symbol<void(move::pmove_t *pm, vehicle::VehicleState *vehicleState,
                 const cm::GroundTrace *groundTrace, bool gravity, float dt)>
    BG_VehicleStepSlideMove{0x0, 0x0, 0x1406CFBA0};
WEAK symbol<void(move::pmove_t *pm, const move::pml_t *pml)>
    BG_VehicleSteerMove{0x14264F890, 0x0, 0x1406CF430};
WEAK symbol<void(move::pmove_t *pm, vehicle::VehicleState *vehicleState,
                 const cm::GroundTrace *groundTrace, bool gravity, float dt)>
    BG_VehicleSlideMove{0x0, 0x0, 0x1406CE2E0};
WEAK symbol<void(move::pmove_t *pm, const move::pml_t *pml)>
    BG_VehiclePlaneMove{0x1425FC6E0, 0x0, 0x14069E710};
WEAK symbol<void(move::pmove_t *pm, const move::pml_t *pml)> BG_VehicleMove{
    0x14264E5F0, 0x0, 0x1406CE190};
WEAK symbol<void(move::pmove_t *pm, const move::pml_t *pml)>
    BG_VehicleHelicopterMove{0x1425FBC00, 0x0, 0x14069DC30};
WEAK symbol<void(move::pmove_t *pm, vehicle::VehicleState *vehicleState,
                 cm::GroundTrace *groundTrace, float dtt)>
    BG_VehicleGroundTrace{0x14264E440, 0x0, 0x1406CDFE0};
WEAK symbol<void(move::pmove_t *pm, vehicle::VehicleState *vehicleState,
                 const cm::GroundTrace *groundTrace, bool gravity, float dt)>
    BG_VehicleGroundPlant{0x0, 0x0, 0x1406CD680};
WEAK symbol<void(move::pmove_t *pm, vehicle::VehicleState *vehicleState,
                 const cm::GroundTrace *groundTrace, bool gravity, float dt)>
    BG_VehicleAirMove{0x0, 0x0, 0x1406CD500};

} // namespace bg
} // namespace game
