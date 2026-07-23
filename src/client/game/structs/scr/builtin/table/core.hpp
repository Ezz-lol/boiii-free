#pragma once

#include "game/structs/scr/builtin/core.hpp"
#include "game/structs/scr/builtin/table/macros.hpp"

namespace game {
namespace scr {
namespace builtin {
namespace table {

union ActorInterfaceMethodTable {
  struct {
    const BuiltinMethodDef ChooseBetterCoverNode;
    const BuiltinMethodDef GetFireMode;
    const BuiltinMethodDef BB_GetArrivalStance;
    const BuiltinMethodDef BB_GetCurrentExposedType;
    const BuiltinMethodDef BB_GetLocomotionArrivalDistance;
    const BuiltinMethodDef BB_GetLocomotionArrivalYaw;
    const BuiltinMethodDef BB_GetLocomotionMotionAngle;
    const BuiltinMethodDef BB_GetLocomotionTurnYaw;
  };
  const BuiltinMethodDef methods[8];

  static constexpr const char *names[8] = {
      "ChooseBetterCoverNode",
      "GetFireMode",
      "BB_GetArrivalStance",
      "BB_GetCurrentExposedType",
      "BB_GetLocomotionArrivalDistance",
      "BB_GetLocomotionArrivalYaw",
      "BB_GetLocomotionMotionAngle",
      "BB_GetLocomotionTurnYaw",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(ActorInterfaceMethodTable,
            sizeof(ActorInterfaceMethodTable::methods));

union ActorMethodTable {
  struct {
    const BuiltinMethodDef StartCoverArrival;
    const BuiltinMethodDef Melee;
    const BuiltinMethodDef ReacquireStep;
    const BuiltinMethodDef FindReacquireNode;
    const BuiltinMethodDef GetReacquireNode;
    const BuiltinMethodDef UseReacquireNode;
    const BuiltinMethodDef FindReacquireDirectPath;
    const BuiltinMethodDef TrimPathToAttack;
    const BuiltinMethodDef ReacquireMove;
    const BuiltinMethodDef FindReacquireProximatePath;
    const BuiltinMethodDef FlagEnemyUnattackable;
    const BuiltinMethodDef ClearPitchOrient;
    const BuiltinMethodDef SetPitchOrient;
    const BuiltinMethodDef CanSee;
    const BuiltinMethodDef SeeRecently;
    const BuiltinMethodDef LastKnownTime;
    const BuiltinMethodDef LastKnownPos;
    const BuiltinMethodDef MayMoveToPoint;
    const BuiltinMethodDef MayMoveFromPointToPoint;
    const BuiltinMethodDef Teleport;
    const BuiltinMethodDef IsPathDirect;
    const BuiltinMethodDef IsStanceAllowed;
    const BuiltinMethodDef TraverseMode;
    const BuiltinMethodDef AnimMode;
    const BuiltinMethodDef OrientMode;
    const BuiltinMethodDef GetOrientMode;
    const BuiltinMethodDef ShouldFaceMotion;
    const BuiltinMethodDef GetAnglesToLikelyEnemyPath;
    const BuiltinMethodDef LerpPosition;
    const BuiltinMethodDef GetHitEntType;
    const BuiltinMethodDef GetHitYaw;
    const BuiltinMethodDef GetGroundEntType;
    const BuiltinMethodDef IsDeflected;
    const BuiltinMethodDef AnimCustom;
    const BuiltinMethodDef CanAttackEnemyNode;
    const BuiltinMethodDef GetPathLength;
    const BuiltinMethodDef CalcPathLength;
    const BuiltinMethodDef CheckProne;
    const BuiltinMethodDef PushPlayer;
    const BuiltinMethodDef PushActors;
    const BuiltinMethodDef NearNode;
    const BuiltinMethodDef NearClaimNode;
    const BuiltinMethodDef NearClaimNodeAndAngle;
    const BuiltinMethodDef AtDangerousNode;
    const BuiltinMethodDef GetEnemyInfo;
    const BuiltinMethodDef ClearEnemy;
    const BuiltinMethodDef SetPotentialThreat;
    const BuiltinMethodDef ClearPotentialThreat;
    const BuiltinMethodDef SetFlashbangImmunity;
    const BuiltinMethodDef SetFlashBanged;
    const BuiltinMethodDef GetFlashBangedStrength;
    const BuiltinMethodDef IsKnownEnemyInRadius;
    const BuiltinMethodDef IsKnownEnemyInVolume;
    const BuiltinMethodDef SetTalkToSpecies;
    const BuiltinMethodDef AllowPitchAngle;
    const BuiltinMethodDef KnockBack;
    const BuiltinMethodDef GetDeltaTurnYaw;
    const BuiltinMethodDef SetRepairPaths;
    const BuiltinMethodDef FinishActorDamage;
    const BuiltinMethodDef CalcLookaheadPos;
    const BuiltinMethodDef IsInScriptedState;
    const BuiltinMethodDef IsActorShooting;
    const BuiltinMethodDef MeleeWithOffset;
    const BuiltinMethodDef DropWeapon;
    const BuiltinMethodDef Shoot;
    const BuiltinMethodDef SetFixedNodeSafeVolume;
    const BuiltinMethodDef GetFixedNodeSafeVolume;
    const BuiltinMethodDef ClearFixedNodeSafeVolume;
    const BuiltinMethodDef UsePosition;
    const BuiltinMethodDef ClearUsePosition;
    const BuiltinMethodDef GetNodeOffsetPosition;
    const BuiltinMethodDef GetNodeOffsetAngles;
    const BuiltinMethodDef UseCoverNode;
    const BuiltinMethodDef FindBestCoverNode;
    const BuiltinMethodDef FindBestCoverNodes;
    const BuiltinMethodDef FindCoverNodeAtLocation;
    const BuiltinMethodDef GetDropToFloorPosition;
    const BuiltinMethodDef IsSuppressionWaiting;
    const BuiltinMethodDef IsSuppressed;
    const BuiltinMethodDef IsMoveSuppressed;
    const BuiltinMethodDef ForceTeleport;
    const BuiltinMethodDef SetEntityOwner;
    const BuiltinMethodDef ClearEntityOwner;
    const BuiltinMethodDef AIthrowgrenade;
    const BuiltinMethodDef GrenadeThrow;
    const BuiltinMethodDef IsGrenadePosSafe;
    const BuiltinMethodDef CheckGrenadeThrow;
    const BuiltinMethodDef IsSafeFromGrenade;
    const BuiltinMethodDef CanShoot;
    const BuiltinMethodDef CanShootEnemy;
    const BuiltinMethodDef CheckGrenadeThrowPos;
    const BuiltinMethodDef CanThrowGrenade;
    const BuiltinMethodDef CanThrowGrenadePos;
    const BuiltinMethodDef PickUpGrenade;
    const BuiltinMethodDef HasValidInterrupt;
    const BuiltinMethodDef GetBehaviorTreeStatus;
    const BuiltinMethodDef IsCurrentBTActionLooping;
    const BuiltinMethodDef ShouldHoldGroundAgainstEnemy;
    const BuiltinMethodDef GetEnemyVelocity;
    const BuiltinMethodDef IsAtGoal;
    const BuiltinMethodDef IsPosAtGoal;
    const BuiltinMethodDef IsApproachingGoal;
    const BuiltinMethodDef ShouldUseCoverNode;
    const BuiltinMethodDef IsAtCoverNode;
    const BuiltinMethodDef IsAtCoverNodeStrict;
    const BuiltinMethodDef IsFlankedAtCoverNode;
    const BuiltinMethodDef ShouldStartArrival;
    const BuiltinMethodDef ShouldStartTraversal;
    const BuiltinMethodDef FinishTraversal;
    const BuiltinMethodDef SetStairsExitTransform;
    const BuiltinMethodDef SetAnimState;
    const BuiltinMethodDef SetAnimStateFromASD;
    const BuiltinMethodDef HasAnimStateFromASD;
    const BuiltinMethodDef GetAnimStateFromASD;
    const BuiltinMethodDef GetAnimSubStateFromASD;
    const BuiltinMethodDef GetAnimSubStateCountFromASD;
    const BuiltinMethodDef GetAnimFromASD;
    const BuiltinMethodDef GetAnimLengthFromASD;
    const BuiltinMethodDef GetAnimHasNotetrackFromASD;
    const BuiltinMethodDef SetPhysParams;
    const BuiltinMethodDef SetSteeringMode;
    const BuiltinMethodDef PredictPath;
    const BuiltinMethodDef PredictArrival;
    const BuiltinMethodDef PredictExit;
    const BuiltinMethodDef GetTurnExitYaw;
    const BuiltinMethodDef GetFinalPathPos;
    const BuiltinMethodDef UpdateTrackedBlackboardAttribute;
    const BuiltinMethodDef TrackBlackBoardAttribute;
    const BuiltinMethodDef FinalizeTrackedBlackBoardAttributes;
    const BuiltinMethodDef HasPath;
    const BuiltinMethodDef ClearPath;
    const BuiltinMethodDef GenerateRadioEvent;
    const BuiltinMethodDef GetAimLimitsFromEntry;
    const BuiltinMethodDef SetFreeCameraLockOnAllowed;
    const BuiltinMethodDef PathMode;
    const BuiltinMethodDef GetPathMode;
    const BuiltinMethodDef DamageMode;
    const BuiltinMethodDef IsCoverValid;
    const BuiltinMethodDef IsInAnyBadPlace;
    const BuiltinMethodDef CalcApproximatePathToPosition;
    const BuiltinMethodDef GetNextFindBestCoverTime;
  };
  const BuiltinMethodDef methods[141];

  static constexpr const char *names[141] = {
      "StartCoverArrival",
      "Melee",
      "ReacquireStep",
      "FindReacquireNode",
      "GetReacquireNode",
      "UseReacquireNode",
      "FindReacquireDirectPath",
      "TrimPathToAttack",
      "ReacquireMove",
      "FindReacquireProximatePath",
      "FlagEnemyUnattackable",
      "ClearPitchOrient",
      "SetPitchOrient",
      "CanSee",
      "SeeRecently",
      "LastKnownTime",
      "LastKnownPos",
      "MayMoveToPoint",
      "MayMoveFromPointToPoint",
      "Teleport",
      "IsPathDirect",
      "IsStanceAllowed",
      "TraverseMode",
      "AnimMode",
      "OrientMode",
      "GetOrientMode",
      "ShouldFaceMotion",
      "GetAnglesToLikelyEnemyPath",
      "LerpPosition",
      "GetHitEntType",
      "GetHitYaw",
      "GetGroundEntType",
      "IsDeflected",
      "AnimCustom",
      "CanAttackEnemyNode",
      "GetPathLength",
      "CalcPathLength",
      "CheckProne",
      "PushPlayer",
      "PushActors",
      "NearNode",
      "NearClaimNode",
      "NearClaimNodeAndAngle",
      "AtDangerousNode",
      "GetEnemyInfo",
      "ClearEnemy",
      "SetPotentialThreat",
      "ClearPotentialThreat",
      "SetFlashbangImmunity",
      "SetFlashBanged",
      "GetFlashBangedStrength",
      "IsKnownEnemyInRadius",
      "IsKnownEnemyInVolume",
      "SetTalkToSpecies",
      "AllowPitchAngle",
      "KnockBack",
      "GetDeltaTurnYaw",
      "SetRepairPaths",
      "FinishActorDamage",
      "CalcLookaheadPos",
      "IsInScriptedState",
      "IsActorShooting",
      "MeleeWithOffset",
      "DropWeapon",
      "Shoot",
      "SetFixedNodeSafeVolume",
      "GetFixedNodeSafeVolume",
      "ClearFixedNodeSafeVolume",
      "UsePosition",
      "ClearUsePosition",
      "GetNodeOffsetPosition",
      "GetNodeOffsetAngles",
      "UseCoverNode",
      "FindBestCoverNode",
      "FindBestCoverNodes",
      "FindCoverNodeAtLocation",
      "GetDropToFloorPosition",
      "IsSuppressionWaiting",
      "IsSuppressed",
      "IsMoveSuppressed",
      "ForceTeleport",
      "SetEntityOwner",
      "ClearEntityOwner",
      "AIthrowgrenade",
      "GrenadeThrow",
      "IsGrenadePosSafe",
      "CheckGrenadeThrow",
      "IsSafeFromGrenade",
      "CanShoot",
      "CanShootEnemy",
      "CheckGrenadeThrowPos",
      "CanThrowGrenade",
      "CanThrowGrenadePos",
      "PickUpGrenade",
      "HasValidInterrupt",
      "GetBehaviorTreeStatus",
      "IsCurrentBTActionLooping",
      "ShouldHoldGroundAgainstEnemy",
      "GetEnemyVelocity",
      "IsAtGoal",
      "IsPosAtGoal",
      "IsApproachingGoal",
      "ShouldUseCoverNode",
      "IsAtCoverNode",
      "IsAtCoverNodeStrict",
      "IsFlankedAtCoverNode",
      "ShouldStartArrival",
      "ShouldStartTraversal",
      "FinishTraversal",
      "SetStairsExitTransform",
      "SetAnimState",
      "SetAnimStateFromASD",
      "HasAnimStateFromASD",
      "GetAnimStateFromASD",
      "GetAnimSubStateFromASD",
      "GetAnimSubStateCountFromASD",
      "GetAnimFromASD",
      "GetAnimLengthFromASD",
      "GetAnimHasNotetrackFromASD",
      "SetPhysParams",
      "SetSteeringMode",
      "PredictPath",
      "PredictArrival",
      "PredictExit",
      "GetTurnExitYaw",
      "GetFinalPathPos",
      "UpdateTrackedBlackboardAttribute",
      "TrackBlackBoardAttribute",
      "FinalizeTrackedBlackBoardAttributes",
      "HasPath",
      "ClearPath",
      "GenerateRadioEvent",
      "GetAimLimitsFromEntry",
      "SetFreeCameraLockOnAllowed",
      "PathMode",
      "GetPathMode",
      "DamageMode",
      "IsCoverValid",
      "IsInAnyBadPlace",
      "CalcApproximatePathToPosition",
      "GetNextFindBestCoverTime",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(ActorMethodTable, sizeof(ActorMethodTable::methods));

union BotMethodTable {
  struct {
    const BuiltinMethodDef BotDropClient;
    const BuiltinMethodDef BotInLastStand;
    const BuiltinMethodDef BotInLowReady;
    const BuiltinMethodDef BotGetJetEnergy;
    const BuiltinMethodDef BotTakeManualControl;
    const BuiltinMethodDef BotReleaseManualControl;
    const BuiltinMethodDef BotUnderManualControl;
    const BuiltinMethodDef BotSetGoal;
    const BuiltinMethodDef BotGetGoalPosition;
    const BuiltinMethodDef BotGetGoalRadius;
    const BuiltinMethodDef BotGoalSet;
    const BuiltinMethodDef BotGoalReached;
    const BuiltinMethodDef BotRequestPath;
    const BuiltinMethodDef BotCancelPath;
    const BuiltinMethodDef BotLookNone;
    const BuiltinMethodDef BotLookForward;
    const BuiltinMethodDef BotLookAtPoint;
    const BuiltinMethodDef BotSetLookAngles;
    const BuiltinMethodDef BotSetLookAnglesFromPoint;
    const BuiltinMethodDef BotGetFov;
    const BuiltinMethodDef BotSetMoveMagnitude;
    const BuiltinMethodDef BotSetMoveAngle;
    const BuiltinMethodDef BotSetMoveAngleFromPoint;
    const BuiltinMethodDef BotTapButton;
    const BuiltinMethodDef BotPressButton;
    const BuiltinMethodDef BotReleaseButton;
    const BuiltinMethodDef BotReleaseButtons;
    const BuiltinMethodDef BotPressButtonForGadget;
    const BuiltinMethodDef BotSwitchToWeapon;
    const BuiltinMethodDef BotGetThreats;
    const BuiltinMethodDef BotSightTrace;
    const BuiltinMethodDef BotSetDefaultClass;
    const BuiltinMethodDef BotClassAddItem;
    const BuiltinMethodDef BotClassSetWeaponOption;
    const BuiltinMethodDef BotClassAddAttachment;
    const BuiltinMethodDef BotToggleGodMode;
    const BuiltinMethodDef BotToggleDemigodMode;
    const BuiltinMethodDef PredictPosition;
    const BuiltinMethodDef LookAt;
    const BuiltinMethodDef LookDirection;
    const BuiltinMethodDef LookTowards;
    const BuiltinMethodDef ClearLookAt;
    const BuiltinMethodDef StopLook;
    const BuiltinMethodDef MoveTowards;
    const BuiltinMethodDef MoveDirection;
    const BuiltinMethodDef BotLeaveGame;
    const BuiltinMethodDef GetThreats;
    const BuiltinMethodDef BotSightTracePassed;
    const BuiltinMethodDef PressDTPButton;
    const BuiltinMethodDef CancelGoal;
    const BuiltinMethodDef BotSetMeleeChargeEnt;
    const BuiltinMethodDef BotClearMeleeChargeEnt;
    const BuiltinMethodDef BotSetFailSafeNode;
    const BuiltinMethodDef ThrowGrenade;
    const BuiltinMethodDef AddGoal;
    const BuiltinMethodDef AtGoal;
    const BuiltinMethodDef HasGoal;
    const BuiltinMethodDef GetGoal;
    const BuiltinMethodDef PressUseButton;
    const BuiltinMethodDef PressAttackButton;
    const BuiltinMethodDef PressJumpButton;
    const BuiltinMethodDef PressDoubleJumpButton;
    const BuiltinMethodDef ReleaseDoubleJumpButton;
    const BuiltinMethodDef PressSlideButton;
    const BuiltinMethodDef GetLookAheadDist;
    const BuiltinMethodDef GetLookAheadDir;
    const BuiltinMethodDef AllowAttack;
    const BuiltinMethodDef PressAds;
    const BuiltinMethodDef PressMelee;
  };
  const BuiltinMethodDef methods[69];

  static constexpr const char *names[69] = {
      "BotDropClient",
      "BotInLastStand",
      "BotInLowReady",
      "BotGetJetEnergy",
      "BotTakeManualControl",
      "BotReleaseManualControl",
      "BotUnderManualControl",
      "BotSetGoal",
      "BotGetGoalPosition",
      "BotGetGoalRadius",
      "BotGoalSet",
      "BotGoalReached",
      "BotRequestPath",
      "BotCancelPath",
      "BotLookNone",
      "BotLookForward",
      "BotLookAtPoint",
      "BotSetLookAngles",
      "BotSetLookAnglesFromPoint",
      "BotGetFov",
      "BotSetMoveMagnitude",
      "BotSetMoveAngle",
      "BotSetMoveAngleFromPoint",
      "BotTapButton",
      "BotPressButton",
      "BotReleaseButton",
      "BotReleaseButtons",
      "BotPressButtonForGadget",
      "BotSwitchToWeapon",
      "BotGetThreats",
      "BotSightTrace",
      "BotSetDefaultClass",
      "BotClassAddItem",
      "BotClassSetWeaponOption",
      "BotClassAddAttachment",
      "BotToggleGodMode",
      "BotToggleDemigodMode",
      "PredictPosition",
      "LookAt",
      "LookDirection",
      "LookTowards",
      "ClearLookAt",
      "StopLook",
      "MoveTowards",
      "MoveDirection",
      "BotLeaveGame",
      "GetThreats",
      "BotSightTracePassed",
      "PressDTPButton",
      "CancelGoal",
      "BotSetMeleeChargeEnt",
      "BotClearMeleeChargeEnt",
      "BotSetFailSafeNode",
      "ThrowGrenade",
      "AddGoal",
      "AtGoal",
      "HasGoal",
      "GetGoal",
      "PressUseButton",
      "PressAttackButton",
      "PressJumpButton",
      "PressDoubleJumpButton",
      "ReleaseDoubleJumpButton",
      "PressSlideButton",
      "GetLookAheadDist",
      "GetLookAheadDir",
      "AllowAttack",
      "PressAds",
      "PressMelee",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(BotMethodTable, sizeof(BotMethodTable::methods));

union PlayerMethodTable {
  struct {
    const BuiltinMethodDef GiveWeapon;
    const BuiltinMethodDef HasMaxPrimaryWeapons;
    const BuiltinMethodDef UpdateWeaponOptions;
    const BuiltinMethodDef SetBlockWeaponPickup;
    const BuiltinMethodDef GetInventoryHeldWeapon;
    const BuiltinMethodDef SetInventoryHeldWeapon;
    const BuiltinMethodDef GetWeaponAcvi;
    const BuiltinMethodDef CalcWeaponOptions;
    const BuiltinMethodDef CalcPlayerOptions;
    const BuiltinMethodDef SetPlayerRenderOptions;
    const BuiltinMethodDef NextPlayerRenderOption;
    const BuiltinMethodDef BlackboxClassChoice;
    const BuiltinMethodDef SetHighlighted;
    const BuiltinMethodDef SortHeldWeapons;
    const BuiltinMethodDef TakeWeapon;
    const BuiltinMethodDef IsThrowingGrenade;
    const BuiltinMethodDef ForceGrenadeThrow;
    const BuiltinMethodDef IsFiring;
    const BuiltinMethodDef IsMeleeing;
    const BuiltinMethodDef IsGrappling;
    const BuiltinMethodDef IsSwitchingWeapons;
    const BuiltinMethodDef IsDroppingWeapon;
    const BuiltinMethodDef IsReloading;
    const BuiltinMethodDef TakeAllWeapons;
    const BuiltinMethodDef IsInMoveMode;
    const BuiltinMethodDef GetCurrentWeapon;
    const BuiltinMethodDef GetCurrentWeaponSpinLerp;
    const BuiltinMethodDef SetCurrentWeaponSpinLerp;
    const BuiltinMethodDef GetCurrentWeaponAltWeapon;
    const BuiltinMethodDef IsWeaponOverheating;
    const BuiltinMethodDef SetWeaponOverheating;
    const BuiltinMethodDef GetCurrentOffHand;
    const BuiltinMethodDef IsUsingOffHand;
    const BuiltinMethodDef ForceOffHandEnd;
    const BuiltinMethodDef IsChargeShotPending;
    const BuiltinMethodDef HasWeapon;
    const BuiltinMethodDef GetWeaponForWeaponRoot;
    const BuiltinMethodDef SwitchToWeapon;
    const BuiltinMethodDef SwitchToWeaponImmediate;
    const BuiltinMethodDef SwitchToOffHand;
    const BuiltinMethodDef PlayBombPlant;
    const BuiltinMethodDef PlayBombDefuse;
    const BuiltinMethodDef WeaponPlayEjectBrass;
    const BuiltinMethodDef GetLockOnRadius;
    const BuiltinMethodDef GetLockOnLossRadius;
    const BuiltinMethodDef GetLockOnSpeed;
    const BuiltinMethodDef GiveStartAmmo;
    const BuiltinMethodDef GiveMaxAmmo;
    const BuiltinMethodDef GetFractionStartAmmo;
    const BuiltinMethodDef GetFractionMaxAmmo;
    const BuiltinMethodDef SetWeaponHeatPercent;
    const BuiltinMethodDef SetWeaponHackPercent;
    const BuiltinMethodDef SetWeaponLockOnPercent;
    const BuiltinMethodDef SetWeaponFriendlyTargeting;
    const BuiltinMethodDef SetWeaponFriendlyTargetLocked;
    const BuiltinMethodDef SetWeaponFriendlyHacking;
    const BuiltinMethodDef SetTargetedEntityEndTime;
    const BuiltinMethodDef SetTargetedMissilesRemaining;
    const BuiltinMethodDef SetOrigin;
    const BuiltinMethodDef SetVelocity;
    const BuiltinMethodDef SetPlayerAngles;
    const BuiltinMethodDef GetPlayerAngles;
    const BuiltinMethodDef GetGunAngles;
    const BuiltinMethodDef GetPlayerCameraPos;
    const BuiltinMethodDef CameraActivate;
    const BuiltinMethodDef CameraSetPosition;
    const BuiltinMethodDef CameraSetLookAt;
    const BuiltinMethodDef CameraSetAngles;
    const BuiltinMethodDef StartCameraTween;
    const BuiltinMethodDef GetVRCamPos;
    const BuiltinMethodDef GetVRCamAngles;
    const BuiltinMethodDef IsVR;
    const BuiltinMethodDef GamepadUsedLast;
    const BuiltinMethodDef ButtonBitState;
    const BuiltinMethodDef PrimaryButtonPressedLocal;
    const BuiltinMethodDef SecondaryButtonPressedLocal;
    const BuiltinMethodDef UseButtonPressed;
    const BuiltinMethodDef ChangeSeatButtonPressed;
    const BuiltinMethodDef BoostButtonPressed;
    const BuiltinMethodDef AttackButtonPressed;
    const BuiltinMethodDef ActionSlotOneButtonPressed;
    const BuiltinMethodDef ActionSlotTwoButtonPressed;
    const BuiltinMethodDef ActionSlotThreeButtonPressed;
    const BuiltinMethodDef ActionSlotFourButtonPressed;
    const BuiltinMethodDef AdsButtonPressed;
    const BuiltinMethodDef ThrowButtonPressed;
    const BuiltinMethodDef MeleeButtonPressed;
    const BuiltinMethodDef JumpButtonPressed;
    const BuiltinMethodDef ActionButtonPressed;
    const BuiltinMethodDef StanceButtonPressed;
    const BuiltinMethodDef FragButtonPressed;
    const BuiltinMethodDef ReloadButtonPressed;
    const BuiltinMethodDef SecondaryOffhandButtonPressed;
    const BuiltinMethodDef InventoryButtonPressed;
    const BuiltinMethodDef SprintButtonPressed;
    const BuiltinMethodDef OffhandSpecialButtonPressed;
    const BuiltinMethodDef WeaponSwitchButtonPressed;
    const BuiltinMethodDef VehicleMoveUpButtonPressed;
    const BuiltinMethodDef SprintUpRequired;
    const BuiltinMethodDef PlayerADS;
    const BuiltinMethodDef PingPlayer;
    const BuiltinMethodDef SetPlayerCharacter;
    const BuiltinMethodDef ResetCharacterCustomization;
    const BuiltinMethodDef GetCharacterBodyModel;
    const BuiltinMethodDef GetCharacterHelmetModel;
    const BuiltinMethodDef GetCharacterHeadModel;
    const BuiltinMethodDef GetCharacterBodyRenderOptions;
    const BuiltinMethodDef SetCharacterBodyType;
    const BuiltinMethodDef GetCharacterBodyType;
    const BuiltinMethodDef SetCharacterBodyStyle;
    const BuiltinMethodDef SetCharacterHelmetStyle;
    const BuiltinMethodDef GetPlayerGenderType;
    const BuiltinMethodDef GetPlayerShowcaseWeapon;
    const BuiltinMethodDef GetPlayerSelectedTaunt;
    const BuiltinMethodDef GetPlayerSelectedGesture;
    const BuiltinMethodDef GetPlayerSelectedTauntName;
    const BuiltinMethodDef GetPlayerSelectedGestureName;
    const BuiltinMethodDef ShowViewModel;
    const BuiltinMethodDef HideViewModel;
    const BuiltinMethodDef SetOffhandSecondaryClass;
    const BuiltinMethodDef GetOffHandSecondaryClass;
    const BuiltinMethodDef SetOffHandPrimaryClass;
    const BuiltinMethodDef BeginLocationSelection;
    const BuiltinMethodDef BeginLocationAirstrikeSelection;
    const BuiltinMethodDef BeginLocationMortarSelection;
    const BuiltinMethodDef BeginLocationArtillerySelection;
    const BuiltinMethodDef BeginLocationComlinkSelection;
    const BuiltinMethodDef BeginLocationNapalmSelection;
    const BuiltinMethodDef EndLocationSelection;
    const BuiltinMethodDef ClientSysSetState;
    const BuiltinMethodDef WeaponLockDetect;
    const BuiltinMethodDef WeaponLockStart;
    const BuiltinMethodDef WeaponLockFinalize;
    const BuiltinMethodDef WeaponLockFree;
    const BuiltinMethodDef WeaponLockTargetTooClose;
    const BuiltinMethodDef WeaponLockNoClearance;
    const BuiltinMethodDef WeaponLockCount;
    const BuiltinMethodDef WeaponLockRemoveSlot;
    const BuiltinMethodDef GetMpDialogName;
    const BuiltinMethodDef GetHeroWeaponName;
    const BuiltinMethodDef GetHeroAbilityName;
    const BuiltinMethodDef GetFriendlyDogTagModel;
    const BuiltinMethodDef GetEnemyDogTagModel;
    const BuiltinMethodDef GetTargetScreenAngles;
    const BuiltinMethodDef GetReplayGunLockOnOrigin;
    const BuiltinMethodDef GetLockOnOrigin;
    const BuiltinMethodDef GetNormalizedMovement;
    const BuiltinMethodDef ButtonPressed;
    const BuiltinMethodDef GiveWeaponNextAttachment;
    const BuiltinMethodDef GiveNextBaseWeapon;
    const BuiltinMethodDef GetNormalizedCameraMovement;
    const BuiltinMethodDef SayAll;
    const BuiltinMethodDef SayTeam;
    const BuiltinMethodDef SetSpawnWeapon;
    const BuiltinMethodDef ShouldDoInitialWeaponRaise;
    const BuiltinMethodDef InitialWeaponRaise;
    const BuiltinMethodDef SetEverHadWeaponAll;
    const BuiltinMethodDef DropItem;
    const BuiltinMethodDef DropScavengerItem;
    const BuiltinMethodDef HasAGrenadePulledBack;
    const BuiltinMethodDef ClearDamageIndicator;
    const BuiltinMethodDef AddToDamageIndicator;
    const BuiltinMethodDef FinishPlayerDamage;
    const BuiltinMethodDef FinishMeleeHit;
    const BuiltinMethodDef ResetAnimations;
    const BuiltinMethodDef RevivePlayer;
    const BuiltinMethodDef Rejack;
    const BuiltinMethodDef InLastStand;
    const BuiltinMethodDef UndoLastStand;
    const BuiltinMethodDef Suicide;
    const BuiltinMethodDef ResetFlashBack;
    const BuiltinMethodDef CreateFlashbackClone;
    const BuiltinMethodDef TurnedHuman;
    const BuiltinMethodDef IsInVehicle;
    const BuiltinMethodDef IsRemoteControlling;
    const BuiltinMethodDef IsWeaponViewOnlyLinked;
    const BuiltinMethodDef AllowStand;
    const BuiltinMethodDef AllowCrouch;
    const BuiltinMethodDef AllowProne;
    const BuiltinMethodDef AllowLean;
    const BuiltinMethodDef AllowSlide;
    const BuiltinMethodDef AllowDoubleJump;
    const BuiltinMethodDef AllowWallRun;
    const BuiltinMethodDef GetVehicleOccupied;
    const BuiltinMethodDef GetFriendlies;
    const BuiltinMethodDef OpenMenu;
    const BuiltinMethodDef CloseMenu;
    const BuiltinMethodDef OpenMenuNoMouse;
    const BuiltinMethodDef CloseInGameMenu;
    const BuiltinMethodDef SetControllerUIModelValue;
    const BuiltinMethodDef GetControllerUIModelValue;
    const BuiltinMethodDef OpenLUIMenu;
    const BuiltinMethodDef GetLUIMenu;
    const BuiltinMethodDef SetLUIMenuData;
    const BuiltinMethodDef GetLUIMenuData;
    const BuiltinMethodDef CloseLUIMenu;
    const BuiltinMethodDef SetElectrifiedState;
    const BuiltinMethodDef FreezeControls;
    const BuiltinMethodDef FreezeControlsAllowLook;
    const BuiltinMethodDef AreControlsFrozen;
    const BuiltinMethodDef DisableUsability;
    const BuiltinMethodDef EnableUsability;
    const BuiltinMethodDef DisableWeapons;
    const BuiltinMethodDef EnableWeapons;
    const BuiltinMethodDef DisableWeaponFire;
    const BuiltinMethodDef EnableWeaponFire;
    const BuiltinMethodDef DisableDeathStreak;
    const BuiltinMethodDef EnableDeathStreak;
    const BuiltinMethodDef DeathStreakActive;
    const BuiltinMethodDef DisableOffhandWeapons;
    const BuiltinMethodDef EnableOffhandWeapons;
    const BuiltinMethodDef OffhandWeaponsEnabled;
    const BuiltinMethodDef DisableOffhandSpecial;
    const BuiltinMethodDef EnableOffhandSpecial;
    const BuiltinMethodDef DisableWeaponCycling;
    const BuiltinMethodDef EnableWeaponCycling;
    const BuiltinMethodDef WeaponCyclingEnabled;
    const BuiltinMethodDef Revive;
    const BuiltinMethodDef SetLastStandPrevWeap;
    const BuiltinMethodDef SetWeaponAmmoClip;
    const BuiltinMethodDef SetWeaponAmmoStock;
    const BuiltinMethodDef SetWeaponAmmoFuel;
    const BuiltinMethodDef ReloadWeaponAmmo;
    const BuiltinMethodDef GetWeaponAmmoClip;
    const BuiltinMethodDef GetWeaponAmmoStock;
    const BuiltinMethodDef GetWeaponAmmoFuel;
    const BuiltinMethodDef GetWeaponOptions;
    const BuiltinMethodDef GetPlayerAttachmentCosmeticVariantIndexes;
    const BuiltinMethodDef AnyAmmoForWeaponModes;
    const BuiltinMethodDef IPrintLn;
    const BuiltinMethodDef IPrintLnBold;
    const BuiltinMethodDef Spawn;
    const BuiltinMethodDef SpawnSpectator;
    const BuiltinMethodDef SetEnterTime;
    const BuiltinMethodDef ClonePlayer;
    const BuiltinMethodDef SetLowReady;
    const BuiltinMethodDef SetCarryingObject;
    const BuiltinMethodDef SetClientFocalLength;
    const BuiltinMethodDef SetClientThirdPerson;
    const BuiltinMethodDef SetClientThirdPersonAngle;
    const BuiltinMethodDef SetClientCompass;
    const BuiltinMethodDef SetClientScriptMainMenu;
    const BuiltinMethodDef SetClientCGObjectiveText;
    const BuiltinMethodDef SetClientPlayerSprintTime;
    const BuiltinMethodDef SetClientNumLives;
    const BuiltinMethodDef SetClientDrawTalk;
    const BuiltinMethodDef SetClientMiniScoreboardHide;
    const BuiltinMethodDef SetClientHUDHardcore;
    const BuiltinMethodDef SetClientPlayerPushAmount;
    const BuiltinMethodDef SetClientUIVisibilityFlag;
    const BuiltinMethodDef SetClientProfileVar;
    const BuiltinMethodDef PlayClientAirstrike;
    const BuiltinMethodDef PlayClientNapalm;
    const BuiltinMethodDef SetCurrentSpectatorClient;
    const BuiltinMethodDef CarryTurret;
    const BuiltinMethodDef StopCarryTurret;
    const BuiltinMethodDef IsCarryingTurret;
    const BuiltinMethodDef CanPlayerPlaceTurret;
    const BuiltinMethodDef SetPlacementHint;
    const BuiltinMethodDef LinkGuidedMissileCamera;
    const BuiltinMethodDef UnlinkGuidedMissileCamera;
    const BuiltinMethodDef LinkToMissile;
    const BuiltinMethodDef UnlinkFromMissile;
    const BuiltinMethodDef DamageRiotShield;
    const BuiltinMethodDef HasRiotShield;
    const BuiltinMethodDef CanPlaceRiotshield;
    const BuiltinMethodDef SetRiotshieldFailHint;
    const BuiltinMethodDef SetTargetOORHint;
    const BuiltinMethodDef SetTargetAlreadyInUseHint;
    const BuiltinMethodDef SetTargetWrongTypeHint;
    const BuiltinMethodDef SetNoTargetsHint;
    const BuiltinMethodDef SetNoLOSOnTargetsHint;
    const BuiltinMethodDef SetDisabledTargetHint;
    const BuiltinMethodDef SetTargetAlreadyTargetedHint;
    const BuiltinMethodDef SetTargetingAbortedHint;
    const BuiltinMethodDef PickupAmmoEvent;
    const BuiltinMethodDef PickupWeaponEvent;
    const BuiltinMethodDef PrimeMovie;
    const BuiltinMethodDef CanPlayerPlaceVehicle;
    const BuiltinMethodDef RemoteControlTurret;
    const BuiltinMethodDef RemoteControlTurretOff;
    const BuiltinMethodDef RecordKillstreakEvent;
    const BuiltinMethodDef RecordKillstreakEndEvent;
    const BuiltinMethodDef RecordKillstreakAssist;
    const BuiltinMethodDef RecordGameEvent;
    const BuiltinMethodDef RecordGameEventNonPlayer;
    const BuiltinMethodDef RecordKillModifier;
    const BuiltinMethodDef RecordMedal;
    const BuiltinMethodDef RecordLoadoutPerksAndKillStreaks;
    const BuiltinMethodDef RecordLoadoutIndex;
    const BuiltinMethodDef RecordMultiKill;
    const BuiltinMethodDef IncrementLifeStat;
    const BuiltinMethodDef IncrementPlayerStat;
    const BuiltinMethodDef RecordPlayerDeathZombies;
    const BuiltinMethodDef RecordPlayerDownZombies;
    const BuiltinMethodDef RecordPlayerReviveZombies;
    const BuiltinMethodDef RecordZombieZone;
    const BuiltinMethodDef RecordCareerFlag;
    const BuiltinMethodDef RecordCareerStat;
    const BuiltinMethodDef RecordCareerStatForMap;
    const BuiltinMethodDef RecordCareerFlagForMap;
    const BuiltinMethodDef MatchRecordSetCheckpointStat;
    const BuiltinMethodDef MatchRecordIncrementCheckpointStat;
    const BuiltinMethodDef MatchRecordLogCybercoreEvent;
    const BuiltinMethodDef MatchRecordPlayerSpawned;
    const BuiltinMethodDef RecordMapEvent;
    const BuiltinMethodDef GetMatchRecordLifeIndex;
    const BuiltinMethodDef RecordZombieGumballEvent;
    const BuiltinMethodDef GetPlayerMovementState;
    const BuiltinMethodDef IsLookingAt;
    const BuiltinMethodDef CantSeeEntities;
    const BuiltinMethodDef PlayHitMarker;
    const BuiltinMethodDef PlayLocalSound;
    const BuiltinMethodDef StopLocalSound;
    const BuiltinMethodDef IsTalking;
    const BuiltinMethodDef AllowSpectateTeam;
    const BuiltinMethodDef GetIpAddress;
    const BuiltinMethodDef GetGuid;
    const BuiltinMethodDef GetXuid;
    const BuiltinMethodDef IsHost;
    const BuiltinMethodDef IsLocalToHost;
    const BuiltinMethodDef AllowADS;
    const BuiltinMethodDef AllowJump;
    const BuiltinMethodDef AllowSprint;
    const BuiltinMethodDef AllowMelee;
    const BuiltinMethodDef AllowMeleePowerLeft;
    const BuiltinMethodDef ForceSlick;
    const BuiltinMethodDef SetSprintDuration;
    const BuiltinMethodDef SetSprintCooldown;
    const BuiltinMethodDef SetSpreadOverride;
    const BuiltinMethodDef ResetSpreadOverride;
    const BuiltinMethodDef SetActionSlot;
    const BuiltinMethodDef GetInventoryWeapon;
    const BuiltinMethodDef SetInventoryWeapon;
    const BuiltinMethodDef GetWeaponsList;
    const BuiltinMethodDef GetWeaponsListPrimaries;
    const BuiltinMethodDef StartRevive;
    const BuiltinMethodDef StopRevive;
    const BuiltinMethodDef ApplyKnockBack;
    const BuiltinMethodDef FreerunSetHighScores;
    const BuiltinMethodDef SetPerk;
    const BuiltinMethodDef HasPerk;
    const BuiltinMethodDef GetPerks;
    const BuiltinMethodDef ClearPerks;
    const BuiltinMethodDef UnSetPerk;
    const BuiltinMethodDef HasDLCAvailable;
    const BuiltinMethodDef GetDLCAvailable;
    const BuiltinMethodDef SetRank;
    const BuiltinMethodDef SetParagonRank;
    const BuiltinMethodDef SetParagonIconId;
    const BuiltinMethodDef EnableInvulnerability;
    const BuiltinMethodDef DisableInvulnerability;
    const BuiltinMethodDef SetArenaPoints;
    const BuiltinMethodDef BotSetRandomCharacterCustomization;
    const BuiltinMethodDef IsSplitscreen;
    const BuiltinMethodDef IsPlayerOnSameMachine;
    const BuiltinMethodDef PlayerKnockback;
    const BuiltinMethodDef GetPlayerViewHeight;
    const BuiltinMethodDef IsHostForBots;
    const BuiltinMethodDef MurderLine;
    const BuiltinMethodDef SetOffhandVisible;
    const BuiltinMethodDef FakeDamageFrom;
    const BuiltinMethodDef GetSnapshotAckIndex;
    const BuiltinMethodDef GetLeagueTeamID;
    const BuiltinMethodDef DisableRoundStartDelay;
    const BuiltinMethodDef EnableRoundStartDelay;
    const BuiltinMethodDef HasCyberComAbility;
    const BuiltinMethodDef SetCyberComAbility;
    const BuiltinMethodDef SetCyberComAbilities;
    const BuiltinMethodDef GetCyberComAbilities;
    const BuiltinMethodDef GetCyberComUpgrades;
    const BuiltinMethodDef GetCyberComRigs;
    const BuiltinMethodDef ClearCyberComAbility;
    const BuiltinMethodDef HasCyberComRig;
    const BuiltinMethodDef SetCyberComRig;
    const BuiltinMethodDef SetCyberComRigsFlags;
    const BuiltinMethodDef SetCyberComAbilityFlags;
    const BuiltinMethodDef SetCyberComUpgradeFlags;
    const BuiltinMethodDef ClearCyberComRig;
    const BuiltinMethodDef SetCyberComActiveType;
    const BuiltinMethodDef GetCyberComActiveType;
    const BuiltinMethodDef PopulateCyberComUnlocks;
    const BuiltinMethodDef SetTacticalWheel;
    const BuiltinMethodDef SetLightingState;
    const BuiltinMethodDef GetLightingState;
    const BuiltinMethodDef SetSunShadowSplitDistance;
    const BuiltinMethodDef GetSunShadowSplitDistance;
    const BuiltinMethodDef SetModelLodBias;
    const BuiltinMethodDef GiveDedicatedShadow;
    const BuiltinMethodDef SetVehicleFocusEntity;
    const BuiltinMethodDef GetVehicleFocusEntity;
    const BuiltinMethodDef GetVehicleBoostTimeLeft;
    const BuiltinMethodDef GetVehicleBoostTime;
    const BuiltinMethodDef GetVehicleMinBoostTime;
    const BuiltinMethodDef SetVehicleDrivableDuration;
    const BuiltinMethodDef SetVehicleDrivableEndTime;
    const BuiltinMethodDef SetVehicleDamageMeter;
    const BuiltinMethodDef SetVehicleLockedOnByEnt;
    const BuiltinMethodDef SetVehicleWeaponWaitDuration;
    const BuiltinMethodDef SetVehicleWeaponWaitEndTime;
    const BuiltinMethodDef ResetInactivityTimer;
    const BuiltinMethodDef SetViewClamp;
  };
  const BuiltinMethodDef methods[402];

  static constexpr const char *names[402] = {
      "GiveWeapon",
      "HasMaxPrimaryWeapons",
      "UpdateWeaponOptions",
      "SetBlockWeaponPickup",
      "GetInventoryHeldWeapon",
      "SetInventoryHeldWeapon",
      "GetWeaponAcvi",
      "CalcWeaponOptions",
      "CalcPlayerOptions",
      "SetPlayerRenderOptions",
      "NextPlayerRenderOption",
      "BlackboxClassChoice",
      "SetHighlighted",
      "SortHeldWeapons",
      "TakeWeapon",
      "IsThrowingGrenade",
      "ForceGrenadeThrow",
      "IsFiring",
      "IsMeleeing",
      "IsGrappling",
      "IsSwitchingWeapons",
      "IsDroppingWeapon",
      "IsReloading",
      "TakeAllWeapons",
      "IsInMoveMode",
      "GetCurrentWeapon",
      "GetCurrentWeaponSpinLerp",
      "SetCurrentWeaponSpinLerp",
      "GetCurrentWeaponAltWeapon",
      "IsWeaponOverheating",
      "SetWeaponOverheating",
      "GetCurrentOffHand",
      "IsUsingOffHand",
      "ForceOffHandEnd",
      "IsChargeShotPending",
      "HasWeapon",
      "GetWeaponForWeaponRoot",
      "SwitchToWeapon",
      "SwitchToWeaponImmediate",
      "SwitchToOffHand",
      "PlayBombPlant",
      "PlayBombDefuse",
      "WeaponPlayEjectBrass",
      "GetLockOnRadius",
      "GetLockOnLossRadius",
      "GetLockOnSpeed",
      "GiveStartAmmo",
      "GiveMaxAmmo",
      "GetFractionStartAmmo",
      "GetFractionMaxAmmo",
      "SetWeaponHeatPercent",
      "SetWeaponHackPercent",
      "SetWeaponLockOnPercent",
      "SetWeaponFriendlyTargeting",
      "SetWeaponFriendlyTargetLocked",
      "SetWeaponFriendlyHacking",
      "SetTargetedEntityEndTime",
      "SetTargetedMissilesRemaining",
      "SetOrigin",
      "SetVelocity",
      "SetPlayerAngles",
      "GetPlayerAngles",
      "GetGunAngles",
      "GetPlayerCameraPos",
      "CameraActivate",
      "CameraSetPosition",
      "CameraSetLookAt",
      "CameraSetAngles",
      "StartCameraTween",
      "GetVRCamPos",
      "GetVRCamAngles",
      "IsVR",
      "GamepadUsedLast",
      "ButtonBitState",
      "PrimaryButtonPressedLocal",
      "SecondaryButtonPressedLocal",
      "UseButtonPressed",
      "ChangeSeatButtonPressed",
      "BoostButtonPressed",
      "AttackButtonPressed",
      "ActionSlotOneButtonPressed",
      "ActionSlotTwoButtonPressed",
      "ActionSlotThreeButtonPressed",
      "ActionSlotFourButtonPressed",
      "AdsButtonPressed",
      "ThrowButtonPressed",
      "MeleeButtonPressed",
      "JumpButtonPressed",
      "ActionButtonPressed",
      "StanceButtonPressed",
      "FragButtonPressed",
      "ReloadButtonPressed",
      "SecondaryOffhandButtonPressed",
      "InventoryButtonPressed",
      "SprintButtonPressed",
      "OffhandSpecialButtonPressed",
      "WeaponSwitchButtonPressed",
      "VehicleMoveUpButtonPressed",
      "SprintUpRequired",
      "PlayerADS",
      "PingPlayer",
      "SetPlayerCharacter",
      "ResetCharacterCustomization",
      "GetCharacterBodyModel",
      "GetCharacterHelmetModel",
      "GetCharacterHeadModel",
      "GetCharacterBodyRenderOptions",
      "SetCharacterBodyType",
      "GetCharacterBodyType",
      "SetCharacterBodyStyle",
      "SetCharacterHelmetStyle",
      "GetPlayerGenderType",
      "GetPlayerShowcaseWeapon",
      "GetPlayerSelectedTaunt",
      "GetPlayerSelectedGesture",
      "GetPlayerSelectedTauntName",
      "GetPlayerSelectedGestureName",
      "ShowViewModel",
      "HideViewModel",
      "SetOffhandSecondaryClass",
      "GetOffHandSecondaryClass",
      "SetOffHandPrimaryClass",
      "BeginLocationSelection",
      "BeginLocationAirstrikeSelection",
      "BeginLocationMortarSelection",
      "BeginLocationArtillerySelection",
      "BeginLocationComlinkSelection",
      "BeginLocationNapalmSelection",
      "EndLocationSelection",
      "ClientSysSetState",
      "WeaponLockDetect",
      "WeaponLockStart",
      "WeaponLockFinalize",
      "WeaponLockFree",
      "WeaponLockTargetTooClose",
      "WeaponLockNoClearance",
      "WeaponLockCount",
      "WeaponLockRemoveSlot",
      "GetMpDialogName",
      "GetHeroWeaponName",
      "GetHeroAbilityName",
      "GetFriendlyDogTagModel",
      "GetEnemyDogTagModel",
      "GetTargetScreenAngles",
      "GetReplayGunLockOnOrigin",
      "GetLockOnOrigin",
      "GetNormalizedMovement",
      "ButtonPressed",
      "GiveWeaponNextAttachment",
      "GiveNextBaseWeapon",
      "GetNormalizedCameraMovement",
      "SayAll",
      "SayTeam",
      "SetSpawnWeapon",
      "ShouldDoInitialWeaponRaise",
      "InitialWeaponRaise",
      "SetEverHadWeaponAll",
      "DropItem",
      "DropScavengerItem",
      "HasAGrenadePulledBack",
      "ClearDamageIndicator",
      "AddToDamageIndicator",
      "FinishPlayerDamage",
      "FinishMeleeHit",
      "ResetAnimations",
      "RevivePlayer",
      "Rejack",
      "InLastStand",
      "UndoLastStand",
      "Suicide",
      "ResetFlashBack",
      "CreateFlashbackClone",
      "TurnedHuman",
      "IsInVehicle",
      "IsRemoteControlling",
      "IsWeaponViewOnlyLinked",
      "AllowStand",
      "AllowCrouch",
      "AllowProne",
      "AllowLean",
      "AllowSlide",
      "AllowDoubleJump",
      "AllowWallRun",
      "GetVehicleOccupied",
      "GetFriendlies",
      "OpenMenu",
      "CloseMenu",
      "OpenMenuNoMouse",
      "CloseInGameMenu",
      "SetControllerUIModelValue",
      "GetControllerUIModelValue",
      "OpenLUIMenu",
      "GetLUIMenu",
      "SetLUIMenuData",
      "GetLUIMenuData",
      "CloseLUIMenu",
      "SetElectrifiedState",
      "FreezeControls",
      "FreezeControlsAllowLook",
      "AreControlsFrozen",
      "DisableUsability",
      "EnableUsability",
      "DisableWeapons",
      "EnableWeapons",
      "DisableWeaponFire",
      "EnableWeaponFire",
      "DisableDeathStreak",
      "EnableDeathStreak",
      "DeathStreakActive",
      "DisableOffhandWeapons",
      "EnableOffhandWeapons",
      "OffhandWeaponsEnabled",
      "DisableOffhandSpecial",
      "EnableOffhandSpecial",
      "DisableWeaponCycling",
      "EnableWeaponCycling",
      "WeaponCyclingEnabled",
      "Revive",
      "SetLastStandPrevWeap",
      "SetWeaponAmmoClip",
      "SetWeaponAmmoStock",
      "SetWeaponAmmoFuel",
      "ReloadWeaponAmmo",
      "GetWeaponAmmoClip",
      "GetWeaponAmmoStock",
      "GetWeaponAmmoFuel",
      "GetWeaponOptions",
      "GetPlayerAttachmentCosmeticVariantIndexes",
      "AnyAmmoForWeaponModes",
      "IPrintLn",
      "IPrintLnBold",
      "Spawn",
      "SpawnSpectator",
      "SetEnterTime",
      "ClonePlayer",
      "SetLowReady",
      "SetCarryingObject",
      "SetClientFocalLength",
      "SetClientThirdPerson",
      "SetClientThirdPersonAngle",
      "SetClientCompass",
      "SetClientScriptMainMenu",
      "SetClientCGObjectiveText",
      "SetClientPlayerSprintTime",
      "SetClientNumLives",
      "SetClientDrawTalk",
      "SetClientMiniScoreboardHide",
      "SetClientHUDHardcore",
      "SetClientPlayerPushAmount",
      "SetClientUIVisibilityFlag",
      "SetClientProfileVar",
      "PlayClientAirstrike",
      "PlayClientNapalm",
      "SetCurrentSpectatorClient",
      "CarryTurret",
      "StopCarryTurret",
      "IsCarryingTurret",
      "CanPlayerPlaceTurret",
      "SetPlacementHint",
      "LinkGuidedMissileCamera",
      "UnlinkGuidedMissileCamera",
      "LinkToMissile",
      "UnlinkFromMissile",
      "DamageRiotShield",
      "HasRiotShield",
      "CanPlaceRiotshield",
      "SetRiotshieldFailHint",
      "SetTargetOORHint",
      "SetTargetAlreadyInUseHint",
      "SetTargetWrongTypeHint",
      "SetNoTargetsHint",
      "SetNoLOSOnTargetsHint",
      "SetDisabledTargetHint",
      "SetTargetAlreadyTargetedHint",
      "SetTargetingAbortedHint",
      "PickupAmmoEvent",
      "PickupWeaponEvent",
      "PrimeMovie",
      "CanPlayerPlaceVehicle",
      "RemoteControlTurret",
      "RemoteControlTurretOff",
      "RecordKillstreakEvent",
      "RecordKillstreakEndEvent",
      "RecordKillstreakAssist",
      "RecordGameEvent",
      "RecordGameEventNonPlayer",
      "RecordKillModifier",
      "RecordMedal",
      "RecordLoadoutPerksAndKillStreaks",
      "RecordLoadoutIndex",
      "RecordMultiKill",
      "IncrementLifeStat",
      "IncrementPlayerStat",
      "RecordPlayerDeathZombies",
      "RecordPlayerDownZombies",
      "RecordPlayerReviveZombies",
      "RecordZombieZone",
      "RecordCareerFlag",
      "RecordCareerStat",
      "RecordCareerStatForMap",
      "RecordCareerFlagForMap",
      "MatchRecordSetCheckpointStat",
      "MatchRecordIncrementCheckpointStat",
      "MatchRecordLogCybercoreEvent",
      "MatchRecordPlayerSpawned",
      "RecordMapEvent",
      "GetMatchRecordLifeIndex",
      "RecordZombieGumballEvent",
      "GetPlayerMovementState",
      "IsLookingAt",
      "CantSeeEntities",
      "PlayHitMarker",
      "PlayLocalSound",
      "StopLocalSound",
      "IsTalking",
      "AllowSpectateTeam",
      "GetIpAddress",
      "GetGuid",
      "GetXuid",
      "IsHost",
      "IsLocalToHost",
      "AllowADS",
      "AllowJump",
      "AllowSprint",
      "AllowMelee",
      "AllowMeleePowerLeft",
      "ForceSlick",
      "SetSprintDuration",
      "SetSprintCooldown",
      "SetSpreadOverride",
      "ResetSpreadOverride",
      "SetActionSlot",
      "GetInventoryWeapon",
      "SetInventoryWeapon",
      "GetWeaponsList",
      "GetWeaponsListPrimaries",
      "StartRevive",
      "StopRevive",
      "ApplyKnockBack",
      "FreerunSetHighScores",
      "SetPerk",
      "HasPerk",
      "GetPerks",
      "ClearPerks",
      "UnSetPerk",
      "HasDLCAvailable",
      "GetDLCAvailable",
      "SetRank",
      "SetParagonRank",
      "SetParagonIconId",
      "EnableInvulnerability",
      "DisableInvulnerability",
      "SetArenaPoints",
      "BotSetRandomCharacterCustomization",
      "IsSplitscreen",
      "IsPlayerOnSameMachine",
      "PlayerKnockback",
      "GetPlayerViewHeight",
      "IsHostForBots",
      "MurderLine",
      "SetOffhandVisible",
      "FakeDamageFrom",
      "GetSnapshotAckIndex",
      "GetLeagueTeamID",
      "DisableRoundStartDelay",
      "EnableRoundStartDelay",
      "HasCyberComAbility",
      "SetCyberComAbility",
      "SetCyberComAbilities",
      "GetCyberComAbilities",
      "GetCyberComUpgrades",
      "GetCyberComRigs",
      "ClearCyberComAbility",
      "HasCyberComRig",
      "SetCyberComRig",
      "SetCyberComRigsFlags",
      "SetCyberComAbilityFlags",
      "SetCyberComUpgradeFlags",
      "ClearCyberComRig",
      "SetCyberComActiveType",
      "GetCyberComActiveType",
      "PopulateCyberComUnlocks",
      "SetTacticalWheel",
      "SetLightingState",
      "GetLightingState",
      "SetSunShadowSplitDistance",
      "GetSunShadowSplitDistance",
      "SetModelLodBias",
      "GiveDedicatedShadow",
      "SetVehicleFocusEntity",
      "GetVehicleFocusEntity",
      "GetVehicleBoostTimeLeft",
      "GetVehicleBoostTime",
      "GetVehicleMinBoostTime",
      "SetVehicleDrivableDuration",
      "SetVehicleDrivableEndTime",
      "SetVehicleDamageMeter",
      "SetVehicleLockedOnByEnt",
      "SetVehicleWeaponWaitDuration",
      "SetVehicleWeaponWaitEndTime",
      "ResetInactivityTimer",
      "SetViewClamp",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(PlayerMethodTable, sizeof(PlayerMethodTable::methods));

union CommonFunctionTable {
  struct {
    const BuiltinFunctionDef ReportStubUsage;
    const BuiltinFunctionDef Record3DText;
    const BuiltinFunctionDef RecordEntText;
    const BuiltinFunctionDef RecordLine;
    const BuiltinFunctionDef RecordStar;
    const BuiltinFunctionDef RecordSphere;
    const BuiltinFunctionDef RecordCircle;
    const BuiltinFunctionDef RecordCone;
    const BuiltinFunctionDef RecordEnt;
    const BuiltinFunctionDef RecorderPlayback;
    const BuiltinFunctionDef Print;
    const BuiltinFunctionDef PrintLn;
    const BuiltinFunctionDef PrintTopRightln;
    const BuiltinFunctionDef IPrintLn;
    const BuiltinFunctionDef IPrintLnBold;
    const BuiltinFunctionDef Print3D;
    const BuiltinFunctionDef Line;
    const BuiltinFunctionDef LineList;
    const BuiltinFunctionDef Box;
    const BuiltinFunctionDef Circle;
    const BuiltinFunctionDef SphericalCone;
    const BuiltinFunctionDef DebugStar;
    const BuiltinFunctionDef Sphere;
    const BuiltinFunctionDef GetEnterButton;
    const BuiltinFunctionDef ArraySort;
    const BuiltinFunctionDef ArraySortClosest;
    const BuiltinFunctionDef GetEnt;
    const BuiltinFunctionDef GetEntArray;
    const BuiltinFunctionDef GetEntArrayFromArray;
    const BuiltinFunctionDef IsNavVolumeLoaded;
    const BuiltinFunctionDef GeneratePointsAroundCenter;
    const BuiltinFunctionDef GetAngleFromBits;
    const BuiltinFunctionDef GetBitsForAngle;
    const BuiltinFunctionDef PredictPosition;
    const BuiltinFunctionDef GetNode;
    const BuiltinFunctionDef GetNodeArray;
    const BuiltinFunctionDef GetNodeArraySorted;
    const BuiltinFunctionDef GetAnyNodeArray;
    const BuiltinFunctionDef GetCoverNodeArray;
    const BuiltinFunctionDef GetAllNodes;
    const BuiltinFunctionDef GetNodesInRadius;
    const BuiltinFunctionDef GetNodesInRadiusSorted;
    const BuiltinFunctionDef GetNearestNode;
    const BuiltinFunctionDef GetVisibleNode;
    const BuiltinFunctionDef NodesVisible;
    const BuiltinFunctionDef CanClaimNode;
    const BuiltinFunctionDef GetNodeRegion;
    const BuiltinFunctionDef GetValidCoverPeekOuts;
    const BuiltinFunctionDef GetVisibleNodes;
    const BuiltinFunctionDef SetEnableNode;
    const BuiltinFunctionDef LinkNodes;
    const BuiltinFunctionDef UnLinkNodes;
    const BuiltinFunctionDef LinkTraversal;
    const BuiltinFunctionDef UnlinkTraversal;
    const BuiltinFunctionDef NodesAreLinked;
    const BuiltinFunctionDef DropNodeToFloor;
    const BuiltinFunctionDef SpawnPathNode;
    const BuiltinFunctionDef DeletePathNode;
    const BuiltinFunctionDef SetTurretNode;
    const BuiltinFunctionDef Ld_Hallway_Front;
    const BuiltinFunctionDef SetNodePriority;
    const BuiltinFunctionDef IsNodeOccupied;
    const BuiltinFunctionDef GetNodeOwner;
    const BuiltinFunctionDef IsWallrunNode;
    const BuiltinFunctionDef IsCoverNode;
    const BuiltinFunctionDef PathDistance;
    const BuiltinFunctionDef GetNearestPathPoint;
    const BuiltinFunctionDef IsNodeEnabled;
    const BuiltinFunctionDef GetOtherNodeInNegotiationPair;
    const BuiltinFunctionDef GetClosestPointOnNavMesh;
    const BuiltinFunctionDef TracePassedOnNavMesh;
    const BuiltinFunctionDef CheckNavMeshDirection;
    const BuiltinFunctionDef MarkNoVehicleNavMeshFaces;
    const BuiltinFunctionDef ToggleNoVehicleFaceTraversability;
    const BuiltinFunctionDef PositionQuery_Source_Navigation;
    const BuiltinFunctionDef PositionQuery_Filter_Sight;
    const BuiltinFunctionDef PositionQuery_Filter_Directness;
    const BuiltinFunctionDef PositionQuery_Filter_DistanceToGoal;
    const BuiltinFunctionDef PositionQuery_Filter_InClaimedLocation;
    const BuiltinFunctionDef NavPointSightFilter;
    const BuiltinFunctionDef IsPointOnNavMesh;
    const BuiltinFunctionDef IsPointOnStairs;
    const BuiltinFunctionDef SetClearanceCeiling;
    const BuiltinFunctionDef GetNavMeshFaceNormal;
    const BuiltinFunctionDef GetNavFaceRegion;
    const BuiltinFunctionDef EnableNavMeshTrigger;
    const BuiltinFunctionDef GetNavMeshTriggersForPoint;
    const BuiltinFunctionDef IsPointInNavVolume;
    const BuiltinFunctionDef GetReflectionLocs;
    const BuiltinFunctionDef GetReflectionOrigin;
    const BuiltinFunctionDef BBPrint;
    const BuiltinFunctionDef BBPostDemoStreamStatsForRound;
    const BuiltinFunctionDef RecordComScoreEvent;
    const BuiltinFunctionDef GetVehicleNode;
    const BuiltinFunctionDef GetVehicleNodeArray;
    const BuiltinFunctionDef GetAllVehicleNodes;
    const BuiltinFunctionDef GetTimeFromVehicleNodeToNode;
    const BuiltinFunctionDef ReconnectVehicleNodes;
    const BuiltinFunctionDef CreateTrack;
    const BuiltinFunctionDef GetNumVehicles;
    const BuiltinFunctionDef SetHeliHeightPatchEnabled;
    const BuiltinFunctionDef DoesWeaponReplaceSpawnWeapon;
    const BuiltinFunctionDef GetWeaponWorldModel;
    const BuiltinFunctionDef AddDebugCommand;
    const BuiltinFunctionDef IsEntity;
    const BuiltinFunctionDef IsAlive;
    const BuiltinFunctionDef IsCorpse;
    const BuiltinFunctionDef IsActorCorpse;
    const BuiltinFunctionDef IsActorSpawner;
    const BuiltinFunctionDef IsVehicleSpawner;
    const BuiltinFunctionDef IsSpawner;
    const BuiltinFunctionDef IsPlayer;
    const BuiltinFunctionDef IsAI;
    const BuiltinFunctionDef IsActor;
    const BuiltinFunctionDef IsVehicle;
    const BuiltinFunctionDef IsSentient;
    const BuiltinFunctionDef IsPathfinder;
    const BuiltinFunctionDef IsAirborne;
    const BuiltinFunctionDef IsUsingNavVolume;
    const BuiltinFunctionDef IsGodMode;
    const BuiltinFunctionDef HasASM;
    const BuiltinFunctionDef SetDvar;
    const BuiltinFunctionDef EACPathSet;
    const BuiltinFunctionDef GetTime;
    const BuiltinFunctionDef GetMillisecondsRaw;
    const BuiltinFunctionDef GetMicrosecondsRaw;
    const BuiltinFunctionDef SessionModeIsOnlineGame;
    const BuiltinFunctionDef SessionModeIsPrivateOnlineGame;
    const BuiltinFunctionDef SessionModeIsPrivate;
    const BuiltinFunctionDef SessionModeIsSystemlink;
    const BuiltinFunctionDef SessionModeIsZombiesGame;
    const BuiltinFunctionDef SessionModeIsCampaignGame;
    const BuiltinFunctionDef SessionModeIsMultiplayerGame;
    const BuiltinFunctionDef SessionModeAbbreviation;
    const BuiltinFunctionDef CurrentSessionMode;
    const BuiltinFunctionDef IsDedicated;
    const BuiltinFunctionDef GameModeIsMode;
    const BuiltinFunctionDef GameModeIsUsingXP;
    const BuiltinFunctionDef GameModeIsUsingStats;
    const BuiltinFunctionDef GetGametypeSetting;
    const BuiltinFunctionDef SetGametypeSetting;
    const BuiltinFunctionDef GameModeIsArena;
    const BuiltinFunctionDef CheckpointCreate;
    const BuiltinFunctionDef CheckpointCommit;
    const BuiltinFunctionDef CheckpointClear;
    const BuiltinFunctionDef CheckpointRestore;
    const BuiltinFunctionDef SaveGame_Create;
    const BuiltinFunctionDef SaveGame_ResetCampaignProgress;
    const BuiltinFunctionDef SaveGame_GetSavedMap;
    const BuiltinFunctionDef SaveGame_GetSavedQueuedMap;
    const BuiltinFunctionDef PlaySoundAtPosition;
    const BuiltinFunctionDef SetLocalProfileVar;
    const BuiltinFunctionDef SetLocalProfileArrayVar;
    const BuiltinFunctionDef GetLocalProfileInt;
    const BuiltinFunctionDef GetLocalProfileArrayInt;
    const BuiltinFunctionDef GetLocalProfileFloat;
    const BuiltinFunctionDef GetLocalProfileString;
    const BuiltinFunctionDef ExecDevgui;
    const BuiltinFunctionDef UploadStats;
    const BuiltinFunctionDef RegisterXP;
    const BuiltinFunctionDef IsMature;
    const BuiltinFunctionDef CamAnimScripted;
    const BuiltinFunctionDef EndCamAnimScripted;
    const BuiltinFunctionDef ExtraCamAnimScripted;
    const BuiltinFunctionDef EndExtraCamAnimScripted;
    const BuiltinFunctionDef GetCamAnimTime;
    const BuiltinFunctionDef IsCamAnimLooping;
    const BuiltinFunctionDef GetClassIndexFromName;
    const BuiltinFunctionDef GetAIArray;
    const BuiltinFunctionDef GetVehicleArray;
    const BuiltinFunctionDef GetActorArray;
    const BuiltinFunctionDef GetAISpeciesArray;
    const BuiltinFunctionDef GetAIArchetypeArray;
    const BuiltinFunctionDef GetFreeActorCount;
    const BuiltinFunctionDef GetAITeamArray;
    const BuiltinFunctionDef GetActorTeamArray;
    const BuiltinFunctionDef GetActorTeamCountRadius;
    const BuiltinFunctionDef GetVehicleTeamArray;
    const BuiltinFunctionDef GetScriptMoverArray;
    const BuiltinFunctionDef GetZBarrierArray;
    const BuiltinFunctionDef GetItemArray;
    const BuiltinFunctionDef GetCorpseArray;
    const BuiltinFunctionDef GetSpawnerArray;
    const BuiltinFunctionDef GetVehicleSpawnerArray;
    const BuiltinFunctionDef GetActorSpawnerArray;
    const BuiltinFunctionDef GetSpawnerTeamArray;
    const BuiltinFunctionDef GetActorSpawnerTeamArray;
    const BuiltinFunctionDef GetVehicleSpawnerTeamArray;
    const BuiltinFunctionDef GetGrappleTargetArray;
    const BuiltinFunctionDef BadPlace_Delete;
    const BuiltinFunctionDef BadPlace_Box;
    const BuiltinFunctionDef BadPlace_Cylinder;
    const BuiltinFunctionDef PhysicsTraceEx;
    const BuiltinFunctionDef LUINotifyEvent;
    const BuiltinFunctionDef ProfileLog_BeginTiming;
    const BuiltinFunctionDef ProfileLog_EndTiming;
    const BuiltinFunctionDef GetNotetracksInDelta;
    const BuiltinFunctionDef IsAnimLeaf;
    const BuiltinFunctionDef IsAnimLooping;
    const BuiltinFunctionDef IsAnimLoaded;
    const BuiltinFunctionDef GetAnimLength;
    const BuiltinFunctionDef GetAnimFrameCount;
    const BuiltinFunctionDef AnimHasNotetrack;
    const BuiltinFunctionDef GetNotetrackTimes;
    const BuiltinFunctionDef GetStartOrigin;
    const BuiltinFunctionDef GetStartAngles;
    const BuiltinFunctionDef DisableGrenadeSuicide;
    const BuiltinFunctionDef EnableGrenadeSuicide;
    const BuiltinFunctionDef ActivateClientExploder;
    const BuiltinFunctionDef DeactivateClientExploder;
    const BuiltinFunctionDef ActivateClientRadiantExploder;
    const BuiltinFunctionDef DeactivateClientRadiantExploder;
    const BuiltinFunctionDef KillClientRadiantExploder;
    const BuiltinFunctionDef GetMissileOwner;
    const BuiltinFunctionDef PrecacheLeaderboards;
    const BuiltinFunctionDef IsAssetLoaded;
    const BuiltinFunctionDef DeployRiotShield;
    const BuiltinFunctionDef GetNumExpectedPlayers;
    const BuiltinFunctionDef GetNumConnectedPlayers;
    const BuiltinFunctionDef IsLoadingCinematicPlaying;
    const BuiltinFunctionDef GetAtrLoaded;
    const BuiltinFunctionDef FindAnimByName;
    const BuiltinFunctionDef AnimHasImportantNotifies;
    const BuiltinFunctionDef SetHideonClientWhenScriptedAnimCompleted;
    const BuiltinFunctionDef RegisterClientField;
    const BuiltinFunctionDef CodeSetClientField;
    const BuiltinFunctionDef CodeGetClientField;
    const BuiltinFunctionDef CodeIncrementClientField;
    const BuiltinFunctionDef CodeSetPlayerStateClientField;
    const BuiltinFunctionDef CodeGetPlayerStateClientField;
    const BuiltinFunctionDef CodeIncrementUIModelClientField;
    const BuiltinFunctionDef CodeSetUIModelClientField;
    const BuiltinFunctionDef CodeGetUIModelClientField;
    const BuiltinFunctionDef CodeIncrementPlayerStateClientField;
    const BuiltinFunctionDef CodeSetWorldClientField;
    const BuiltinFunctionDef CodeGetWorldClientField;
    const BuiltinFunctionDef CodeIncrementWorldClientField;
    const BuiltinFunctionDef GetClientFieldVersion;
    const BuiltinFunctionDef SetSavedDvar;
    const BuiltinFunctionDef GetWeapon;
    const BuiltinFunctionDef GetWeaponAccuracy;
    const BuiltinFunctionDef GetAttachmentCosmeticVariantIndexes;
    const BuiltinFunctionDef GetRandomCompatibleAttachmentsForWeapon;
    const BuiltinFunctionDef SetAILimit;
    const BuiltinFunctionDef GetAILimit;
    const BuiltinFunctionDef GetAICount;
    const BuiltinFunctionDef ResetAILimit;
    const BuiltinFunctionDef AwarenessUpdatePoolThresholds;
    const BuiltinFunctionDef AwarenessSetWeaponShotDistance;
    const BuiltinFunctionDef SwitchMap_Preload;
    const BuiltinFunctionDef SwitchMap_Load;
    const BuiltinFunctionDef SwitchMap_Switch;
    const BuiltinFunctionDef SwitchMap_SetLoadingMovie;
    const BuiltinFunctionDef GetNextMap;
    const BuiltinFunctionDef GetMapOrder;
    const BuiltinFunctionDef GetMapIntroMovie;
    const BuiltinFunctionDef GetMapOutroMovie;
    const BuiltinFunctionDef IsMapSubLevel;
    const BuiltinFunctionDef GetRootMapName;
    const BuiltinFunctionDef GetMissionName;
    const BuiltinFunctionDef MissionHasAccolades;
    const BuiltinFunctionDef MissionHasCollectibles;
    const BuiltinFunctionDef GetNumberOfCollectiblesForLevel;
    const BuiltinFunctionDef GetMissionUniqueID;
    const BuiltinFunctionDef GetMissionVersion;
    const BuiltinFunctionDef GetFreerunTrackIndex;
    const BuiltinFunctionDef GetPlayerCharacterAttribute;
    const BuiltinFunctionDef RatRecordMessage;
    const BuiltinFunctionDef RatReportCommandResult;
    const BuiltinFunctionDef GetHostMigrationValue;
    const BuiltinFunctionDef GetHostMigrationArrayCount;
    const BuiltinFunctionDef SetHostMigrationValue;
    const BuiltinFunctionDef SetLightingState;
    const BuiltinFunctionDef VideoPrime;
    const BuiltinFunctionDef VideoStart;
    const BuiltinFunctionDef VideoPause;
    const BuiltinFunctionDef VideoUnpause;
    const BuiltinFunctionDef VideoStop;
    const BuiltinFunctionDef HideMiscModels;
    const BuiltinFunctionDef ShowMiscModels;
    const BuiltinFunctionDef UmbraGate_Set;
    const BuiltinFunctionDef StreamerSkiptoDebug;
    const BuiltinFunctionDef UpdateNavTriggers;
    const BuiltinFunctionDef IsArenaMode;
    const BuiltinFunctionDef ShowInfoVolume;
    const BuiltinFunctionDef HideInfoVolume;
    const BuiltinFunctionDef InfoVolumeDebugInit;
  };
  const BuiltinFunctionDef functions[287];

  static constexpr const char *names[287] = {
      "ReportStubUsage",
      "Record3DText",
      "RecordEntText",
      "RecordLine",
      "RecordStar",
      "RecordSphere",
      "RecordCircle",
      "RecordCone",
      "RecordEnt",
      "RecorderPlayback",
      "Print",
      "PrintLn",
      "PrintTopRightln",
      "IPrintLn",
      "IPrintLnBold",
      "Print3D",
      "Line",
      "LineList",
      "Box",
      "Circle",
      "SphericalCone",
      "DebugStar",
      "Sphere",
      "GetEnterButton",
      "ArraySort",
      "ArraySortClosest",
      "GetEnt",
      "GetEntArray",
      "GetEntArrayFromArray",
      "IsNavVolumeLoaded",
      "GeneratePointsAroundCenter",
      "GetAngleFromBits",
      "GetBitsForAngle",
      "PredictPosition",
      "GetNode",
      "GetNodeArray",
      "GetNodeArraySorted",
      "GetAnyNodeArray",
      "GetCoverNodeArray",
      "GetAllNodes",
      "GetNodesInRadius",
      "GetNodesInRadiusSorted",
      "GetNearestNode",
      "GetVisibleNode",
      "NodesVisible",
      "CanClaimNode",
      "GetNodeRegion",
      "GetValidCoverPeekOuts",
      "GetVisibleNodes",
      "SetEnableNode",
      "LinkNodes",
      "UnLinkNodes",
      "LinkTraversal",
      "UnlinkTraversal",
      "NodesAreLinked",
      "DropNodeToFloor",
      "SpawnPathNode",
      "DeletePathNode",
      "SetTurretNode",
      "Ld_Hallway_Front",
      "SetNodePriority",
      "IsNodeOccupied",
      "GetNodeOwner",
      "IsWallrunNode",
      "IsCoverNode",
      "PathDistance",
      "GetNearestPathPoint",
      "IsNodeEnabled",
      "GetOtherNodeInNegotiationPair",
      "GetClosestPointOnNavMesh",
      "TracePassedOnNavMesh",
      "CheckNavMeshDirection",
      "MarkNoVehicleNavMeshFaces",
      "ToggleNoVehicleFaceTraversability",
      "PositionQuery_Source_Navigation",
      "PositionQuery_Filter_Sight",
      "PositionQuery_Filter_Directness",
      "PositionQuery_Filter_DistanceToGoal",
      "PositionQuery_Filter_InClaimedLocation",
      "NavPointSightFilter",
      "IsPointOnNavMesh",
      "IsPointOnStairs",
      "SetClearanceCeiling",
      "GetNavMeshFaceNormal",
      "GetNavFaceRegion",
      "EnableNavMeshTrigger",
      "GetNavMeshTriggersForPoint",
      "IsPointInNavVolume",
      "GetReflectionLocs",
      "GetReflectionOrigin",
      "BBPrint",
      "BBPostDemoStreamStatsForRound",
      "RecordComScoreEvent",
      "GetVehicleNode",
      "GetVehicleNodeArray",
      "GetAllVehicleNodes",
      "GetTimeFromVehicleNodeToNode",
      "ReconnectVehicleNodes",
      "CreateTrack",
      "GetNumVehicles",
      "SetHeliHeightPatchEnabled",
      "DoesWeaponReplaceSpawnWeapon",
      "GetWeaponWorldModel",
      "AddDebugCommand",
      "IsEntity",
      "IsAlive",
      "IsCorpse",
      "IsActorCorpse",
      "IsActorSpawner",
      "IsVehicleSpawner",
      "IsSpawner",
      "IsPlayer",
      "IsAI",
      "IsActor",
      "IsVehicle",
      "IsSentient",
      "IsPathfinder",
      "IsAirborne",
      "IsUsingNavVolume",
      "IsGodMode",
      "HasASM",
      "SetDvar",
      "EACPathSet",
      "GetTime",
      "GetMillisecondsRaw",
      "GetMicrosecondsRaw",
      "SessionModeIsOnlineGame",
      "SessionModeIsPrivateOnlineGame",
      "SessionModeIsPrivate",
      "SessionModeIsSystemlink",
      "SessionModeIsZombiesGame",
      "SessionModeIsCampaignGame",
      "SessionModeIsMultiplayerGame",
      "SessionModeAbbreviation",
      "CurrentSessionMode",
      "IsDedicated",
      "GameModeIsMode",
      "GameModeIsUsingXP",
      "GameModeIsUsingStats",
      "GetGametypeSetting",
      "SetGametypeSetting",
      "GameModeIsArena",
      "CheckpointCreate",
      "CheckpointCommit",
      "CheckpointClear",
      "CheckpointRestore",
      "SaveGame_Create",
      "SaveGame_ResetCampaignProgress",
      "SaveGame_GetSavedMap",
      "SaveGame_GetSavedQueuedMap",
      "PlaySoundAtPosition",
      "SetLocalProfileVar",
      "SetLocalProfileArrayVar",
      "GetLocalProfileInt",
      "GetLocalProfileArrayInt",
      "GetLocalProfileFloat",
      "GetLocalProfileString",
      "ExecDevgui",
      "UploadStats",
      "RegisterXP",
      "IsMature",
      "CamAnimScripted",
      "EndCamAnimScripted",
      "ExtraCamAnimScripted",
      "EndExtraCamAnimScripted",
      "GetCamAnimTime",
      "IsCamAnimLooping",
      "GetClassIndexFromName",
      "GetAIArray",
      "GetVehicleArray",
      "GetActorArray",
      "GetAISpeciesArray",
      "GetAIArchetypeArray",
      "GetFreeActorCount",
      "GetAITeamArray",
      "GetActorTeamArray",
      "GetActorTeamCountRadius",
      "GetVehicleTeamArray",
      "GetScriptMoverArray",
      "GetZBarrierArray",
      "GetItemArray",
      "GetCorpseArray",
      "GetSpawnerArray",
      "GetVehicleSpawnerArray",
      "GetActorSpawnerArray",
      "GetSpawnerTeamArray",
      "GetActorSpawnerTeamArray",
      "GetVehicleSpawnerTeamArray",
      "GetGrappleTargetArray",
      "BadPlace_Delete",
      "BadPlace_Box",
      "BadPlace_Cylinder",
      "PhysicsTraceEx",
      "LUINotifyEvent",
      "ProfileLog_BeginTiming",
      "ProfileLog_EndTiming",
      "GetNotetracksInDelta",
      "IsAnimLeaf",
      "IsAnimLooping",
      "IsAnimLoaded",
      "GetAnimLength",
      "GetAnimFrameCount",
      "AnimHasNotetrack",
      "GetNotetrackTimes",
      "GetStartOrigin",
      "GetStartAngles",
      "DisableGrenadeSuicide",
      "EnableGrenadeSuicide",
      "ActivateClientExploder",
      "DeactivateClientExploder",
      "ActivateClientRadiantExploder",
      "DeactivateClientRadiantExploder",
      "KillClientRadiantExploder",
      "GetMissileOwner",
      "PrecacheLeaderboards",
      "IsAssetLoaded",
      "DeployRiotShield",
      "GetNumExpectedPlayers",
      "GetNumConnectedPlayers",
      "IsLoadingCinematicPlaying",
      "GetAtrLoaded",
      "FindAnimByName",
      "AnimHasImportantNotifies",
      "SetHideonClientWhenScriptedAnimCompleted",
      "RegisterClientField",
      "CodeSetClientField",
      "CodeGetClientField",
      "CodeIncrementClientField",
      "CodeSetPlayerStateClientField",
      "CodeGetPlayerStateClientField",
      "CodeIncrementUIModelClientField",
      "CodeSetUIModelClientField",
      "CodeGetUIModelClientField",
      "CodeIncrementPlayerStateClientField",
      "CodeSetWorldClientField",
      "CodeGetWorldClientField",
      "CodeIncrementWorldClientField",
      "GetClientFieldVersion",
      "SetSavedDvar",
      "GetWeapon",
      "GetWeaponAccuracy",
      "GetAttachmentCosmeticVariantIndexes",
      "GetRandomCompatibleAttachmentsForWeapon",
      "SetAILimit",
      "GetAILimit",
      "GetAICount",
      "ResetAILimit",
      "AwarenessUpdatePoolThresholds",
      "AwarenessSetWeaponShotDistance",
      "SwitchMap_Preload",
      "SwitchMap_Load",
      "SwitchMap_Switch",
      "SwitchMap_SetLoadingMovie",
      "GetNextMap",
      "GetMapOrder",
      "GetMapIntroMovie",
      "GetMapOutroMovie",
      "IsMapSubLevel",
      "GetRootMapName",
      "GetMissionName",
      "MissionHasAccolades",
      "MissionHasCollectibles",
      "GetNumberOfCollectiblesForLevel",
      "GetMissionUniqueID",
      "GetMissionVersion",
      "GetFreerunTrackIndex",
      "GetPlayerCharacterAttribute",
      "RatRecordMessage",
      "RatReportCommandResult",
      "GetHostMigrationValue",
      "GetHostMigrationArrayCount",
      "SetHostMigrationValue",
      "SetLightingState",
      "VideoPrime",
      "VideoStart",
      "VideoPause",
      "VideoUnpause",
      "VideoStop",
      "HideMiscModels",
      "ShowMiscModels",
      "UmbraGate_Set",
      "StreamerSkiptoDebug",
      "UpdateNavTriggers",
      "IsArenaMode",
      "ShowInfoVolume",
      "HideInfoVolume",
      "InfoVolumeDebugInit",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(CommonFunctionTable, sizeof(CommonFunctionTable::functions));

union CommonMethodTable {
  struct {
    const BuiltinMethodDef Attach;
    const BuiltinMethodDef Detach;
    const BuiltinMethodDef DetachAll;
    const BuiltinMethodDef GetAttachSize;
    const BuiltinMethodDef GetAttachModelName;
    const BuiltinMethodDef GetAttachTagName;
    const BuiltinMethodDef GetAttachIgnoreCollision;
    const BuiltinMethodDef HidePart;
    const BuiltinMethodDef HasPart;
    const BuiltinMethodDef ShowPart;
    const BuiltinMethodDef ShowAllParts;
    const BuiltinMethodDef PlaySoundToPlayer;
    const BuiltinMethodDef PlaySoundToAllButPlayer;
    const BuiltinMethodDef AllowedStances;
    const BuiltinMethodDef AreAllMissionsAtScore;
    const BuiltinMethodDef IsStartingClassDefault;
    const BuiltinMethodDef GetNumChallengesComplete;
    const BuiltinMethodDef HasCompletedAllGameChallenges;
    const BuiltinMethodDef HasCollectedAllCollectibles;
    const BuiltinMethodDef GetDStat;
    const BuiltinMethodDef GetDStatArrayCount;
    const BuiltinMethodDef SetAARStat;
    const BuiltinMethodDef SetDStat;
    const BuiltinMethodDef AddDStat;
    const BuiltinMethodDef SetNonCheckpointData;
    const BuiltinMethodDef GetNonCheckpointData;
    const BuiltinMethodDef ClearAllNonCheckpointData;
    const BuiltinMethodDef ClearNonCheckpointData;
    const BuiltinMethodDef AddWeaponStat;
    const BuiltinMethodDef AddSpecialistStat;
    const BuiltinMethodDef IncrementSpecificWeaponPickedUpCount;
    const BuiltinMethodDef AddBonusCardStat;
    const BuiltinMethodDef GetSessStat;
    const BuiltinMethodDef GetSessStatArrayCount;
    const BuiltinMethodDef SetSessStat;
    const BuiltinMethodDef AddSessStat;
    const BuiltinMethodDef UnlockSongByAlias;
    const BuiltinMethodDef CheckIfSongUnlocked;
    const BuiltinMethodDef GiveMissingUnlockTokens;
    const BuiltinMethodDef UpdateUnlockedAttachmentBits;
    const BuiltinMethodDef GetDecorations;
    const BuiltinMethodDef GiveDecoration;
    const BuiltinMethodDef AddPlayerStat;
    const BuiltinMethodDef AddPlayerStatWithGameType;
    const BuiltinMethodDef AddGameTypeStat;
    const BuiltinMethodDef AddMissionStat;
    const BuiltinMethodDef GetCurrentGunRank;
    const BuiltinMethodDef TrackHeroPowerAvailable;
    const BuiltinMethodDef TrackHeroPowerActivated;
    const BuiltinMethodDef TrackHeroPowerExpired;
    const BuiltinMethodDef TrackWeaponFireNative;
    const BuiltinMethodDef UpdateStatRatio;
    const BuiltinMethodDef AddUnlockToken_Weapon;
    const BuiltinMethodDef AddUnlockToken_Attachment;
    const BuiltinMethodDef GiveUnlockToken;
    const BuiltinMethodDef GetXPScale;
    const BuiltinMethodDef GetCryptoScale;
    const BuiltinMethodDef GetVialsScale;
    const BuiltinMethodDef AddRankXP;
    const BuiltinMethodDef AddRankXPValue;
    const BuiltinMethodDef DoubleXPTimerFired;
    const BuiltinMethodDef ReportLootReward;
    const BuiltinMethodDef ReportLootConsume;
    const BuiltinMethodDef PROTECTED(NoteLootConsume);
    const BuiltinMethodDef PROTECTED(ReportNotedLoot);
    const BuiltinMethodDef HasSeasonPass;
    const BuiltinMethodDef IsStarterPack;
    const BuiltinMethodDef SetEMPJammed;
    const BuiltinMethodDef IsEMPJammed;
    const BuiltinMethodDef SetGrapplableType;
    const BuiltinMethodDef LUINotifyEvent;
    const BuiltinMethodDef Launch;
    const BuiltinMethodDef SetMovingPlatformEnabled;
    const BuiltinMethodDef IsMovingPlatform;
    const BuiltinMethodDef SetDrawInfrared;
    const BuiltinMethodDef SetAssassinationEnabled;
    const BuiltinMethodDef GameHistoryStartMatch;
    const BuiltinMethodDef GameHistoryFinishMatch;
    const BuiltinMethodDef Unlink;
    const BuiltinMethodDef LinkTo;
    const BuiltinMethodDef LinkToBlendToTag;
    const BuiltinMethodDef EnableLinkTo;
    const BuiltinMethodDef LinkToUpdateOffset;
    const BuiltinMethodDef PlayerLinkTo;
    const BuiltinMethodDef PlayerLinkToBlend;
    const BuiltinMethodDef PlayerLinkToDelta;
    const BuiltinMethodDef PlayerLinkWeaponViewToDelta;
    const BuiltinMethodDef PlayerLinkToAbsolute;
    const BuiltinMethodDef PlayerCamLinkTo;
    const BuiltinMethodDef PlayerCamUnlink;
    const BuiltinMethodDef LerpViewAngleClamp;
    const BuiltinMethodDef SetViewAngleResistance;
    const BuiltinMethodDef PlayerLinkedOffsetEnable;
    const BuiltinMethodDef PlayerLinkedOffsetDisable;
    const BuiltinMethodDef PlayerLinkedSetUseBaseAngleForViewClamp;
    const BuiltinMethodDef ClearGroundEnt;
    const BuiltinMethodDef DontInterpolate;
    const BuiltinMethodDef MagicGrenadeType;
    const BuiltinMethodDef StartFadingBlur;
    const BuiltinMethodDef LocalToWorldCoords;
    const BuiltinMethodDef WorldToLocalCoords;
    const BuiltinMethodDef SetTurretSpinning;
    const BuiltinMethodDef AttachShieldModel;
    const BuiltinMethodDef DetachShieldModel;
    const BuiltinMethodDef SetScriptMoverFlag;
    const BuiltinMethodDef RefreshShieldAttachment;
    const BuiltinMethodDef MarkDestructibleDestroyed;
    const BuiltinMethodDef ResetDestructible;
    const BuiltinMethodDef FindPath;
    const BuiltinMethodDef CanPath;
    const BuiltinMethodDef GetNodeIndexOnPath;
    const BuiltinMethodDef GetNextTraversalNodeOnPath;
    const BuiltinMethodDef GetClosestPointOnNavVolume;
    const BuiltinMethodDef GetRandomPointOnNavVolume;
    const BuiltinMethodDef GetInterestPoolValue;
    const BuiltinMethodDef GetEventPointOfInterest;
    const BuiltinMethodDef GetCurrentEventType;
    const BuiltinMethodDef GetCurrentEventTypeName;
    const BuiltinMethodDef GetCurrentEventId;
    const BuiltinMethodDef GetCurrentEventName;
    const BuiltinMethodDef GetCurrentEventOriginator;
    const BuiltinMethodDef AddToInterestPool;
    const BuiltinMethodDef GetInterestPoolAwareness;
    const BuiltinMethodDef PropagateCurrentEvent;
    const BuiltinMethodDef GenerateScriptEvent;
    const BuiltinMethodDef ServiceEvent;
    const BuiltinMethodDef ServiceEventsInRadius;
    const BuiltinMethodDef SetAvoidanceMask;
    const BuiltinMethodDef SetAvoidanceProperty;
    const BuiltinMethodDef IsEventServiced;
    const BuiltinMethodDef SetVisionSetForPlayer;
    const BuiltinMethodDef UseServerVisionset;
    const BuiltinMethodDef SetInfraredVision;
    const BuiltinMethodDef GetControllerType;
    const BuiltinMethodDef GetActorWeaponOptions;
    const BuiltinMethodDef SetActorWeapon;
    const BuiltinMethodDef ResetFOV;
    const BuiltinMethodDef ClearAnim;
    const BuiltinMethodDef ClearAnimLimited;
    const BuiltinMethodDef SetAnim;
    const BuiltinMethodDef SetFlaggedAnimKnob;
    const BuiltinMethodDef SetFlaggedAnimKnobLimited;
    const BuiltinMethodDef SetFlaggedAnimKnobRestart;
    const BuiltinMethodDef SetFlaggedAnimKnobLimitedRestart;
    const BuiltinMethodDef SetFlaggedAnimKnobAll;
    const BuiltinMethodDef SetFlaggedAnimKnobAllRestart;
    const BuiltinMethodDef SetFlaggedAnim;
    const BuiltinMethodDef SetFlaggedAnimLimited;
    const BuiltinMethodDef SetFlaggedAnimRestart;
    const BuiltinMethodDef SetFlaggedAnimLimitedRestart;
    const BuiltinMethodDef SetAnimKnob;
    const BuiltinMethodDef SetAnimKnobLimited;
    const BuiltinMethodDef SetAnimKnobRestart;
    const BuiltinMethodDef SetAnimKnobLimitedRestart;
    const BuiltinMethodDef SetAnimKnobAll;
    const BuiltinMethodDef SetAnimKnobAllLimited;
    const BuiltinMethodDef SetAnimKnobAllRestart;
    const BuiltinMethodDef SetAnimKnobAllLimitedRestart;
    const BuiltinMethodDef SetAnimForceNew;
    const BuiltinMethodDef MagicGrenade;
    const BuiltinMethodDef MagicGrenadeManual;
    const BuiltinMethodDef MagicGrenadeManualPlayer;
    const BuiltinMethodDef GetAimAngles;
    const BuiltinMethodDef GetTagOrigin;
    const BuiltinMethodDef GetTagAngles;
    const BuiltinMethodDef GetShootAtPos;
    const BuiltinMethodDef MakeFakeAI;
    const BuiltinMethodDef MakeSentient;
    const BuiltinMethodDef MakePathfinder;
    const BuiltinMethodDef GetDebugEye;
    const BuiltinMethodDef ASMRequestSubstate;
    const BuiltinMethodDef ASMGetStatus;
    const BuiltinMethodDef ASMGetCurrentDeltaAnimation;
    const BuiltinMethodDef ASMIsTerminating;
    const BuiltinMethodDef ASMIsTransitionRunning;
    const BuiltinMethodDef ASMIsSubStatePending;
    const BuiltinMethodDef ASMIsTransDecRunning;
    const BuiltinMethodDef AnimMappingSearch;
    const BuiltinMethodDef ASTSearch;
    const BuiltinMethodDef ASMChangeAnimMappingTable;
    const BuiltinMethodDef ASMSetAnimationRate;
    const BuiltinMethodDef ActorIKEnabled;
    const BuiltinMethodDef LookAtEntity;
    const BuiltinMethodDef LookAtPos;
    const BuiltinMethodDef AimAtPosIK;
    const BuiltinMethodDef AimAtEntityIK;
    const BuiltinMethodDef GadgetFlickering;
    const BuiltinMethodDef GadgetCharging;
    const BuiltinMethodDef GadgetPowerChange;
    const BuiltinMethodDef GadgetPowerSet;
    const BuiltinMethodDef GadgetPowerGet;
    const BuiltinMethodDef GadgetIsActive;
    const BuiltinMethodDef GadgetIsPrimed;
    const BuiltinMethodDef GadgetIsReady;
    const BuiltinMethodDef GadgetSetEntity;
    const BuiltinMethodDef GadgetSetActivateTime;
    const BuiltinMethodDef GadgetPowerReset;
    const BuiltinMethodDef GadgetGetSlot;
    const BuiltinMethodDef GadgetStateChange;
    const BuiltinMethodDef GadgetActivate;
    const BuiltinMethodDef GadgetDeactivate;
    const BuiltinMethodDef GadgetTargetResult;
    const BuiltinMethodDef FlashbackFinish;
    const BuiltinMethodDef FlashbackStart;
    const BuiltinMethodDef StartResurrectViewAngleTransition;
    const BuiltinMethodDef IsGadgetMeleeCharging;
    const BuiltinMethodDef SetTargetEntity;
    const BuiltinMethodDef SetTargetOrigin;
    const BuiltinMethodDef GetTargetOrigin;
    const BuiltinMethodDef GetTargetEntity;
    const BuiltinMethodDef SetOnTargetAngle;
    const BuiltinMethodDef GetEnemies;
    const BuiltinMethodDef SetScale;
    const BuiltinMethodDef SetTmodeProvider;
    const BuiltinMethodDef TmodeSetServerUser;
    const BuiltinMethodDef GetMeleeChainCount;
    const BuiltinMethodDef OverrideSpawnflags;
    const BuiltinMethodDef ExperimentsGetVariant;
  };
  const BuiltinMethodDef methods[218];

  static constexpr const char *names[218] = {
      "Attach",
      "Detach",
      "DetachAll",
      "GetAttachSize",
      "GetAttachModelName",
      "GetAttachTagName",
      "GetAttachIgnoreCollision",
      "HidePart",
      "HasPart",
      "ShowPart",
      "ShowAllParts",
      "PlaySoundToPlayer",
      "PlaySoundToAllButPlayer",
      "AllowedStances",
      "AreAllMissionsAtScore",
      "IsStartingClassDefault",
      "GetNumChallengesComplete",
      "HasCompletedAllGameChallenges",
      "HasCollectedAllCollectibles",
      "GetDStat",
      "GetDStatArrayCount",
      "SetAARStat",
      "SetDStat",
      "AddDStat",
      "SetNonCheckpointData",
      "GetNonCheckpointData",
      "ClearAllNonCheckpointData",
      "ClearNonCheckpointData",
      "AddWeaponStat",
      "AddSpecialistStat",
      "IncrementSpecificWeaponPickedUpCount",
      "AddBonusCardStat",
      "GetSessStat",
      "GetSessStatArrayCount",
      "SetSessStat",
      "AddSessStat",
      "UnlockSongByAlias",
      "CheckIfSongUnlocked",
      "GiveMissingUnlockTokens",
      "UpdateUnlockedAttachmentBits",
      "GetDecorations",
      "GiveDecoration",
      "AddPlayerStat",
      "AddPlayerStatWithGameType",
      "AddGameTypeStat",
      "AddMissionStat",
      "GetCurrentGunRank",
      "TrackHeroPowerAvailable",
      "TrackHeroPowerActivated",
      "TrackHeroPowerExpired",
      "TrackWeaponFireNative",
      "UpdateStatRatio",
      "AddUnlockToken_Weapon",
      "AddUnlockToken_Attachment",
      "GiveUnlockToken",
      "GetXPScale",
      "GetCryptoScale",
      "GetVialsScale",
      "AddRankXP",
      "AddRankXPValue",
      "DoubleXPTimerFired",
      "ReportLootReward",
      "ReportLootConsume",
      "__protected__NoteLootConsume",
      "__protected__ReportNotedLoot",
      "HasSeasonPass",
      "IsStarterPack",
      "SetEMPJammed",
      "IsEMPJammed",
      "SetGrapplableType",
      "LUINotifyEvent",
      "Launch",
      "SetMovingPlatformEnabled",
      "IsMovingPlatform",
      "SetDrawInfrared",
      "SetAssassinationEnabled",
      "GameHistoryStartMatch",
      "GameHistoryFinishMatch",
      "Unlink",
      "LinkTo",
      "LinkToBlendToTag",
      "EnableLinkTo",
      "LinkToUpdateOffset",
      "PlayerLinkTo",
      "PlayerLinkToBlend",
      "PlayerLinkToDelta",
      "PlayerLinkWeaponViewToDelta",
      "PlayerLinkToAbsolute",
      "PlayerCamLinkTo",
      "PlayerCamUnlink",
      "LerpViewAngleClamp",
      "SetViewAngleResistance",
      "PlayerLinkedOffsetEnable",
      "PlayerLinkedOffsetDisable",
      "PlayerLinkedSetUseBaseAngleForViewClamp",
      "ClearGroundEnt",
      "DontInterpolate",
      "MagicGrenadeType",
      "StartFadingBlur",
      "LocalToWorldCoords",
      "WorldToLocalCoords",
      "SetTurretSpinning",
      "AttachShieldModel",
      "DetachShieldModel",
      "SetScriptMoverFlag",
      "RefreshShieldAttachment",
      "MarkDestructibleDestroyed",
      "ResetDestructible",
      "FindPath",
      "CanPath",
      "GetNodeIndexOnPath",
      "GetNextTraversalNodeOnPath",
      "GetClosestPointOnNavVolume",
      "GetRandomPointOnNavVolume",
      "GetInterestPoolValue",
      "GetEventPointOfInterest",
      "GetCurrentEventType",
      "GetCurrentEventTypeName",
      "GetCurrentEventId",
      "GetCurrentEventName",
      "GetCurrentEventOriginator",
      "AddToInterestPool",
      "GetInterestPoolAwareness",
      "PropagateCurrentEvent",
      "GenerateScriptEvent",
      "ServiceEvent",
      "ServiceEventsInRadius",
      "SetAvoidanceMask",
      "SetAvoidanceProperty",
      "IsEventServiced",
      "SetVisionSetForPlayer",
      "UseServerVisionset",
      "SetInfraredVision",
      "GetControllerType",
      "GetActorWeaponOptions",
      "SetActorWeapon",
      "ResetFOV",
      "ClearAnim",
      "ClearAnimLimited",
      "SetAnim",
      "SetFlaggedAnimKnob",
      "SetFlaggedAnimKnobLimited",
      "SetFlaggedAnimKnobRestart",
      "SetFlaggedAnimKnobLimitedRestart",
      "SetFlaggedAnimKnobAll",
      "SetFlaggedAnimKnobAllRestart",
      "SetFlaggedAnim",
      "SetFlaggedAnimLimited",
      "SetFlaggedAnimRestart",
      "SetFlaggedAnimLimitedRestart",
      "SetAnimKnob",
      "SetAnimKnobLimited",
      "SetAnimKnobRestart",
      "SetAnimKnobLimitedRestart",
      "SetAnimKnobAll",
      "SetAnimKnobAllLimited",
      "SetAnimKnobAllRestart",
      "SetAnimKnobAllLimitedRestart",
      "SetAnimForceNew",
      "MagicGrenade",
      "MagicGrenadeManual",
      "MagicGrenadeManualPlayer",
      "GetAimAngles",
      "GetTagOrigin",
      "GetTagAngles",
      "GetShootAtPos",
      "MakeFakeAI",
      "MakeSentient",
      "MakePathfinder",
      "GetDebugEye",
      "ASMRequestSubstate",
      "ASMGetStatus",
      "ASMGetCurrentDeltaAnimation",
      "ASMIsTerminating",
      "ASMIsTransitionRunning",
      "ASMIsSubStatePending",
      "ASMIsTransDecRunning",
      "AnimMappingSearch",
      "ASTSearch",
      "ASMChangeAnimMappingTable",
      "ASMSetAnimationRate",
      "ActorIKEnabled",
      "LookAtEntity",
      "LookAtPos",
      "AimAtPosIK",
      "AimAtEntityIK",
      "GadgetFlickering",
      "GadgetCharging",
      "GadgetPowerChange",
      "GadgetPowerSet",
      "GadgetPowerGet",
      "GadgetIsActive",
      "GadgetIsPrimed",
      "GadgetIsReady",
      "GadgetSetEntity",
      "GadgetSetActivateTime",
      "GadgetPowerReset",
      "GadgetGetSlot",
      "GadgetStateChange",
      "GadgetActivate",
      "GadgetDeactivate",
      "GadgetTargetResult",
      "FlashbackFinish",
      "FlashbackStart",
      "StartResurrectViewAngleTransition",
      "IsGadgetMeleeCharging",
      "SetTargetEntity",
      "SetTargetOrigin",
      "GetTargetOrigin",
      "GetTargetEntity",
      "SetOnTargetAngle",
      "GetEnemies",
      "SetScale",
      "SetTmodeProvider",
      "TmodeSetServerUser",
      "GetMeleeChainCount",
      "OverrideSpawnflags",
      "ExperimentsGetVariant",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(CommonMethodTable, sizeof(CommonMethodTable::methods));

union HelicopterMethodTable {
  struct {
    const BuiltinMethodDef SetDamageStage;
    const BuiltinMethodDef SetHeliHeightLock;
    const BuiltinMethodDef SetHeliHeightCap;
    const BuiltinMethodDef GetHeliHeightLock;
    const BuiltinMethodDef IsInsideHeliHeightLock;
    const BuiltinMethodDef GetHeliHeightLockHeight;
    const BuiltinMethodDef SetHeliDogfighting;
  };
  const BuiltinMethodDef methods[7];

  static constexpr const char *names[7] = {
      "SetDamageStage",     "SetHeliHeightLock",      "SetHeliHeightCap",
      "GetHeliHeightLock",  "IsInsideHeliHeightLock", "GetHeliHeightLockHeight",
      "SetHeliDogfighting",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(HelicopterMethodTable, sizeof(HelicopterMethodTable::methods));

union HudElemMethodTable {
  struct {
    const BuiltinMethodDef SetText;
    const BuiltinMethodDef SetShader;
    const BuiltinMethodDef SetTargetEnt;
    const BuiltinMethodDef ClearTargetEnt;
    const BuiltinMethodDef SetTimer;
    const BuiltinMethodDef SetTimerUp;
    const BuiltinMethodDef SetTenthsTimer;
    const BuiltinMethodDef SetTenthsTimerUp;
    const BuiltinMethodDef SetClock;
    const BuiltinMethodDef SetClockUp;
    const BuiltinMethodDef SetValue;
    const BuiltinMethodDef SetWayPoint;
    const BuiltinMethodDef FadeOverTime;
    const BuiltinMethodDef ScaleOverTime;
    const BuiltinMethodDef MoveOverTime;
    const BuiltinMethodDef Reset;
    const BuiltinMethodDef Destroy;
    const BuiltinMethodDef SetPulseFX;
    const BuiltinMethodDef SetCOD7DecodeFX;
    const BuiltinMethodDef SetRedactFX;
    const BuiltinMethodDef SetTypewriterFX;
    const BuiltinMethodDef GetTextWidth;
    const BuiltinMethodDef SetPerks;
    const BuiltinMethodDef SetPlayerNameString;
    const BuiltinMethodDef SetMapNameString;
    const BuiltinMethodDef SetGameTypeString;
    const BuiltinMethodDef SetWarGameData;
    const BuiltinMethodDef ChangeFontScaleOverTime;
  };
  const BuiltinMethodDef methods[28];

  static constexpr const char *names[28] = {
      "SetText",          "SetShader",
      "SetTargetEnt",     "ClearTargetEnt",
      "SetTimer",         "SetTimerUp",
      "SetTenthsTimer",   "SetTenthsTimerUp",
      "SetClock",         "SetClockUp",
      "SetValue",         "SetWayPoint",
      "FadeOverTime",     "ScaleOverTime",
      "MoveOverTime",     "Reset",
      "Destroy",          "SetPulseFX",
      "SetCOD7DecodeFX",  "SetRedactFX",
      "SetTypewriterFX",  "GetTextWidth",
      "SetPerks",         "SetPlayerNameString",
      "SetMapNameString", "SetGameTypeString",
      "SetWarGameData",   "ChangeFontScaleOverTime",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(HudElemMethodTable, sizeof(HudElemMethodTable::methods));

union MathFunctionTable {
  struct {
    const BuiltinFunctionDef RandomInt;
    const BuiltinFunctionDef RandomFloat;
    const BuiltinFunctionDef RandomIntRange;
    const BuiltinFunctionDef RandomFloatRange;
  };
  const BuiltinFunctionDef functions[4];

  static constexpr const char *names[4] = {
      "RandomInt",
      "RandomFloat",
      "RandomIntRange",
      "RandomFloatRange",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(MathFunctionTable, sizeof(MathFunctionTable::functions));

union ScriptEntMethodTable {
  struct {
    const BuiltinMethodDef MoveTo;
    const BuiltinMethodDef MoveX;
    const BuiltinMethodDef MoveY;
    const BuiltinMethodDef MoveZ;
    const BuiltinMethodDef MoveGravity;
    const BuiltinMethodDef MoveSlide;
    const BuiltinMethodDef StopMoveSlide;
    const BuiltinMethodDef RotateTo;
    const BuiltinMethodDef RotatePitch;
    const BuiltinMethodDef RotateYaw;
    const BuiltinMethodDef RotateRoll;
    const BuiltinMethodDef Vibrate;
    const BuiltinMethodDef Bobbing;
    const BuiltinMethodDef Rotate;
    const BuiltinMethodDef RotateVelocity;
    const BuiltinMethodDef SetCanDamage;
    const BuiltinMethodDef PhysicsLaunch;
  };
  const BuiltinMethodDef methods[17];

  static constexpr const char *names[17] = {
      "MoveTo",        "MoveX",     "MoveY",          "MoveZ",
      "MoveGravity",   "MoveSlide", "StopMoveSlide",  "RotateTo",
      "RotatePitch",   "RotateYaw", "RotateRoll",     "Vibrate",
      "Bobbing",       "Rotate",    "RotateVelocity", "SetCanDamage",
      "PhysicsLaunch",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(ScriptEntMethodTable, sizeof(ScriptEntMethodTable::methods));

union ScriptVehicleMethodTable {
  struct {
    const BuiltinMethodDef AttachPath;
    const BuiltinMethodDef GetAttachPos;
    const BuiltinMethodDef StartPath;
    const BuiltinMethodDef DrivePath;
    const BuiltinMethodDef PathMove;
    const BuiltinMethodDef PathMoveClear;
    const BuiltinMethodDef Flip180;
    const BuiltinMethodDef PathFixedOffset;
    const BuiltinMethodDef PathFixedOffsetClear;
    const BuiltinMethodDef PathVariableOffset;
    const BuiltinMethodDef PathVariableOffsetClear;
    const BuiltinMethodDef GetPathFixedOffset;
    const BuiltinMethodDef GetPathVariableOffset;
    const BuiltinMethodDef SetDrivePathPhysicsScale;
    const BuiltinMethodDef SetSwitchNode;
    const BuiltinMethodDef SwitchStartNode;
    const BuiltinMethodDef RecalcSplinePaths;
    const BuiltinMethodDef SetSpeed;
    const BuiltinMethodDef SetSpeedImmediate;
    const BuiltinMethodDef GetSpeed;
    const BuiltinMethodDef GetSpeedMPH;
    const BuiltinMethodDef GetGoalSpeedMPH;
    const BuiltinMethodDef SetBrake;
    const BuiltinMethodDef GetBrake;
    const BuiltinMethodDef GetHandBrake;
    const BuiltinMethodDef GetScriptBrake;
    const BuiltinMethodDef GetDefaultAcceleration;
    const BuiltinMethodDef SetAcceleration;
    const BuiltinMethodDef SetDeceleration;
    const BuiltinMethodDef SetMaxSpeedScale;
    const BuiltinMethodDef SetMaxAccelerationScale;
    const BuiltinMethodDef ResumeSpeed;
    const BuiltinMethodDef SetYawSpeed;
    const BuiltinMethodDef SetMaxPitchRoll;
    const BuiltinMethodDef SetTurningAbility;
    const BuiltinMethodDef SetJitterParams;
    const BuiltinMethodDef SetAirResistance;
    const BuiltinMethodDef GetSteering;
    const BuiltinMethodDef GetThrottle;
    const BuiltinMethodDef SetPhysAngles;
    const BuiltinMethodDef SetHoverParams;
    const BuiltinMethodDef JoltBody;
    const BuiltinMethodDef FreeVehicle;
    const BuiltinMethodDef GetWheelSurface;
    const BuiltinMethodDef GetVehicleOwner;
    const BuiltinMethodDef IsRemoteControl;
    const BuiltinMethodDef IsVehicleUsable;
    const BuiltinMethodDef MakeVehicleUsable;
    const BuiltinMethodDef MakeVehicleUnusable;
    const BuiltinMethodDef UseVehicle;
    const BuiltinMethodDef IsVehicleSeatOccupied;
    const BuiltinMethodDef SetNearGoalNotifyDist;
    const BuiltinMethodDef SetVehGoalPos;
    const BuiltinMethodDef ClearVehGoalPos;
    const BuiltinMethodDef SetPlaneGoalPos;
    const BuiltinMethodDef SetPlaneBarrelRoll;
    const BuiltinMethodDef SetGoalYaw;
    const BuiltinMethodDef GetGoalYaw;
    const BuiltinMethodDef ClearGoalYaw;
    const BuiltinMethodDef SetTargetYaw;
    const BuiltinMethodDef ClearTargetYaw;
    const BuiltinMethodDef SetMinDesiredTurnYaw;
    const BuiltinMethodDef SetLookAtEnt;
    const BuiltinMethodDef SetLookAtOrigin;
    const BuiltinMethodDef ClearLookAtEnt;
    const BuiltinMethodDef SetVehicleForDropDeploy;
    const BuiltinMethodDef ResetVehicleFromDropDeploy;
    const BuiltinMethodDef TakePlayerControl;
    const BuiltinMethodDef ReturnPlayerControl;
    const BuiltinMethodDef SetTurretTargetVec;
    const BuiltinMethodDef SetTurretTargetEnt;
    const BuiltinMethodDef SetTurretTargetRelativeAngles;
    const BuiltinMethodDef GetTurretLimitsYaw;
    const BuiltinMethodDef ClearTurretTarget;
    const BuiltinMethodDef SetGunnerTargetVec;
    const BuiltinMethodDef GetTurretTargetVec;
    const BuiltinMethodDef GetGunnerTargetVec;
    const BuiltinMethodDef SetGunnerTargetEnt;
    const BuiltinMethodDef ClearGunnerTarget;
    const BuiltinMethodDef SetGunnerTurretOnTargetRange;
    const BuiltinMethodDef GetGunnerTargetEnt;
    const BuiltinMethodDef SetVehWeapon;
    const BuiltinMethodDef FireWeapon;
    const BuiltinMethodDef StopFireWeapon;
    const BuiltinMethodDef IsTurretReady;
    const BuiltinMethodDef SetVehMaxSpeed;
    const BuiltinMethodDef GetVehOccupants;
    const BuiltinMethodDef GetSeatOccupant;
    const BuiltinMethodDef GetOccupantSeat;
    const BuiltinMethodDef SetSeatOccupied;
    const BuiltinMethodDef SeatGetWeapon;
    const BuiltinMethodDef GetSeatFiringOrigin;
    const BuiltinMethodDef GetSeatFiringAngles;
    const BuiltinMethodDef IsGunnerFiring;
    const BuiltinMethodDef DisableGunnerFiring;
    const BuiltinMethodDef IsDriverFiring;
    const BuiltinMethodDef DisableDriverFiring;
    const BuiltinMethodDef FinishVehicleDamage;
    const BuiltinMethodDef FinishVehicleRadiusDamage;
    const BuiltinMethodDef IsVehicleImmuneToDamage;
    const BuiltinMethodDef SetDefaultPitch;
    const BuiltinMethodDef ClearDefaultPitch;
    const BuiltinMethodDef GetAngularVelocity;
    const BuiltinMethodDef SetAngularVelocity;
    const BuiltinMethodDef SetVehVelocity;
    const BuiltinMethodDef CancelAIMove;
    const BuiltinMethodDef SetVehicleType;
    const BuiltinMethodDef GetPhysAcceleration;
    const BuiltinMethodDef SetPhysAcceleration;
    const BuiltinMethodDef DrawTrajectory;
    const BuiltinMethodDef SetRotorSpeed;
    const BuiltinMethodDef GetMaxSpeed;
    const BuiltinMethodDef SetBuoyancyOffset;
    const BuiltinMethodDef SetMantleEnabled;
    const BuiltinMethodDef SetVehicleAvoidance;
    const BuiltinMethodDef GetVehicleAvoidance;
    const BuiltinMethodDef VehCanSee;
    const BuiltinMethodDef VehSeenRecently;
    const BuiltinMethodDef VehClearEnemy;
    const BuiltinMethodDef GetTurretHeatValue;
    const BuiltinMethodDef IsVehicleTurretOverheating;
    const BuiltinMethodDef SetPathTransitionTime;
    const BuiltinMethodDef GetVehicleAvoidanceNodes;
    const BuiltinMethodDef SetVehicleManualControl;
    const BuiltinMethodDef IsVehicleManualControl;
    const BuiltinMethodDef VehDriverAimAtCrosshairs;
    const BuiltinMethodDef GetVehDamageMultiplier;
  };
  const BuiltinMethodDef methods[127];

  static constexpr const char *names[127] = {
      "AttachPath",
      "GetAttachPos",
      "StartPath",
      "DrivePath",
      "PathMove",
      "PathMoveClear",
      "Flip180",
      "PathFixedOffset",
      "PathFixedOffsetClear",
      "PathVariableOffset",
      "PathVariableOffsetClear",
      "GetPathFixedOffset",
      "GetPathVariableOffset",
      "SetDrivePathPhysicsScale",
      "SetSwitchNode",
      "SwitchStartNode",
      "RecalcSplinePaths",
      "SetSpeed",
      "SetSpeedImmediate",
      "GetSpeed",
      "GetSpeedMPH",
      "GetGoalSpeedMPH",
      "SetBrake",
      "GetBrake",
      "GetHandBrake",
      "GetScriptBrake",
      "GetDefaultAcceleration",
      "SetAcceleration",
      "SetDeceleration",
      "SetMaxSpeedScale",
      "SetMaxAccelerationScale",
      "ResumeSpeed",
      "SetYawSpeed",
      "SetMaxPitchRoll",
      "SetTurningAbility",
      "SetJitterParams",
      "SetAirResistance",
      "GetSteering",
      "GetThrottle",
      "SetPhysAngles",
      "SetHoverParams",
      "JoltBody",
      "FreeVehicle",
      "GetWheelSurface",
      "GetVehicleOwner",
      "IsRemoteControl",
      "IsVehicleUsable",
      "MakeVehicleUsable",
      "MakeVehicleUnusable",
      "UseVehicle",
      "IsVehicleSeatOccupied",
      "SetNearGoalNotifyDist",
      "SetVehGoalPos",
      "ClearVehGoalPos",
      "SetPlaneGoalPos",
      "SetPlaneBarrelRoll",
      "SetGoalYaw",
      "GetGoalYaw",
      "ClearGoalYaw",
      "SetTargetYaw",
      "ClearTargetYaw",
      "SetMinDesiredTurnYaw",
      "SetLookAtEnt",
      "SetLookAtOrigin",
      "ClearLookAtEnt",
      "SetVehicleForDropDeploy",
      "ResetVehicleFromDropDeploy",
      "TakePlayerControl",
      "ReturnPlayerControl",
      "SetTurretTargetVec",
      "SetTurretTargetEnt",
      "SetTurretTargetRelativeAngles",
      "GetTurretLimitsYaw",
      "ClearTurretTarget",
      "SetGunnerTargetVec",
      "GetTurretTargetVec",
      "GetGunnerTargetVec",
      "SetGunnerTargetEnt",
      "ClearGunnerTarget",
      "SetGunnerTurretOnTargetRange",
      "GetGunnerTargetEnt",
      "SetVehWeapon",
      "FireWeapon",
      "StopFireWeapon",
      "IsTurretReady",
      "SetVehMaxSpeed",
      "GetVehOccupants",
      "GetSeatOccupant",
      "GetOccupantSeat",
      "SetSeatOccupied",
      "SeatGetWeapon",
      "GetSeatFiringOrigin",
      "GetSeatFiringAngles",
      "IsGunnerFiring",
      "DisableGunnerFiring",
      "IsDriverFiring",
      "DisableDriverFiring",
      "FinishVehicleDamage",
      "FinishVehicleRadiusDamage",
      "IsVehicleImmuneToDamage",
      "SetDefaultPitch",
      "ClearDefaultPitch",
      "GetAngularVelocity",
      "SetAngularVelocity",
      "SetVehVelocity",
      "CancelAIMove",
      "SetVehicleType",
      "GetPhysAcceleration",
      "SetPhysAcceleration",
      "DrawTrajectory",
      "SetRotorSpeed",
      "GetMaxSpeed",
      "SetBuoyancyOffset",
      "SetMantleEnabled",
      "SetVehicleAvoidance",
      "GetVehicleAvoidance",
      "VehCanSee",
      "VehSeenRecently",
      "VehClearEnemy",
      "GetTurretHeatValue",
      "IsVehicleTurretOverheating",
      "SetPathTransitionTime",
      "GetVehicleAvoidanceNodes",
      "SetVehicleManualControl",
      "IsVehicleManualControl",
      "VehDriverAimAtCrosshairs",
      "GetVehDamageMultiplier",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(ScriptVehicleMethodTable,
            sizeof(ScriptVehicleMethodTable::methods));

union SentientFunctionTable {
  struct {
    const BuiltinFunctionDef CreateThreatBiasGroup;
    const BuiltinFunctionDef ThreatBiasGroupExists;
    const BuiltinFunctionDef GetThreatBias;
    const BuiltinFunctionDef SetThreatBias;
    const BuiltinFunctionDef SetThreatBiasAgainstAll;
    const BuiltinFunctionDef SetIgnoreMeGroup;
    const BuiltinFunctionDef SetStealthSigh;
    const BuiltinFunctionDef ResetVisibilityCacheWithinRadius;
  };
  const BuiltinFunctionDef functions[8];

  static constexpr const char *names[8] = {
      "CreateThreatBiasGroup",   "ThreatBiasGroupExists",
      "GetThreatBias",           "SetThreatBias",
      "SetThreatBiasAgainstAll", "SetIgnoreMeGroup",
      "SetStealthSigh",          "ResetVisibilityCacheWithinRadius",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(SentientFunctionTable, sizeof(SentientFunctionTable::functions));

union SentientMethodTable {
  struct {
    const BuiltinMethodDef GetClosestEnemySqDist;
    const BuiltinMethodDef SetThreatBiasGroup;
    const BuiltinMethodDef GetThreatBiasGroup;
    const BuiltinMethodDef SetPersonalThreatBias;
    const BuiltinMethodDef GetPersonalThreatBias;
    const BuiltinMethodDef SetPersonalIgnore;
    const BuiltinMethodDef IsNoTarget;
    const BuiltinMethodDef IsBadGuy;
    const BuiltinMethodDef SetEntityTarget;
    const BuiltinMethodDef ClearEntityTarget;
    const BuiltinMethodDef GetPerfectInfo;
    const BuiltinMethodDef IsPosInClaimedLocation;
    const BuiltinMethodDef SetGoal;
    const BuiltinMethodDef ClearForcedGoal;
    const BuiltinMethodDef SetGoalNode;
    const BuiltinMethodDef SetGoalPos;
    const BuiltinMethodDef SetGoalEntity;
    const BuiltinMethodDef SetGoalVolume;
    const BuiltinMethodDef SetGoalVolumeAuto;
    const BuiltinMethodDef GetGoalVolume;
    const BuiltinMethodDef ClearGoalVolume;
    const BuiltinMethodDef IsInGoal;
    const BuiltinMethodDef AttackedRecently;
    const BuiltinMethodDef SetIgnoreEnt;
    const BuiltinMethodDef GetIgnoreEnt;
    const BuiltinMethodDef AddSentientEventListener;
    const BuiltinMethodDef RemoveSentientEventListener;
    const BuiltinMethodDef GetStealthSightValue;
    const BuiltinMethodDef SetStealthSightValue;
    const BuiltinMethodDef SetStealthSightAwareness;
    const BuiltinMethodDef GetMotionAngle;
  };
  const BuiltinMethodDef methods[31];

  static constexpr const char *names[31] = {
      "GetClosestEnemySqDist",
      "SetThreatBiasGroup",
      "GetThreatBiasGroup",
      "SetPersonalThreatBias",
      "GetPersonalThreatBias",
      "SetPersonalIgnore",
      "IsNoTarget",
      "IsBadGuy",
      "SetEntityTarget",
      "ClearEntityTarget",
      "GetPerfectInfo",
      "IsPosInClaimedLocation",
      "SetGoal",
      "ClearForcedGoal",
      "SetGoalNode",
      "SetGoalPos",
      "SetGoalEntity",
      "SetGoalVolume",
      "SetGoalVolumeAuto",
      "GetGoalVolume",
      "ClearGoalVolume",
      "IsInGoal",
      "AttackedRecently",
      "SetIgnoreEnt",
      "GetIgnoreEnt",
      "AddSentientEventListener",
      "RemoveSentientEventListener",
      "GetStealthSightValue",
      "SetStealthSightValue",
      "SetStealthSightAwareness",
      "GetMotionAngle",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(SentientMethodTable, sizeof(SentientMethodTable::methods));

union UtilFunctionTable {
  struct {
    const BuiltinFunctionDef SetSharedViewPort;
    const BuiltinFunctionDef SetTopDownCameraYaw;
    const BuiltinFunctionDef SetJumpHeight;
  };
  const BuiltinFunctionDef functions[3];

  static constexpr const char *names[3] = {
      "SetSharedViewPort",
      "SetTopDownCameraYaw",
      "SetJumpHeight",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(UtilFunctionTable, sizeof(UtilFunctionTable::functions));

} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game