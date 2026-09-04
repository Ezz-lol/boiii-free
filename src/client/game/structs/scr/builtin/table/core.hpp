#pragma once

#include <game/structs/scr/builtin/core.hpp>
#include <game/structs/scr/builtin/table/macros.hpp>

namespace game {
namespace scr {
namespace builtin {
namespace table {

union ActorInterfaceMethodTable {
  struct {
    BuiltinMethodDef ChooseBetterCoverNode;
    BuiltinMethodDef GetFireMode;
    BuiltinMethodDef BB_GetArrivalStance;
    BuiltinMethodDef BB_GetCurrentExposedType;
    BuiltinMethodDef BB_GetLocomotionArrivalDistance;
    BuiltinMethodDef BB_GetLocomotionArrivalYaw;
    BuiltinMethodDef BB_GetLocomotionMotionAngle;
    BuiltinMethodDef BB_GetLocomotionTurnYaw;
  };

  static inline constexpr size_t COUNT = 8;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "ChooseBetterCoverNode",
      "GetFireMode",
      "BB_GetArrivalStance",
      "BB_GetCurrentExposedType",
      "BB_GetLocomotionArrivalDistance",
      "BB_GetLocomotionArrivalYaw",
      "BB_GetLocomotionMotionAngle",
      "BB_GetLocomotionTurnYaw",
  };

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(ActorInterfaceMethodTable,
            sizeof(ActorInterfaceMethodTable::methods));

union ActorMethodTable {
  struct {
    BuiltinMethodDef StartCoverArrival;
    BuiltinMethodDef Melee;
    BuiltinMethodDef ReacquireStep;
    BuiltinMethodDef FindReacquireNode;
    BuiltinMethodDef GetReacquireNode;
    BuiltinMethodDef UseReacquireNode;
    BuiltinMethodDef FindReacquireDirectPath;
    BuiltinMethodDef TrimPathToAttack;
    BuiltinMethodDef ReacquireMove;
    BuiltinMethodDef FindReacquireProximatePath;
    BuiltinMethodDef FlagEnemyUnattackable;
    BuiltinMethodDef ClearPitchOrient;
    BuiltinMethodDef SetPitchOrient;
    BuiltinMethodDef CanSee;
    BuiltinMethodDef SeeRecently;
    BuiltinMethodDef LastKnownTime;
    BuiltinMethodDef LastKnownPos;
    BuiltinMethodDef MayMoveToPoint;
    BuiltinMethodDef MayMoveFromPointToPoint;
    BuiltinMethodDef Teleport;
    BuiltinMethodDef IsPathDirect;
    BuiltinMethodDef IsStanceAllowed;
    BuiltinMethodDef TraverseMode;
    BuiltinMethodDef AnimMode;
    BuiltinMethodDef OrientMode;
    BuiltinMethodDef GetOrientMode;
    BuiltinMethodDef ShouldFaceMotion;
    BuiltinMethodDef GetAnglesToLikelyEnemyPath;
    BuiltinMethodDef LerpPosition;
    BuiltinMethodDef GetHitEntType;
    BuiltinMethodDef GetHitYaw;
    BuiltinMethodDef GetGroundEntType;
    BuiltinMethodDef IsDeflected;
    BuiltinMethodDef AnimCustom;
    BuiltinMethodDef CanAttackEnemyNode;
    BuiltinMethodDef GetPathLength;
    BuiltinMethodDef CalcPathLength;
    BuiltinMethodDef CheckProne;
    BuiltinMethodDef PushPlayer;
    BuiltinMethodDef PushActors;
    BuiltinMethodDef NearNode;
    BuiltinMethodDef NearClaimNode;
    BuiltinMethodDef NearClaimNodeAndAngle;
    BuiltinMethodDef AtDangerousNode;
    BuiltinMethodDef GetEnemyInfo;
    BuiltinMethodDef ClearEnemy;
    BuiltinMethodDef SetPotentialThreat;
    BuiltinMethodDef ClearPotentialThreat;
    BuiltinMethodDef SetFlashbangImmunity;
    BuiltinMethodDef SetFlashBanged;
    BuiltinMethodDef GetFlashBangedStrength;
    BuiltinMethodDef IsKnownEnemyInRadius;
    BuiltinMethodDef IsKnownEnemyInVolume;
    BuiltinMethodDef SetTalkToSpecies;
    BuiltinMethodDef AllowPitchAngle;
    BuiltinMethodDef KnockBack;
    BuiltinMethodDef GetDeltaTurnYaw;
    BuiltinMethodDef SetRepairPaths;
    BuiltinMethodDef FinishActorDamage;
    BuiltinMethodDef CalcLookaheadPos;
    BuiltinMethodDef IsInScriptedState;
    BuiltinMethodDef IsActorShooting;
    BuiltinMethodDef MeleeWithOffset;
    BuiltinMethodDef DropWeapon;
    BuiltinMethodDef Shoot;
    BuiltinMethodDef SetFixedNodeSafeVolume;
    BuiltinMethodDef GetFixedNodeSafeVolume;
    BuiltinMethodDef ClearFixedNodeSafeVolume;
    BuiltinMethodDef UsePosition;
    BuiltinMethodDef ClearUsePosition;
    BuiltinMethodDef GetNodeOffsetPosition;
    BuiltinMethodDef GetNodeOffsetAngles;
    BuiltinMethodDef UseCoverNode;
    BuiltinMethodDef FindBestCoverNode;
    BuiltinMethodDef FindBestCoverNodes;
    BuiltinMethodDef FindCoverNodeAtLocation;
    BuiltinMethodDef GetDropToFloorPosition;
    BuiltinMethodDef IsSuppressionWaiting;
    BuiltinMethodDef IsSuppressed;
    BuiltinMethodDef IsMoveSuppressed;
    BuiltinMethodDef ForceTeleport;
    BuiltinMethodDef SetEntityOwner;
    BuiltinMethodDef ClearEntityOwner;
    BuiltinMethodDef AIthrowgrenade;
    BuiltinMethodDef GrenadeThrow;
    BuiltinMethodDef IsGrenadePosSafe;
    BuiltinMethodDef CheckGrenadeThrow;
    BuiltinMethodDef IsSafeFromGrenade;
    BuiltinMethodDef CanShoot;
    BuiltinMethodDef CanShootEnemy;
    BuiltinMethodDef CheckGrenadeThrowPos;
    BuiltinMethodDef CanThrowGrenade;
    BuiltinMethodDef CanThrowGrenadePos;
    BuiltinMethodDef PickUpGrenade;
    BuiltinMethodDef HasValidInterrupt;
    BuiltinMethodDef GetBehaviorTreeStatus;
    BuiltinMethodDef IsCurrentBTActionLooping;
    BuiltinMethodDef ShouldHoldGroundAgainstEnemy;
    BuiltinMethodDef GetEnemyVelocity;
    BuiltinMethodDef IsAtGoal;
    BuiltinMethodDef IsPosAtGoal;
    BuiltinMethodDef IsApproachingGoal;
    BuiltinMethodDef ShouldUseCoverNode;
    BuiltinMethodDef IsAtCoverNode;
    BuiltinMethodDef IsAtCoverNodeStrict;
    BuiltinMethodDef IsFlankedAtCoverNode;
    BuiltinMethodDef ShouldStartArrival;
    BuiltinMethodDef ShouldStartTraversal;
    BuiltinMethodDef FinishTraversal;
    BuiltinMethodDef SetStairsExitTransform;
    BuiltinMethodDef SetAnimState;
    BuiltinMethodDef SetAnimStateFromASD;
    BuiltinMethodDef HasAnimStateFromASD;
    BuiltinMethodDef GetAnimStateFromASD;
    BuiltinMethodDef GetAnimSubStateFromASD;
    BuiltinMethodDef GetAnimSubStateCountFromASD;
    BuiltinMethodDef GetAnimFromASD;
    BuiltinMethodDef GetAnimLengthFromASD;
    BuiltinMethodDef GetAnimHasNotetrackFromASD;
    BuiltinMethodDef SetPhysParams;
    BuiltinMethodDef SetSteeringMode;
    BuiltinMethodDef PredictPath;
    BuiltinMethodDef PredictArrival;
    BuiltinMethodDef PredictExit;
    BuiltinMethodDef GetTurnExitYaw;
    BuiltinMethodDef GetFinalPathPos;
    BuiltinMethodDef UpdateTrackedBlackboardAttribute;
    BuiltinMethodDef TrackBlackBoardAttribute;
    BuiltinMethodDef FinalizeTrackedBlackBoardAttributes;
    BuiltinMethodDef HasPath;
    BuiltinMethodDef ClearPath;
    BuiltinMethodDef GenerateRadioEvent;
    BuiltinMethodDef GetAimLimitsFromEntry;
    BuiltinMethodDef SetFreeCameraLockOnAllowed;
    BuiltinMethodDef PathMode;
    BuiltinMethodDef GetPathMode;
    BuiltinMethodDef DamageMode;
    BuiltinMethodDef IsCoverValid;
    BuiltinMethodDef IsInAnyBadPlace;
    BuiltinMethodDef CalcApproximatePathToPosition;
    BuiltinMethodDef GetNextFindBestCoverTime;
  };

  static inline constexpr size_t COUNT = 141;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
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

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(ActorMethodTable, sizeof(ActorMethodTable::methods));

union BotMethodTable {
  struct {
    BuiltinMethodDef BotDropClient;
    BuiltinMethodDef BotInLastStand;
    BuiltinMethodDef BotInLowReady;
    BuiltinMethodDef BotGetJetEnergy;
    BuiltinMethodDef BotTakeManualControl;
    BuiltinMethodDef BotReleaseManualControl;
    BuiltinMethodDef BotUnderManualControl;
    BuiltinMethodDef BotSetGoal;
    BuiltinMethodDef BotGetGoalPosition;
    BuiltinMethodDef BotGetGoalRadius;
    BuiltinMethodDef BotGoalSet;
    BuiltinMethodDef BotGoalReached;
    BuiltinMethodDef BotRequestPath;
    BuiltinMethodDef BotCancelPath;
    BuiltinMethodDef BotLookNone;
    BuiltinMethodDef BotLookForward;
    BuiltinMethodDef BotLookAtPoint;
    BuiltinMethodDef BotSetLookAngles;
    BuiltinMethodDef BotSetLookAnglesFromPoint;
    BuiltinMethodDef BotGetFov;
    BuiltinMethodDef BotSetMoveMagnitude;
    BuiltinMethodDef BotSetMoveAngle;
    BuiltinMethodDef BotSetMoveAngleFromPoint;
    BuiltinMethodDef BotTapButton;
    BuiltinMethodDef BotPressButton;
    BuiltinMethodDef BotReleaseButton;
    BuiltinMethodDef BotReleaseButtons;
    BuiltinMethodDef BotPressButtonForGadget;
    BuiltinMethodDef BotSwitchToWeapon;
    BuiltinMethodDef BotGetThreats;
    BuiltinMethodDef BotSightTrace;
    BuiltinMethodDef BotSetDefaultClass;
    BuiltinMethodDef BotClassAddItem;
    BuiltinMethodDef BotClassSetWeaponOption;
    BuiltinMethodDef BotClassAddAttachment;
    BuiltinMethodDef BotToggleGodMode;
    BuiltinMethodDef BotToggleDemigodMode;
    BuiltinMethodDef PredictPosition;
    BuiltinMethodDef LookAt;
    BuiltinMethodDef LookDirection;
    BuiltinMethodDef LookTowards;
    BuiltinMethodDef ClearLookAt;
    BuiltinMethodDef StopLook;
    BuiltinMethodDef MoveTowards;
    BuiltinMethodDef MoveDirection;
    BuiltinMethodDef BotLeaveGame;
    BuiltinMethodDef GetThreats;
    BuiltinMethodDef BotSightTracePassed;
    BuiltinMethodDef PressDTPButton;
    BuiltinMethodDef CancelGoal;
    BuiltinMethodDef BotSetMeleeChargeEnt;
    BuiltinMethodDef BotClearMeleeChargeEnt;
    BuiltinMethodDef BotSetFailSafeNode;
    BuiltinMethodDef ThrowGrenade;
    BuiltinMethodDef AddGoal;
    BuiltinMethodDef AtGoal;
    BuiltinMethodDef HasGoal;
    BuiltinMethodDef GetGoal;
    BuiltinMethodDef PressUseButton;
    BuiltinMethodDef PressAttackButton;
    BuiltinMethodDef PressJumpButton;
    BuiltinMethodDef PressDoubleJumpButton;
    BuiltinMethodDef ReleaseDoubleJumpButton;
    BuiltinMethodDef PressSlideButton;
    BuiltinMethodDef GetLookAheadDist;
    BuiltinMethodDef GetLookAheadDir;
    BuiltinMethodDef AllowAttack;
    BuiltinMethodDef PressAds;
    BuiltinMethodDef PressMelee;
  };

  static inline constexpr size_t COUNT = 69;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
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

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(BotMethodTable, sizeof(BotMethodTable::methods));

union PlayerMethodTable {
  struct {
    BuiltinMethodDef GiveWeapon;
    BuiltinMethodDef HasMaxPrimaryWeapons;
    BuiltinMethodDef UpdateWeaponOptions;
    BuiltinMethodDef SetBlockWeaponPickup;
    BuiltinMethodDef GetInventoryHeldWeapon;
    BuiltinMethodDef SetInventoryHeldWeapon;
    BuiltinMethodDef GetWeaponAcvi;
    BuiltinMethodDef CalcWeaponOptions;
    BuiltinMethodDef CalcPlayerOptions;
    BuiltinMethodDef SetPlayerRenderOptions;
    BuiltinMethodDef NextPlayerRenderOption;
    BuiltinMethodDef BlackboxClassChoice;
    BuiltinMethodDef SetHighlighted;
    BuiltinMethodDef SortHeldWeapons;
    BuiltinMethodDef TakeWeapon;
    BuiltinMethodDef IsThrowingGrenade;
    BuiltinMethodDef ForceGrenadeThrow;
    BuiltinMethodDef IsFiring;
    BuiltinMethodDef IsMeleeing;
    BuiltinMethodDef IsGrappling;
    BuiltinMethodDef IsSwitchingWeapons;
    BuiltinMethodDef IsDroppingWeapon;
    BuiltinMethodDef IsReloading;
    BuiltinMethodDef TakeAllWeapons;
    BuiltinMethodDef IsInMoveMode;
    BuiltinMethodDef GetCurrentWeapon;
    BuiltinMethodDef GetCurrentWeaponSpinLerp;
    BuiltinMethodDef SetCurrentWeaponSpinLerp;
    BuiltinMethodDef GetCurrentWeaponAltWeapon;
    BuiltinMethodDef IsWeaponOverheating;
    BuiltinMethodDef SetWeaponOverheating;
    BuiltinMethodDef GetCurrentOffHand;
    BuiltinMethodDef IsUsingOffHand;
    BuiltinMethodDef ForceOffHandEnd;
    BuiltinMethodDef IsChargeShotPending;
    BuiltinMethodDef HasWeapon;
    BuiltinMethodDef GetWeaponForWeaponRoot;
    BuiltinMethodDef SwitchToWeapon;
    BuiltinMethodDef SwitchToWeaponImmediate;
    BuiltinMethodDef SwitchToOffHand;
    BuiltinMethodDef PlayBombPlant;
    BuiltinMethodDef PlayBombDefuse;
    BuiltinMethodDef WeaponPlayEjectBrass;
    BuiltinMethodDef GetLockOnRadius;
    BuiltinMethodDef GetLockOnLossRadius;
    BuiltinMethodDef GetLockOnSpeed;
    BuiltinMethodDef GiveStartAmmo;
    BuiltinMethodDef GiveMaxAmmo;
    BuiltinMethodDef GetFractionStartAmmo;
    BuiltinMethodDef GetFractionMaxAmmo;
    BuiltinMethodDef SetWeaponHeatPercent;
    BuiltinMethodDef SetWeaponHackPercent;
    BuiltinMethodDef SetWeaponLockOnPercent;
    BuiltinMethodDef SetWeaponFriendlyTargeting;
    BuiltinMethodDef SetWeaponFriendlyTargetLocked;
    BuiltinMethodDef SetWeaponFriendlyHacking;
    BuiltinMethodDef SetTargetedEntityEndTime;
    BuiltinMethodDef SetTargetedMissilesRemaining;
    BuiltinMethodDef SetOrigin;
    BuiltinMethodDef SetVelocity;
    BuiltinMethodDef SetPlayerAngles;
    BuiltinMethodDef GetPlayerAngles;
    BuiltinMethodDef GetGunAngles;
    BuiltinMethodDef GetPlayerCameraPos;
    BuiltinMethodDef CameraActivate;
    BuiltinMethodDef CameraSetPosition;
    BuiltinMethodDef CameraSetLookAt;
    BuiltinMethodDef CameraSetAngles;
    BuiltinMethodDef StartCameraTween;
    BuiltinMethodDef GetVRCamPos;
    BuiltinMethodDef GetVRCamAngles;
    BuiltinMethodDef IsVR;
    BuiltinMethodDef GamepadUsedLast;
    BuiltinMethodDef ButtonBitState;
    BuiltinMethodDef PrimaryButtonPressedLocal;
    BuiltinMethodDef SecondaryButtonPressedLocal;
    BuiltinMethodDef UseButtonPressed;
    BuiltinMethodDef ChangeSeatButtonPressed;
    BuiltinMethodDef BoostButtonPressed;
    BuiltinMethodDef AttackButtonPressed;
    BuiltinMethodDef ActionSlotOneButtonPressed;
    BuiltinMethodDef ActionSlotTwoButtonPressed;
    BuiltinMethodDef ActionSlotThreeButtonPressed;
    BuiltinMethodDef ActionSlotFourButtonPressed;
    BuiltinMethodDef AdsButtonPressed;
    BuiltinMethodDef ThrowButtonPressed;
    BuiltinMethodDef MeleeButtonPressed;
    BuiltinMethodDef JumpButtonPressed;
    BuiltinMethodDef ActionButtonPressed;
    BuiltinMethodDef StanceButtonPressed;
    BuiltinMethodDef FragButtonPressed;
    BuiltinMethodDef ReloadButtonPressed;
    BuiltinMethodDef SecondaryOffhandButtonPressed;
    BuiltinMethodDef InventoryButtonPressed;
    BuiltinMethodDef SprintButtonPressed;
    BuiltinMethodDef OffhandSpecialButtonPressed;
    BuiltinMethodDef WeaponSwitchButtonPressed;
    BuiltinMethodDef VehicleMoveUpButtonPressed;
    BuiltinMethodDef SprintUpRequired;
    BuiltinMethodDef PlayerADS;
    BuiltinMethodDef PingPlayer;
    BuiltinMethodDef SetPlayerCharacter;
    BuiltinMethodDef ResetCharacterCustomization;
    BuiltinMethodDef GetCharacterBodyModel;
    BuiltinMethodDef GetCharacterHelmetModel;
    BuiltinMethodDef GetCharacterHeadModel;
    BuiltinMethodDef GetCharacterBodyRenderOptions;
    BuiltinMethodDef SetCharacterBodyType;
    BuiltinMethodDef GetCharacterBodyType;
    BuiltinMethodDef SetCharacterBodyStyle;
    BuiltinMethodDef SetCharacterHelmetStyle;
    BuiltinMethodDef GetPlayerGenderType;
    BuiltinMethodDef GetPlayerShowcaseWeapon;
    BuiltinMethodDef GetPlayerSelectedTaunt;
    BuiltinMethodDef GetPlayerSelectedGesture;
    BuiltinMethodDef GetPlayerSelectedTauntName;
    BuiltinMethodDef GetPlayerSelectedGestureName;
    BuiltinMethodDef ShowViewModel;
    BuiltinMethodDef HideViewModel;
    BuiltinMethodDef SetOffhandSecondaryClass;
    BuiltinMethodDef GetOffHandSecondaryClass;
    BuiltinMethodDef SetOffHandPrimaryClass;
    BuiltinMethodDef BeginLocationSelection;
    BuiltinMethodDef BeginLocationAirstrikeSelection;
    BuiltinMethodDef BeginLocationMortarSelection;
    BuiltinMethodDef BeginLocationArtillerySelection;
    BuiltinMethodDef BeginLocationComlinkSelection;
    BuiltinMethodDef BeginLocationNapalmSelection;
    BuiltinMethodDef EndLocationSelection;
    BuiltinMethodDef ClientSysSetState;
    BuiltinMethodDef WeaponLockDetect;
    BuiltinMethodDef WeaponLockStart;
    BuiltinMethodDef WeaponLockFinalize;
    BuiltinMethodDef WeaponLockFree;
    BuiltinMethodDef WeaponLockTargetTooClose;
    BuiltinMethodDef WeaponLockNoClearance;
    BuiltinMethodDef WeaponLockCount;
    BuiltinMethodDef WeaponLockRemoveSlot;
    BuiltinMethodDef GetMpDialogName;
    BuiltinMethodDef GetHeroWeaponName;
    BuiltinMethodDef GetHeroAbilityName;
    BuiltinMethodDef GetFriendlyDogTagModel;
    BuiltinMethodDef GetEnemyDogTagModel;
    BuiltinMethodDef GetTargetScreenAngles;
    BuiltinMethodDef GetReplayGunLockOnOrigin;
    BuiltinMethodDef GetLockOnOrigin;
    BuiltinMethodDef GetNormalizedMovement;
    BuiltinMethodDef ButtonPressed;
    BuiltinMethodDef GiveWeaponNextAttachment;
    BuiltinMethodDef GiveNextBaseWeapon;
    BuiltinMethodDef GetNormalizedCameraMovement;
    BuiltinMethodDef SayAll;
    BuiltinMethodDef SayTeam;
    BuiltinMethodDef SetSpawnWeapon;
    BuiltinMethodDef ShouldDoInitialWeaponRaise;
    BuiltinMethodDef InitialWeaponRaise;
    BuiltinMethodDef SetEverHadWeaponAll;
    BuiltinMethodDef DropItem;
    BuiltinMethodDef DropScavengerItem;
    BuiltinMethodDef HasAGrenadePulledBack;
    BuiltinMethodDef ClearDamageIndicator;
    BuiltinMethodDef AddToDamageIndicator;
    BuiltinMethodDef FinishPlayerDamage;
    BuiltinMethodDef FinishMeleeHit;
    BuiltinMethodDef ResetAnimations;
    BuiltinMethodDef RevivePlayer;
    BuiltinMethodDef Rejack;
    BuiltinMethodDef InLastStand;
    BuiltinMethodDef UndoLastStand;
    BuiltinMethodDef Suicide;
    BuiltinMethodDef ResetFlashBack;
    BuiltinMethodDef CreateFlashbackClone;
    BuiltinMethodDef TurnedHuman;
    BuiltinMethodDef IsInVehicle;
    BuiltinMethodDef IsRemoteControlling;
    BuiltinMethodDef IsWeaponViewOnlyLinked;
    BuiltinMethodDef AllowStand;
    BuiltinMethodDef AllowCrouch;
    BuiltinMethodDef AllowProne;
    BuiltinMethodDef AllowLean;
    BuiltinMethodDef AllowSlide;
    BuiltinMethodDef AllowDoubleJump;
    BuiltinMethodDef AllowWallRun;
    BuiltinMethodDef GetVehicleOccupied;
    BuiltinMethodDef GetFriendlies;
    BuiltinMethodDef OpenMenu;
    BuiltinMethodDef CloseMenu;
    BuiltinMethodDef OpenMenuNoMouse;
    BuiltinMethodDef CloseInGameMenu;
    BuiltinMethodDef SetControllerUIModelValue;
    BuiltinMethodDef GetControllerUIModelValue;
    BuiltinMethodDef OpenLUIMenu;
    BuiltinMethodDef GetLUIMenu;
    BuiltinMethodDef SetLUIMenuData;
    BuiltinMethodDef GetLUIMenuData;
    BuiltinMethodDef CloseLUIMenu;
    BuiltinMethodDef SetElectrifiedState;
    BuiltinMethodDef FreezeControls;
    BuiltinMethodDef FreezeControlsAllowLook;
    BuiltinMethodDef AreControlsFrozen;
    BuiltinMethodDef DisableUsability;
    BuiltinMethodDef EnableUsability;
    BuiltinMethodDef DisableWeapons;
    BuiltinMethodDef EnableWeapons;
    BuiltinMethodDef DisableWeaponFire;
    BuiltinMethodDef EnableWeaponFire;
    BuiltinMethodDef DisableDeathStreak;
    BuiltinMethodDef EnableDeathStreak;
    BuiltinMethodDef DeathStreakActive;
    BuiltinMethodDef DisableOffhandWeapons;
    BuiltinMethodDef EnableOffhandWeapons;
    BuiltinMethodDef OffhandWeaponsEnabled;
    BuiltinMethodDef DisableOffhandSpecial;
    BuiltinMethodDef EnableOffhandSpecial;
    BuiltinMethodDef DisableWeaponCycling;
    BuiltinMethodDef EnableWeaponCycling;
    BuiltinMethodDef WeaponCyclingEnabled;
    BuiltinMethodDef Revive;
    BuiltinMethodDef SetLastStandPrevWeap;
    BuiltinMethodDef SetWeaponAmmoClip;
    BuiltinMethodDef SetWeaponAmmoStock;
    BuiltinMethodDef SetWeaponAmmoFuel;
    BuiltinMethodDef ReloadWeaponAmmo;
    BuiltinMethodDef GetWeaponAmmoClip;
    BuiltinMethodDef GetWeaponAmmoStock;
    BuiltinMethodDef GetWeaponAmmoFuel;
    BuiltinMethodDef GetWeaponOptions;
    BuiltinMethodDef GetPlayerAttachmentCosmeticVariantIndexes;
    BuiltinMethodDef AnyAmmoForWeaponModes;
    BuiltinMethodDef IPrintLn;
    BuiltinMethodDef IPrintLnBold;
    BuiltinMethodDef Spawn;
    BuiltinMethodDef SpawnSpectator;
    BuiltinMethodDef SetEnterTime;
    BuiltinMethodDef ClonePlayer;
    BuiltinMethodDef SetLowReady;
    BuiltinMethodDef SetCarryingObject;
    BuiltinMethodDef SetClientFocalLength;
    BuiltinMethodDef SetClientThirdPerson;
    BuiltinMethodDef SetClientThirdPersonAngle;
    BuiltinMethodDef SetClientCompass;
    BuiltinMethodDef SetClientScriptMainMenu;
    BuiltinMethodDef SetClientCGObjectiveText;
    BuiltinMethodDef SetClientPlayerSprintTime;
    BuiltinMethodDef SetClientNumLives;
    BuiltinMethodDef SetClientDrawTalk;
    BuiltinMethodDef SetClientMiniScoreboardHide;
    BuiltinMethodDef SetClientHUDHardcore;
    BuiltinMethodDef SetClientPlayerPushAmount;
    BuiltinMethodDef SetClientUIVisibilityFlag;
    BuiltinMethodDef SetClientProfileVar;
    BuiltinMethodDef PlayClientAirstrike;
    BuiltinMethodDef PlayClientNapalm;
    BuiltinMethodDef SetCurrentSpectatorClient;
    BuiltinMethodDef CarryTurret;
    BuiltinMethodDef StopCarryTurret;
    BuiltinMethodDef IsCarryingTurret;
    BuiltinMethodDef CanPlayerPlaceTurret;
    BuiltinMethodDef SetPlacementHint;
    BuiltinMethodDef LinkGuidedMissileCamera;
    BuiltinMethodDef UnlinkGuidedMissileCamera;
    BuiltinMethodDef LinkToMissile;
    BuiltinMethodDef UnlinkFromMissile;
    BuiltinMethodDef DamageRiotShield;
    BuiltinMethodDef HasRiotShield;
    BuiltinMethodDef CanPlaceRiotshield;
    BuiltinMethodDef SetRiotshieldFailHint;
    BuiltinMethodDef SetTargetOORHint;
    BuiltinMethodDef SetTargetAlreadyInUseHint;
    BuiltinMethodDef SetTargetWrongTypeHint;
    BuiltinMethodDef SetNoTargetsHint;
    BuiltinMethodDef SetNoLOSOnTargetsHint;
    BuiltinMethodDef SetDisabledTargetHint;
    BuiltinMethodDef SetTargetAlreadyTargetedHint;
    BuiltinMethodDef SetTargetingAbortedHint;
    BuiltinMethodDef PickupAmmoEvent;
    BuiltinMethodDef PickupWeaponEvent;
    BuiltinMethodDef PrimeMovie;
    BuiltinMethodDef CanPlayerPlaceVehicle;
    BuiltinMethodDef RemoteControlTurret;
    BuiltinMethodDef RemoteControlTurretOff;
    BuiltinMethodDef RecordKillstreakEvent;
    BuiltinMethodDef RecordKillstreakEndEvent;
    BuiltinMethodDef RecordKillstreakAssist;
    BuiltinMethodDef RecordGameEvent;
    BuiltinMethodDef RecordGameEventNonPlayer;
    BuiltinMethodDef RecordKillModifier;
    BuiltinMethodDef RecordMedal;
    BuiltinMethodDef RecordLoadoutPerksAndKillStreaks;
    BuiltinMethodDef RecordLoadoutIndex;
    BuiltinMethodDef RecordMultiKill;
    BuiltinMethodDef IncrementLifeStat;
    BuiltinMethodDef IncrementPlayerStat;
    BuiltinMethodDef RecordPlayerDeathZombies;
    BuiltinMethodDef RecordPlayerDownZombies;
    BuiltinMethodDef RecordPlayerReviveZombies;
    BuiltinMethodDef RecordZombieZone;
    BuiltinMethodDef RecordCareerFlag;
    BuiltinMethodDef RecordCareerStat;
    BuiltinMethodDef RecordCareerStatForMap;
    BuiltinMethodDef RecordCareerFlagForMap;
    BuiltinMethodDef MatchRecordSetCheckpointStat;
    BuiltinMethodDef MatchRecordIncrementCheckpointStat;
    BuiltinMethodDef MatchRecordLogCybercoreEvent;
    BuiltinMethodDef MatchRecordPlayerSpawned;
    BuiltinMethodDef RecordMapEvent;
    BuiltinMethodDef GetMatchRecordLifeIndex;
    BuiltinMethodDef RecordZombieGumballEvent;
    BuiltinMethodDef GetPlayerMovementState;
    BuiltinMethodDef IsLookingAt;
    BuiltinMethodDef CantSeeEntities;
    BuiltinMethodDef PlayHitMarker;
    BuiltinMethodDef PlayLocalSound;
    BuiltinMethodDef StopLocalSound;
    BuiltinMethodDef IsTalking;
    BuiltinMethodDef AllowSpectateTeam;
    BuiltinMethodDef GetIpAddress;
    BuiltinMethodDef GetGuid;
    BuiltinMethodDef GetXuid;
    BuiltinMethodDef IsHost;
    BuiltinMethodDef IsLocalToHost;
    BuiltinMethodDef AllowADS;
    BuiltinMethodDef AllowJump;
    BuiltinMethodDef AllowSprint;
    BuiltinMethodDef AllowMelee;
    BuiltinMethodDef AllowMeleePowerLeft;
    BuiltinMethodDef ForceSlick;
    BuiltinMethodDef SetSprintDuration;
    BuiltinMethodDef SetSprintCooldown;
    BuiltinMethodDef SetSpreadOverride;
    BuiltinMethodDef ResetSpreadOverride;
    BuiltinMethodDef SetActionSlot;
    BuiltinMethodDef GetInventoryWeapon;
    BuiltinMethodDef SetInventoryWeapon;
    BuiltinMethodDef GetWeaponsList;
    BuiltinMethodDef GetWeaponsListPrimaries;
    BuiltinMethodDef StartRevive;
    BuiltinMethodDef StopRevive;
    BuiltinMethodDef ApplyKnockBack;
    BuiltinMethodDef FreerunSetHighScores;
    BuiltinMethodDef SetPerk;
    BuiltinMethodDef HasPerk;
    BuiltinMethodDef GetPerks;
    BuiltinMethodDef ClearPerks;
    BuiltinMethodDef UnSetPerk;
    BuiltinMethodDef HasDLCAvailable;
    BuiltinMethodDef GetDLCAvailable;
    BuiltinMethodDef SetRank;
    BuiltinMethodDef SetParagonRank;
    BuiltinMethodDef SetParagonIconId;
    BuiltinMethodDef EnableInvulnerability;
    BuiltinMethodDef DisableInvulnerability;
    BuiltinMethodDef SetArenaPoints;
    BuiltinMethodDef BotSetRandomCharacterCustomization;
    BuiltinMethodDef IsSplitscreen;
    BuiltinMethodDef IsPlayerOnSameMachine;
    BuiltinMethodDef PlayerKnockback;
    BuiltinMethodDef GetPlayerViewHeight;
    BuiltinMethodDef IsHostForBots;
    BuiltinMethodDef MurderLine;
    BuiltinMethodDef SetOffhandVisible;
    BuiltinMethodDef FakeDamageFrom;
    BuiltinMethodDef GetSnapshotAckIndex;
    BuiltinMethodDef GetLeagueTeamID;
    BuiltinMethodDef DisableRoundStartDelay;
    BuiltinMethodDef EnableRoundStartDelay;
    BuiltinMethodDef HasCyberComAbility;
    BuiltinMethodDef SetCyberComAbility;
    BuiltinMethodDef SetCyberComAbilities;
    BuiltinMethodDef GetCyberComAbilities;
    BuiltinMethodDef GetCyberComUpgrades;
    BuiltinMethodDef GetCyberComRigs;
    BuiltinMethodDef ClearCyberComAbility;
    BuiltinMethodDef HasCyberComRig;
    BuiltinMethodDef SetCyberComRig;
    BuiltinMethodDef SetCyberComRigsFlags;
    BuiltinMethodDef SetCyberComAbilityFlags;
    BuiltinMethodDef SetCyberComUpgradeFlags;
    BuiltinMethodDef ClearCyberComRig;
    BuiltinMethodDef SetCyberComActiveType;
    BuiltinMethodDef GetCyberComActiveType;
    BuiltinMethodDef PopulateCyberComUnlocks;
    BuiltinMethodDef SetTacticalWheel;
    BuiltinMethodDef SetLightingState;
    BuiltinMethodDef GetLightingState;
    BuiltinMethodDef SetSunShadowSplitDistance;
    BuiltinMethodDef GetSunShadowSplitDistance;
    BuiltinMethodDef SetModelLodBias;
    BuiltinMethodDef GiveDedicatedShadow;
    BuiltinMethodDef SetVehicleFocusEntity;
    BuiltinMethodDef GetVehicleFocusEntity;
    BuiltinMethodDef GetVehicleBoostTimeLeft;
    BuiltinMethodDef GetVehicleBoostTime;
    BuiltinMethodDef GetVehicleMinBoostTime;
    BuiltinMethodDef SetVehicleDrivableDuration;
    BuiltinMethodDef SetVehicleDrivableEndTime;
    BuiltinMethodDef SetVehicleDamageMeter;
    BuiltinMethodDef SetVehicleLockedOnByEnt;
    BuiltinMethodDef SetVehicleWeaponWaitDuration;
    BuiltinMethodDef SetVehicleWeaponWaitEndTime;
    BuiltinMethodDef ResetInactivityTimer;
    BuiltinMethodDef SetViewClamp;
  };

  static inline constexpr size_t COUNT = 402;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
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

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(PlayerMethodTable, sizeof(PlayerMethodTable::methods));

union CommonFunctionTable {
  struct {
    BuiltinFunctionDef ReportStubUsage;
    BuiltinFunctionDef Record3DText;
    BuiltinFunctionDef RecordEntText;
    BuiltinFunctionDef RecordLine;
    BuiltinFunctionDef RecordStar;
    BuiltinFunctionDef RecordSphere;
    BuiltinFunctionDef RecordCircle;
    BuiltinFunctionDef RecordCone;
    BuiltinFunctionDef RecordEnt;
    BuiltinFunctionDef RecorderPlayback;
    BuiltinFunctionDef Print;
    BuiltinFunctionDef PrintLn;
    BuiltinFunctionDef PrintTopRightln;
    BuiltinFunctionDef IPrintLn;
    BuiltinFunctionDef IPrintLnBold;
    BuiltinFunctionDef Print3D;
    BuiltinFunctionDef Line;
    BuiltinFunctionDef LineList;
    BuiltinFunctionDef Box;
    BuiltinFunctionDef Circle;
    BuiltinFunctionDef SphericalCone;
    BuiltinFunctionDef DebugStar;
    BuiltinFunctionDef Sphere;
    BuiltinFunctionDef GetEnterButton;
    BuiltinFunctionDef ArraySort;
    BuiltinFunctionDef ArraySortClosest;
    BuiltinFunctionDef GetEnt;
    BuiltinFunctionDef GetEntArray;
    BuiltinFunctionDef GetEntArrayFromArray;
    BuiltinFunctionDef IsNavVolumeLoaded;
    BuiltinFunctionDef GeneratePointsAroundCenter;
    BuiltinFunctionDef GetAngleFromBits;
    BuiltinFunctionDef GetBitsForAngle;
    BuiltinFunctionDef PredictPosition;
    BuiltinFunctionDef GetNode;
    BuiltinFunctionDef GetNodeArray;
    BuiltinFunctionDef GetNodeArraySorted;
    BuiltinFunctionDef GetAnyNodeArray;
    BuiltinFunctionDef GetCoverNodeArray;
    BuiltinFunctionDef GetAllNodes;
    BuiltinFunctionDef GetNodesInRadius;
    BuiltinFunctionDef GetNodesInRadiusSorted;
    BuiltinFunctionDef GetNearestNode;
    BuiltinFunctionDef GetVisibleNode;
    BuiltinFunctionDef NodesVisible;
    BuiltinFunctionDef CanClaimNode;
    BuiltinFunctionDef GetNodeRegion;
    BuiltinFunctionDef GetValidCoverPeekOuts;
    BuiltinFunctionDef GetVisibleNodes;
    BuiltinFunctionDef SetEnableNode;
    BuiltinFunctionDef LinkNodes;
    BuiltinFunctionDef UnLinkNodes;
    BuiltinFunctionDef LinkTraversal;
    BuiltinFunctionDef UnlinkTraversal;
    BuiltinFunctionDef NodesAreLinked;
    BuiltinFunctionDef DropNodeToFloor;
    BuiltinFunctionDef SpawnPathNode;
    BuiltinFunctionDef DeletePathNode;
    BuiltinFunctionDef SetTurretNode;
    BuiltinFunctionDef Ld_Hallway_Front;
    BuiltinFunctionDef SetNodePriority;
    BuiltinFunctionDef IsNodeOccupied;
    BuiltinFunctionDef GetNodeOwner;
    BuiltinFunctionDef IsWallrunNode;
    BuiltinFunctionDef IsCoverNode;
    BuiltinFunctionDef PathDistance;
    BuiltinFunctionDef GetNearestPathPoint;
    BuiltinFunctionDef IsNodeEnabled;
    BuiltinFunctionDef GetOtherNodeInNegotiationPair;
    BuiltinFunctionDef GetClosestPointOnNavMesh;
    BuiltinFunctionDef TracePassedOnNavMesh;
    BuiltinFunctionDef CheckNavMeshDirection;
    BuiltinFunctionDef MarkNoVehicleNavMeshFaces;
    BuiltinFunctionDef ToggleNoVehicleFaceTraversability;
    BuiltinFunctionDef PositionQuery_Source_Navigation;
    BuiltinFunctionDef PositionQuery_Filter_Sight;
    BuiltinFunctionDef PositionQuery_Filter_Directness;
    BuiltinFunctionDef PositionQuery_Filter_DistanceToGoal;
    BuiltinFunctionDef PositionQuery_Filter_InClaimedLocation;
    BuiltinFunctionDef NavPointSightFilter;
    BuiltinFunctionDef IsPointOnNavMesh;
    BuiltinFunctionDef IsPointOnStairs;
    BuiltinFunctionDef SetClearanceCeiling;
    BuiltinFunctionDef GetNavMeshFaceNormal;
    BuiltinFunctionDef GetNavFaceRegion;
    BuiltinFunctionDef EnableNavMeshTrigger;
    BuiltinFunctionDef GetNavMeshTriggersForPoint;
    BuiltinFunctionDef IsPointInNavVolume;
    BuiltinFunctionDef GetReflectionLocs;
    BuiltinFunctionDef GetReflectionOrigin;
    BuiltinFunctionDef BBPrint;
    BuiltinFunctionDef BBPostDemoStreamStatsForRound;
    BuiltinFunctionDef RecordComScoreEvent;
    BuiltinFunctionDef GetVehicleNode;
    BuiltinFunctionDef GetVehicleNodeArray;
    BuiltinFunctionDef GetAllVehicleNodes;
    BuiltinFunctionDef GetTimeFromVehicleNodeToNode;
    BuiltinFunctionDef ReconnectVehicleNodes;
    BuiltinFunctionDef CreateTrack;
    BuiltinFunctionDef GetNumVehicles;
    BuiltinFunctionDef SetHeliHeightPatchEnabled;
    BuiltinFunctionDef DoesWeaponReplaceSpawnWeapon;
    BuiltinFunctionDef GetWeaponWorldModel;
    BuiltinFunctionDef AddDebugCommand;
    BuiltinFunctionDef IsEntity;
    BuiltinFunctionDef IsAlive;
    BuiltinFunctionDef IsCorpse;
    BuiltinFunctionDef IsActorCorpse;
    BuiltinFunctionDef IsActorSpawner;
    BuiltinFunctionDef IsVehicleSpawner;
    BuiltinFunctionDef IsSpawner;
    BuiltinFunctionDef IsPlayer;
    BuiltinFunctionDef IsAI;
    BuiltinFunctionDef IsActor;
    BuiltinFunctionDef IsVehicle;
    BuiltinFunctionDef IsSentient;
    BuiltinFunctionDef IsPathfinder;
    BuiltinFunctionDef IsAirborne;
    BuiltinFunctionDef IsUsingNavVolume;
    BuiltinFunctionDef IsGodMode;
    BuiltinFunctionDef HasASM;
    BuiltinFunctionDef SetDvar;
    BuiltinFunctionDef EACPathSet;
    BuiltinFunctionDef GetTime;
    BuiltinFunctionDef GetMillisecondsRaw;
    BuiltinFunctionDef GetMicrosecondsRaw;
    BuiltinFunctionDef SessionModeIsOnlineGame;
    BuiltinFunctionDef SessionModeIsPrivateOnlineGame;
    BuiltinFunctionDef SessionModeIsPrivate;
    BuiltinFunctionDef SessionModeIsSystemlink;
    BuiltinFunctionDef SessionModeIsZombiesGame;
    BuiltinFunctionDef SessionModeIsCampaignGame;
    BuiltinFunctionDef SessionModeIsMultiplayerGame;
    BuiltinFunctionDef SessionModeAbbreviation;
    BuiltinFunctionDef CurrentSessionMode;
    BuiltinFunctionDef IsDedicated;
    BuiltinFunctionDef GameModeIsMode;
    BuiltinFunctionDef GameModeIsUsingXP;
    BuiltinFunctionDef GameModeIsUsingStats;
    BuiltinFunctionDef GetGametypeSetting;
    BuiltinFunctionDef SetGametypeSetting;
    BuiltinFunctionDef GameModeIsArena;
    BuiltinFunctionDef CheckpointCreate;
    BuiltinFunctionDef CheckpointCommit;
    BuiltinFunctionDef CheckpointClear;
    BuiltinFunctionDef CheckpointRestore;
    BuiltinFunctionDef SaveGame_Create;
    BuiltinFunctionDef SaveGame_ResetCampaignProgress;
    BuiltinFunctionDef SaveGame_GetSavedMap;
    BuiltinFunctionDef SaveGame_GetSavedQueuedMap;
    BuiltinFunctionDef PlaySoundAtPosition;
    BuiltinFunctionDef SetLocalProfileVar;
    BuiltinFunctionDef SetLocalProfileArrayVar;
    BuiltinFunctionDef GetLocalProfileInt;
    BuiltinFunctionDef GetLocalProfileArrayInt;
    BuiltinFunctionDef GetLocalProfileFloat;
    BuiltinFunctionDef GetLocalProfileString;
    BuiltinFunctionDef ExecDevgui;
    BuiltinFunctionDef UploadStats;
    BuiltinFunctionDef RegisterXP;
    BuiltinFunctionDef IsMature;
    BuiltinFunctionDef CamAnimScripted;
    BuiltinFunctionDef EndCamAnimScripted;
    BuiltinFunctionDef ExtraCamAnimScripted;
    BuiltinFunctionDef EndExtraCamAnimScripted;
    BuiltinFunctionDef GetCamAnimTime;
    BuiltinFunctionDef IsCamAnimLooping;
    BuiltinFunctionDef GetClassIndexFromName;
    BuiltinFunctionDef GetAIArray;
    BuiltinFunctionDef GetVehicleArray;
    BuiltinFunctionDef GetActorArray;
    BuiltinFunctionDef GetAISpeciesArray;
    BuiltinFunctionDef GetAIArchetypeArray;
    BuiltinFunctionDef GetFreeActorCount;
    BuiltinFunctionDef GetAITeamArray;
    BuiltinFunctionDef GetActorTeamArray;
    BuiltinFunctionDef GetActorTeamCountRadius;
    BuiltinFunctionDef GetVehicleTeamArray;
    BuiltinFunctionDef GetScriptMoverArray;
    BuiltinFunctionDef GetZBarrierArray;
    BuiltinFunctionDef GetItemArray;
    BuiltinFunctionDef GetCorpseArray;
    BuiltinFunctionDef GetSpawnerArray;
    BuiltinFunctionDef GetVehicleSpawnerArray;
    BuiltinFunctionDef GetActorSpawnerArray;
    BuiltinFunctionDef GetSpawnerTeamArray;
    BuiltinFunctionDef GetActorSpawnerTeamArray;
    BuiltinFunctionDef GetVehicleSpawnerTeamArray;
    BuiltinFunctionDef GetGrappleTargetArray;
    BuiltinFunctionDef BadPlace_Delete;
    BuiltinFunctionDef BadPlace_Box;
    BuiltinFunctionDef BadPlace_Cylinder;
    BuiltinFunctionDef PhysicsTraceEx;
    BuiltinFunctionDef LUINotifyEvent;
    BuiltinFunctionDef ProfileLog_BeginTiming;
    BuiltinFunctionDef ProfileLog_EndTiming;
    BuiltinFunctionDef GetNotetracksInDelta;
    BuiltinFunctionDef IsAnimLeaf;
    BuiltinFunctionDef IsAnimLooping;
    BuiltinFunctionDef IsAnimLoaded;
    BuiltinFunctionDef GetAnimLength;
    BuiltinFunctionDef GetAnimFrameCount;
    BuiltinFunctionDef AnimHasNotetrack;
    BuiltinFunctionDef GetNotetrackTimes;
    BuiltinFunctionDef GetStartOrigin;
    BuiltinFunctionDef GetStartAngles;
    BuiltinFunctionDef DisableGrenadeSuicide;
    BuiltinFunctionDef EnableGrenadeSuicide;
    BuiltinFunctionDef ActivateClientExploder;
    BuiltinFunctionDef DeactivateClientExploder;
    BuiltinFunctionDef ActivateClientRadiantExploder;
    BuiltinFunctionDef DeactivateClientRadiantExploder;
    BuiltinFunctionDef KillClientRadiantExploder;
    BuiltinFunctionDef GetMissileOwner;
    BuiltinFunctionDef PrecacheLeaderboards;
    BuiltinFunctionDef IsAssetLoaded;
    BuiltinFunctionDef DeployRiotShield;
    BuiltinFunctionDef GetNumExpectedPlayers;
    BuiltinFunctionDef GetNumConnectedPlayers;
    BuiltinFunctionDef IsLoadingCinematicPlaying;
    BuiltinFunctionDef GetAtrLoaded;
    BuiltinFunctionDef FindAnimByName;
    BuiltinFunctionDef AnimHasImportantNotifies;
    BuiltinFunctionDef SetHideonClientWhenScriptedAnimCompleted;
    BuiltinFunctionDef RegisterClientField;
    BuiltinFunctionDef CodeSetClientField;
    BuiltinFunctionDef CodeGetClientField;
    BuiltinFunctionDef CodeIncrementClientField;
    BuiltinFunctionDef CodeSetPlayerStateClientField;
    BuiltinFunctionDef CodeGetPlayerStateClientField;
    BuiltinFunctionDef CodeIncrementUIModelClientField;
    BuiltinFunctionDef CodeSetUIModelClientField;
    BuiltinFunctionDef CodeGetUIModelClientField;
    BuiltinFunctionDef CodeIncrementPlayerStateClientField;
    BuiltinFunctionDef CodeSetWorldClientField;
    BuiltinFunctionDef CodeGetWorldClientField;
    BuiltinFunctionDef CodeIncrementWorldClientField;
    BuiltinFunctionDef GetClientFieldVersion;
    BuiltinFunctionDef SetSavedDvar;
    BuiltinFunctionDef GetWeapon;
    BuiltinFunctionDef GetWeaponAccuracy;
    BuiltinFunctionDef GetAttachmentCosmeticVariantIndexes;
    BuiltinFunctionDef GetRandomCompatibleAttachmentsForWeapon;
    BuiltinFunctionDef SetAILimit;
    BuiltinFunctionDef GetAILimit;
    BuiltinFunctionDef GetAICount;
    BuiltinFunctionDef ResetAILimit;
    BuiltinFunctionDef AwarenessUpdatePoolThresholds;
    BuiltinFunctionDef AwarenessSetWeaponShotDistance;
    BuiltinFunctionDef SwitchMap_Preload;
    BuiltinFunctionDef SwitchMap_Load;
    BuiltinFunctionDef SwitchMap_Switch;
    BuiltinFunctionDef SwitchMap_SetLoadingMovie;
    BuiltinFunctionDef GetNextMap;
    BuiltinFunctionDef GetMapOrder;
    BuiltinFunctionDef GetMapIntroMovie;
    BuiltinFunctionDef GetMapOutroMovie;
    BuiltinFunctionDef IsMapSubLevel;
    BuiltinFunctionDef GetRootMapName;
    BuiltinFunctionDef GetMissionName;
    BuiltinFunctionDef MissionHasAccolades;
    BuiltinFunctionDef MissionHasCollectibles;
    BuiltinFunctionDef GetNumberOfCollectiblesForLevel;
    BuiltinFunctionDef GetMissionUniqueID;
    BuiltinFunctionDef GetMissionVersion;
    BuiltinFunctionDef GetFreerunTrackIndex;
    BuiltinFunctionDef GetPlayerCharacterAttribute;
    BuiltinFunctionDef RatRecordMessage;
    BuiltinFunctionDef RatReportCommandResult;
    BuiltinFunctionDef GetHostMigrationValue;
    BuiltinFunctionDef GetHostMigrationArrayCount;
    BuiltinFunctionDef SetHostMigrationValue;
    BuiltinFunctionDef SetLightingState;
    BuiltinFunctionDef VideoPrime;
    BuiltinFunctionDef VideoStart;
    BuiltinFunctionDef VideoPause;
    BuiltinFunctionDef VideoUnpause;
    BuiltinFunctionDef VideoStop;
    BuiltinFunctionDef HideMiscModels;
    BuiltinFunctionDef ShowMiscModels;
    BuiltinFunctionDef UmbraGate_Set;
    BuiltinFunctionDef StreamerSkiptoDebug;
    BuiltinFunctionDef UpdateNavTriggers;
    BuiltinFunctionDef IsArenaMode;
    BuiltinFunctionDef ShowInfoVolume;
    BuiltinFunctionDef HideInfoVolume;
    BuiltinFunctionDef InfoVolumeDebugInit;
  };

  static inline constexpr size_t COUNT = 287;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
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

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(CommonFunctionTable, sizeof(CommonFunctionTable::functions));

union CommonMethodTable {
  struct {
    BuiltinMethodDef Attach;
    BuiltinMethodDef Detach;
    BuiltinMethodDef DetachAll;
    BuiltinMethodDef GetAttachSize;
    BuiltinMethodDef GetAttachModelName;
    BuiltinMethodDef GetAttachTagName;
    BuiltinMethodDef GetAttachIgnoreCollision;
    BuiltinMethodDef HidePart;
    BuiltinMethodDef HasPart;
    BuiltinMethodDef ShowPart;
    BuiltinMethodDef ShowAllParts;
    BuiltinMethodDef PlaySoundToPlayer;
    BuiltinMethodDef PlaySoundToAllButPlayer;
    BuiltinMethodDef AllowedStances;
    BuiltinMethodDef AreAllMissionsAtScore;
    BuiltinMethodDef IsStartingClassDefault;
    BuiltinMethodDef GetNumChallengesComplete;
    BuiltinMethodDef HasCompletedAllGameChallenges;
    BuiltinMethodDef HasCollectedAllCollectibles;
    BuiltinMethodDef GetDStat;
    BuiltinMethodDef GetDStatArrayCount;
    BuiltinMethodDef SetAARStat;
    BuiltinMethodDef SetDStat;
    BuiltinMethodDef AddDStat;
    BuiltinMethodDef SetNonCheckpointData;
    BuiltinMethodDef GetNonCheckpointData;
    BuiltinMethodDef ClearAllNonCheckpointData;
    BuiltinMethodDef ClearNonCheckpointData;
    BuiltinMethodDef AddWeaponStat;
    BuiltinMethodDef AddSpecialistStat;
    BuiltinMethodDef IncrementSpecificWeaponPickedUpCount;
    BuiltinMethodDef AddBonusCardStat;
    BuiltinMethodDef GetSessStat;
    BuiltinMethodDef GetSessStatArrayCount;
    BuiltinMethodDef SetSessStat;
    BuiltinMethodDef AddSessStat;
    BuiltinMethodDef UnlockSongByAlias;
    BuiltinMethodDef CheckIfSongUnlocked;
    BuiltinMethodDef GiveMissingUnlockTokens;
    BuiltinMethodDef UpdateUnlockedAttachmentBits;
    BuiltinMethodDef GetDecorations;
    BuiltinMethodDef GiveDecoration;
    BuiltinMethodDef AddPlayerStat;
    BuiltinMethodDef AddPlayerStatWithGameType;
    BuiltinMethodDef AddGameTypeStat;
    BuiltinMethodDef AddMissionStat;
    BuiltinMethodDef GetCurrentGunRank;
    BuiltinMethodDef TrackHeroPowerAvailable;
    BuiltinMethodDef TrackHeroPowerActivated;
    BuiltinMethodDef TrackHeroPowerExpired;
    BuiltinMethodDef TrackWeaponFireNative;
    BuiltinMethodDef UpdateStatRatio;
    BuiltinMethodDef AddUnlockToken_Weapon;
    BuiltinMethodDef AddUnlockToken_Attachment;
    BuiltinMethodDef GiveUnlockToken;
    BuiltinMethodDef GetXPScale;
    BuiltinMethodDef GetCryptoScale;
    BuiltinMethodDef GetVialsScale;
    BuiltinMethodDef AddRankXP;
    BuiltinMethodDef AddRankXPValue;
    BuiltinMethodDef DoubleXPTimerFired;
    BuiltinMethodDef ReportLootReward;
    BuiltinMethodDef ReportLootConsume;
    BuiltinMethodDef PROTECTED(NoteLootConsume);
    BuiltinMethodDef PROTECTED(ReportNotedLoot);
    BuiltinMethodDef HasSeasonPass;
    BuiltinMethodDef IsStarterPack;
    BuiltinMethodDef SetEMPJammed;
    BuiltinMethodDef IsEMPJammed;
    BuiltinMethodDef SetGrapplableType;
    BuiltinMethodDef LUINotifyEvent;
    BuiltinMethodDef Launch;
    BuiltinMethodDef SetMovingPlatformEnabled;
    BuiltinMethodDef IsMovingPlatform;
    BuiltinMethodDef SetDrawInfrared;
    BuiltinMethodDef SetAssassinationEnabled;
    BuiltinMethodDef GameHistoryStartMatch;
    BuiltinMethodDef GameHistoryFinishMatch;
    BuiltinMethodDef Unlink;
    BuiltinMethodDef LinkTo;
    BuiltinMethodDef LinkToBlendToTag;
    BuiltinMethodDef EnableLinkTo;
    BuiltinMethodDef LinkToUpdateOffset;
    BuiltinMethodDef PlayerLinkTo;
    BuiltinMethodDef PlayerLinkToBlend;
    BuiltinMethodDef PlayerLinkToDelta;
    BuiltinMethodDef PlayerLinkWeaponViewToDelta;
    BuiltinMethodDef PlayerLinkToAbsolute;
    BuiltinMethodDef PlayerCamLinkTo;
    BuiltinMethodDef PlayerCamUnlink;
    BuiltinMethodDef LerpViewAngleClamp;
    BuiltinMethodDef SetViewAngleResistance;
    BuiltinMethodDef PlayerLinkedOffsetEnable;
    BuiltinMethodDef PlayerLinkedOffsetDisable;
    BuiltinMethodDef PlayerLinkedSetUseBaseAngleForViewClamp;
    BuiltinMethodDef ClearGroundEnt;
    BuiltinMethodDef DontInterpolate;
    BuiltinMethodDef MagicGrenadeType;
    BuiltinMethodDef StartFadingBlur;
    BuiltinMethodDef LocalToWorldCoords;
    BuiltinMethodDef WorldToLocalCoords;
    BuiltinMethodDef SetTurretSpinning;
    BuiltinMethodDef AttachShieldModel;
    BuiltinMethodDef DetachShieldModel;
    BuiltinMethodDef SetScriptMoverFlag;
    BuiltinMethodDef RefreshShieldAttachment;
    BuiltinMethodDef MarkDestructibleDestroyed;
    BuiltinMethodDef ResetDestructible;
    BuiltinMethodDef FindPath;
    BuiltinMethodDef CanPath;
    BuiltinMethodDef GetNodeIndexOnPath;
    BuiltinMethodDef GetNextTraversalNodeOnPath;
    BuiltinMethodDef GetClosestPointOnNavVolume;
    BuiltinMethodDef GetRandomPointOnNavVolume;
    BuiltinMethodDef GetInterestPoolValue;
    BuiltinMethodDef GetEventPointOfInterest;
    BuiltinMethodDef GetCurrentEventType;
    BuiltinMethodDef GetCurrentEventTypeName;
    BuiltinMethodDef GetCurrentEventId;
    BuiltinMethodDef GetCurrentEventName;
    BuiltinMethodDef GetCurrentEventOriginator;
    BuiltinMethodDef AddToInterestPool;
    BuiltinMethodDef GetInterestPoolAwareness;
    BuiltinMethodDef PropagateCurrentEvent;
    BuiltinMethodDef GenerateScriptEvent;
    BuiltinMethodDef ServiceEvent;
    BuiltinMethodDef ServiceEventsInRadius;
    BuiltinMethodDef SetAvoidanceMask;
    BuiltinMethodDef SetAvoidanceProperty;
    BuiltinMethodDef IsEventServiced;
    BuiltinMethodDef SetVisionSetForPlayer;
    BuiltinMethodDef UseServerVisionset;
    BuiltinMethodDef SetInfraredVision;
    BuiltinMethodDef GetControllerType;
    BuiltinMethodDef GetActorWeaponOptions;
    BuiltinMethodDef SetActorWeapon;
    BuiltinMethodDef ResetFOV;
    BuiltinMethodDef ClearAnim;
    BuiltinMethodDef ClearAnimLimited;
    BuiltinMethodDef SetAnim;
    BuiltinMethodDef SetFlaggedAnimKnob;
    BuiltinMethodDef SetFlaggedAnimKnobLimited;
    BuiltinMethodDef SetFlaggedAnimKnobRestart;
    BuiltinMethodDef SetFlaggedAnimKnobLimitedRestart;
    BuiltinMethodDef SetFlaggedAnimKnobAll;
    BuiltinMethodDef SetFlaggedAnimKnobAllRestart;
    BuiltinMethodDef SetFlaggedAnim;
    BuiltinMethodDef SetFlaggedAnimLimited;
    BuiltinMethodDef SetFlaggedAnimRestart;
    BuiltinMethodDef SetFlaggedAnimLimitedRestart;
    BuiltinMethodDef SetAnimKnob;
    BuiltinMethodDef SetAnimKnobLimited;
    BuiltinMethodDef SetAnimKnobRestart;
    BuiltinMethodDef SetAnimKnobLimitedRestart;
    BuiltinMethodDef SetAnimKnobAll;
    BuiltinMethodDef SetAnimKnobAllLimited;
    BuiltinMethodDef SetAnimKnobAllRestart;
    BuiltinMethodDef SetAnimKnobAllLimitedRestart;
    BuiltinMethodDef SetAnimForceNew;
    BuiltinMethodDef MagicGrenade;
    BuiltinMethodDef MagicGrenadeManual;
    BuiltinMethodDef MagicGrenadeManualPlayer;
    BuiltinMethodDef GetAimAngles;
    BuiltinMethodDef GetTagOrigin;
    BuiltinMethodDef GetTagAngles;
    BuiltinMethodDef GetShootAtPos;
    BuiltinMethodDef MakeFakeAI;
    BuiltinMethodDef MakeSentient;
    BuiltinMethodDef MakePathfinder;
    BuiltinMethodDef GetDebugEye;
    BuiltinMethodDef ASMRequestSubstate;
    BuiltinMethodDef ASMGetStatus;
    BuiltinMethodDef ASMGetCurrentDeltaAnimation;
    BuiltinMethodDef ASMIsTerminating;
    BuiltinMethodDef ASMIsTransitionRunning;
    BuiltinMethodDef ASMIsSubStatePending;
    BuiltinMethodDef ASMIsTransDecRunning;
    BuiltinMethodDef AnimMappingSearch;
    BuiltinMethodDef ASTSearch;
    BuiltinMethodDef ASMChangeAnimMappingTable;
    BuiltinMethodDef ASMSetAnimationRate;
    BuiltinMethodDef ActorIKEnabled;
    BuiltinMethodDef LookAtEntity;
    BuiltinMethodDef LookAtPos;
    BuiltinMethodDef AimAtPosIK;
    BuiltinMethodDef AimAtEntityIK;
    BuiltinMethodDef GadgetFlickering;
    BuiltinMethodDef GadgetCharging;
    BuiltinMethodDef GadgetPowerChange;
    BuiltinMethodDef GadgetPowerSet;
    BuiltinMethodDef GadgetPowerGet;
    BuiltinMethodDef GadgetIsActive;
    BuiltinMethodDef GadgetIsPrimed;
    BuiltinMethodDef GadgetIsReady;
    BuiltinMethodDef GadgetSetEntity;
    BuiltinMethodDef GadgetSetActivateTime;
    BuiltinMethodDef GadgetPowerReset;
    BuiltinMethodDef GadgetGetSlot;
    BuiltinMethodDef GadgetStateChange;
    BuiltinMethodDef GadgetActivate;
    BuiltinMethodDef GadgetDeactivate;
    BuiltinMethodDef GadgetTargetResult;
    BuiltinMethodDef FlashbackFinish;
    BuiltinMethodDef FlashbackStart;
    BuiltinMethodDef StartResurrectViewAngleTransition;
    BuiltinMethodDef IsGadgetMeleeCharging;
    BuiltinMethodDef SetTargetEntity;
    BuiltinMethodDef SetTargetOrigin;
    BuiltinMethodDef GetTargetOrigin;
    BuiltinMethodDef GetTargetEntity;
    BuiltinMethodDef SetOnTargetAngle;
    BuiltinMethodDef GetEnemies;
    BuiltinMethodDef SetScale;
    BuiltinMethodDef SetTmodeProvider;
    BuiltinMethodDef TmodeSetServerUser;
    BuiltinMethodDef GetMeleeChainCount;
    BuiltinMethodDef OverrideSpawnflags;
    BuiltinMethodDef ExperimentsGetVariant;
  };

  static inline constexpr size_t COUNT = 218;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
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
      PROTECTED_STR(NoteLootConsume),
      PROTECTED_STR(ReportNotedLoot),
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

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(CommonMethodTable, sizeof(CommonMethodTable::methods));

union HelicopterMethodTable {
  struct {
    BuiltinMethodDef SetDamageStage;
    BuiltinMethodDef SetHeliHeightLock;
    BuiltinMethodDef SetHeliHeightCap;
    BuiltinMethodDef GetHeliHeightLock;
    BuiltinMethodDef IsInsideHeliHeightLock;
    BuiltinMethodDef GetHeliHeightLockHeight;
    BuiltinMethodDef SetHeliDogfighting;
  };

  static inline constexpr size_t COUNT = 7;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "SetDamageStage",     "SetHeliHeightLock",      "SetHeliHeightCap",
      "GetHeliHeightLock",  "IsInsideHeliHeightLock", "GetHeliHeightLockHeight",
      "SetHeliDogfighting",
  };

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(HelicopterMethodTable, sizeof(HelicopterMethodTable::methods));

union HudElemMethodTable {
  struct {
    BuiltinMethodDef SetText;
    BuiltinMethodDef SetShader;
    BuiltinMethodDef SetTargetEnt;
    BuiltinMethodDef ClearTargetEnt;
    BuiltinMethodDef SetTimer;
    BuiltinMethodDef SetTimerUp;
    BuiltinMethodDef SetTenthsTimer;
    BuiltinMethodDef SetTenthsTimerUp;
    BuiltinMethodDef SetClock;
    BuiltinMethodDef SetClockUp;
    BuiltinMethodDef SetValue;
    BuiltinMethodDef SetWayPoint;
    BuiltinMethodDef FadeOverTime;
    BuiltinMethodDef ScaleOverTime;
    BuiltinMethodDef MoveOverTime;
    BuiltinMethodDef Reset;
    BuiltinMethodDef Destroy;
    BuiltinMethodDef SetPulseFX;
    BuiltinMethodDef SetCOD7DecodeFX;
    BuiltinMethodDef SetRedactFX;
    BuiltinMethodDef SetTypewriterFX;
    BuiltinMethodDef GetTextWidth;
    BuiltinMethodDef SetPerks;
    BuiltinMethodDef SetPlayerNameString;
    BuiltinMethodDef SetMapNameString;
    BuiltinMethodDef SetGameTypeString;
    BuiltinMethodDef SetWarGameData;
    BuiltinMethodDef ChangeFontScaleOverTime;
  };

  static inline constexpr size_t COUNT = 28;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
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

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(HudElemMethodTable, sizeof(HudElemMethodTable::methods));

union MathFunctionTable {
  struct {
    BuiltinFunctionDef RandomInt;
    BuiltinFunctionDef RandomFloat;
    BuiltinFunctionDef RandomIntRange;
    BuiltinFunctionDef RandomFloatRange;
  };

  static inline constexpr size_t COUNT = 4;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "RandomInt",
      "RandomFloat",
      "RandomIntRange",
      "RandomFloatRange",
  };

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(MathFunctionTable, sizeof(MathFunctionTable::functions));

union ScriptEntMethodTable {
  struct {
    BuiltinMethodDef MoveTo;
    BuiltinMethodDef MoveX;
    BuiltinMethodDef MoveY;
    BuiltinMethodDef MoveZ;
    BuiltinMethodDef MoveGravity;
    BuiltinMethodDef MoveSlide;
    BuiltinMethodDef StopMoveSlide;
    BuiltinMethodDef RotateTo;
    BuiltinMethodDef RotatePitch;
    BuiltinMethodDef RotateYaw;
    BuiltinMethodDef RotateRoll;
    BuiltinMethodDef Vibrate;
    BuiltinMethodDef Bobbing;
    BuiltinMethodDef Rotate;
    BuiltinMethodDef RotateVelocity;
    BuiltinMethodDef SetCanDamage;
    BuiltinMethodDef PhysicsLaunch;
  };

  static inline constexpr size_t COUNT = 17;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "MoveTo",        "MoveX",     "MoveY",          "MoveZ",
      "MoveGravity",   "MoveSlide", "StopMoveSlide",  "RotateTo",
      "RotatePitch",   "RotateYaw", "RotateRoll",     "Vibrate",
      "Bobbing",       "Rotate",    "RotateVelocity", "SetCanDamage",
      "PhysicsLaunch",
  };

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(ScriptEntMethodTable, sizeof(ScriptEntMethodTable::methods));

union ScriptVehicleMethodTable {
  struct {
    BuiltinMethodDef AttachPath;
    BuiltinMethodDef GetAttachPos;
    BuiltinMethodDef StartPath;
    BuiltinMethodDef DrivePath;
    BuiltinMethodDef PathMove;
    BuiltinMethodDef PathMoveClear;
    BuiltinMethodDef Flip180;
    BuiltinMethodDef PathFixedOffset;
    BuiltinMethodDef PathFixedOffsetClear;
    BuiltinMethodDef PathVariableOffset;
    BuiltinMethodDef PathVariableOffsetClear;
    BuiltinMethodDef GetPathFixedOffset;
    BuiltinMethodDef GetPathVariableOffset;
    BuiltinMethodDef SetDrivePathPhysicsScale;
    BuiltinMethodDef SetSwitchNode;
    BuiltinMethodDef SwitchStartNode;
    BuiltinMethodDef RecalcSplinePaths;
    BuiltinMethodDef SetSpeed;
    BuiltinMethodDef SetSpeedImmediate;
    BuiltinMethodDef GetSpeed;
    BuiltinMethodDef GetSpeedMPH;
    BuiltinMethodDef GetGoalSpeedMPH;
    BuiltinMethodDef SetBrake;
    BuiltinMethodDef GetBrake;
    BuiltinMethodDef GetHandBrake;
    BuiltinMethodDef GetScriptBrake;
    BuiltinMethodDef GetDefaultAcceleration;
    BuiltinMethodDef SetAcceleration;
    BuiltinMethodDef SetDeceleration;
    BuiltinMethodDef SetMaxSpeedScale;
    BuiltinMethodDef SetMaxAccelerationScale;
    BuiltinMethodDef ResumeSpeed;
    BuiltinMethodDef SetYawSpeed;
    BuiltinMethodDef SetMaxPitchRoll;
    BuiltinMethodDef SetTurningAbility;
    BuiltinMethodDef SetJitterParams;
    BuiltinMethodDef SetAirResistance;
    BuiltinMethodDef GetSteering;
    BuiltinMethodDef GetThrottle;
    BuiltinMethodDef SetPhysAngles;
    BuiltinMethodDef SetHoverParams;
    BuiltinMethodDef JoltBody;
    BuiltinMethodDef FreeVehicle;
    BuiltinMethodDef GetWheelSurface;
    BuiltinMethodDef GetVehicleOwner;
    BuiltinMethodDef IsRemoteControl;
    BuiltinMethodDef IsVehicleUsable;
    BuiltinMethodDef MakeVehicleUsable;
    BuiltinMethodDef MakeVehicleUnusable;
    BuiltinMethodDef UseVehicle;
    BuiltinMethodDef IsVehicleSeatOccupied;
    BuiltinMethodDef SetNearGoalNotifyDist;
    BuiltinMethodDef SetVehGoalPos;
    BuiltinMethodDef ClearVehGoalPos;
    BuiltinMethodDef SetPlaneGoalPos;
    BuiltinMethodDef SetPlaneBarrelRoll;
    BuiltinMethodDef SetGoalYaw;
    BuiltinMethodDef GetGoalYaw;
    BuiltinMethodDef ClearGoalYaw;
    BuiltinMethodDef SetTargetYaw;
    BuiltinMethodDef ClearTargetYaw;
    BuiltinMethodDef SetMinDesiredTurnYaw;
    BuiltinMethodDef SetLookAtEnt;
    BuiltinMethodDef SetLookAtOrigin;
    BuiltinMethodDef ClearLookAtEnt;
    BuiltinMethodDef SetVehicleForDropDeploy;
    BuiltinMethodDef ResetVehicleFromDropDeploy;
    BuiltinMethodDef TakePlayerControl;
    BuiltinMethodDef ReturnPlayerControl;
    BuiltinMethodDef SetTurretTargetVec;
    BuiltinMethodDef SetTurretTargetEnt;
    BuiltinMethodDef SetTurretTargetRelativeAngles;
    BuiltinMethodDef GetTurretLimitsYaw;
    BuiltinMethodDef ClearTurretTarget;
    BuiltinMethodDef SetGunnerTargetVec;
    BuiltinMethodDef GetTurretTargetVec;
    BuiltinMethodDef GetGunnerTargetVec;
    BuiltinMethodDef SetGunnerTargetEnt;
    BuiltinMethodDef ClearGunnerTarget;
    BuiltinMethodDef SetGunnerTurretOnTargetRange;
    BuiltinMethodDef GetGunnerTargetEnt;
    BuiltinMethodDef SetVehWeapon;
    BuiltinMethodDef FireWeapon;
    BuiltinMethodDef StopFireWeapon;
    BuiltinMethodDef IsTurretReady;
    BuiltinMethodDef SetVehMaxSpeed;
    BuiltinMethodDef GetVehOccupants;
    BuiltinMethodDef GetSeatOccupant;
    BuiltinMethodDef GetOccupantSeat;
    BuiltinMethodDef SetSeatOccupied;
    BuiltinMethodDef SeatGetWeapon;
    BuiltinMethodDef GetSeatFiringOrigin;
    BuiltinMethodDef GetSeatFiringAngles;
    BuiltinMethodDef IsGunnerFiring;
    BuiltinMethodDef DisableGunnerFiring;
    BuiltinMethodDef IsDriverFiring;
    BuiltinMethodDef DisableDriverFiring;
    BuiltinMethodDef FinishVehicleDamage;
    BuiltinMethodDef FinishVehicleRadiusDamage;
    BuiltinMethodDef IsVehicleImmuneToDamage;
    BuiltinMethodDef SetDefaultPitch;
    BuiltinMethodDef ClearDefaultPitch;
    BuiltinMethodDef GetAngularVelocity;
    BuiltinMethodDef SetAngularVelocity;
    BuiltinMethodDef SetVehVelocity;
    BuiltinMethodDef CancelAIMove;
    BuiltinMethodDef SetVehicleType;
    BuiltinMethodDef GetPhysAcceleration;
    BuiltinMethodDef SetPhysAcceleration;
    BuiltinMethodDef DrawTrajectory;
    BuiltinMethodDef SetRotorSpeed;
    BuiltinMethodDef GetMaxSpeed;
    BuiltinMethodDef SetBuoyancyOffset;
    BuiltinMethodDef SetMantleEnabled;
    BuiltinMethodDef SetVehicleAvoidance;
    BuiltinMethodDef GetVehicleAvoidance;
    BuiltinMethodDef VehCanSee;
    BuiltinMethodDef VehSeenRecently;
    BuiltinMethodDef VehClearEnemy;
    BuiltinMethodDef GetTurretHeatValue;
    BuiltinMethodDef IsVehicleTurretOverheating;
    BuiltinMethodDef SetPathTransitionTime;
    BuiltinMethodDef GetVehicleAvoidanceNodes;
    BuiltinMethodDef SetVehicleManualControl;
    BuiltinMethodDef IsVehicleManualControl;
    BuiltinMethodDef VehDriverAimAtCrosshairs;
    BuiltinMethodDef GetVehDamageMultiplier;
  };

  static inline constexpr size_t COUNT = 127;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
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

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(ScriptVehicleMethodTable,
            sizeof(ScriptVehicleMethodTable::methods));

union SentientFunctionTable {
  struct {
    BuiltinFunctionDef CreateThreatBiasGroup;
    BuiltinFunctionDef ThreatBiasGroupExists;
    BuiltinFunctionDef GetThreatBias;
    BuiltinFunctionDef SetThreatBias;
    BuiltinFunctionDef SetThreatBiasAgainstAll;
    BuiltinFunctionDef SetIgnoreMeGroup;
    BuiltinFunctionDef SetStealthSigh;
    BuiltinFunctionDef ResetVisibilityCacheWithinRadius;
  };

  static inline constexpr size_t COUNT = 8;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "CreateThreatBiasGroup",   "ThreatBiasGroupExists",
      "GetThreatBias",           "SetThreatBias",
      "SetThreatBiasAgainstAll", "SetIgnoreMeGroup",
      "SetStealthSigh",          "ResetVisibilityCacheWithinRadius",
  };

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(SentientFunctionTable, sizeof(SentientFunctionTable::functions));

union SentientMethodTable {
  struct {
    BuiltinMethodDef GetClosestEnemySqDist;
    BuiltinMethodDef SetThreatBiasGroup;
    BuiltinMethodDef GetThreatBiasGroup;
    BuiltinMethodDef SetPersonalThreatBias;
    BuiltinMethodDef GetPersonalThreatBias;
    BuiltinMethodDef SetPersonalIgnore;
    BuiltinMethodDef IsNoTarget;
    BuiltinMethodDef IsBadGuy;
    BuiltinMethodDef SetEntityTarget;
    BuiltinMethodDef ClearEntityTarget;
    BuiltinMethodDef GetPerfectInfo;
    BuiltinMethodDef IsPosInClaimedLocation;
    BuiltinMethodDef SetGoal;
    BuiltinMethodDef ClearForcedGoal;
    BuiltinMethodDef SetGoalNode;
    BuiltinMethodDef SetGoalPos;
    BuiltinMethodDef SetGoalEntity;
    BuiltinMethodDef SetGoalVolume;
    BuiltinMethodDef SetGoalVolumeAuto;
    BuiltinMethodDef GetGoalVolume;
    BuiltinMethodDef ClearGoalVolume;
    BuiltinMethodDef IsInGoal;
    BuiltinMethodDef AttackedRecently;
    BuiltinMethodDef SetIgnoreEnt;
    BuiltinMethodDef GetIgnoreEnt;
    BuiltinMethodDef AddSentientEventListener;
    BuiltinMethodDef RemoveSentientEventListener;
    BuiltinMethodDef GetStealthSightValue;
    BuiltinMethodDef SetStealthSightValue;
    BuiltinMethodDef SetStealthSightAwareness;
    BuiltinMethodDef GetMotionAngle;
  };

  static inline constexpr size_t COUNT = 31;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
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

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(SentientMethodTable, sizeof(SentientMethodTable::methods));

union UtilFunctionTable {
  struct {
    BuiltinFunctionDef SetSharedViewPort;
    BuiltinFunctionDef SetTopDownCameraYaw;
    BuiltinFunctionDef SetJumpHeight;
  };

  static inline constexpr size_t COUNT = 3;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "SetSharedViewPort",
      "SetTopDownCameraYaw",
      "SetJumpHeight",
  };

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(UtilFunctionTable, sizeof(UtilFunctionTable::functions));

} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game