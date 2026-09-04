#pragma once

#include <game/structs/scr/builtin/core.hpp>
#include <game/structs/scr/builtin/table/macros.hpp>

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace cscr {
union BuiltinFunctionTable {
  struct {
    BuiltinFunctionDef SpawnStruct;
    BuiltinFunctionDef BulletTrace;
    BuiltinFunctionDef BeamTrace;
    BuiltinFunctionDef TracePoint;
    BuiltinFunctionDef SetThirdPerson;
    BuiltinFunctionDef IsThirdPerson;
    BuiltinFunctionDef SetFriendlyNameDraw;
    BuiltinFunctionDef GetDStat;
    BuiltinFunctionDef GetLocalClientPos;
    BuiltinFunctionDef GetLocalClientEyePos;
    BuiltinFunctionDef GetLocalClientAngles;
    BuiltinFunctionDef GetLocalClientFOV;
    BuiltinFunctionDef GetMaxLocalClients;
    BuiltinFunctionDef GetActiveLocalClients;
    BuiltinFunctionDef LocalClientActive;
    BuiltinFunctionDef AimingAtFriendly;
    BuiltinFunctionDef GetControllerPosition;
    BuiltinFunctionDef VrIsActive;
    BuiltinFunctionDef SetVRHeadsetBaseOrientation;
    BuiltinFunctionDef GetVRControllerPosition;
    BuiltinFunctionDef GetVRControllerAngles;
    BuiltinFunctionDef GetVRControllerGripButton;
    BuiltinFunctionDef GetVRControllerXButton;
    BuiltinFunctionDef GetXCamMouseControl;
    BuiltinFunctionDef AllowActionSlotInput;
    BuiltinFunctionDef AllowScoreboard;
    BuiltinFunctionDef Spawn;
    BuiltinFunctionDef SpawnPlane;
    BuiltinFunctionDef GetNumFreeEntities;
    BuiltinFunctionDef GetEnt;
    BuiltinFunctionDef GetEntByNum;
    BuiltinFunctionDef GetEntArray;
    BuiltinFunctionDef GetEntArrayByType;
    BuiltinFunctionDef GetClones;
    BuiltinFunctionDef GetPlayers;
    BuiltinFunctionDef GetLocalPlayers;
    BuiltinFunctionDef GetLocalPlayer;
    BuiltinFunctionDef GetNonPredictedLocalPlayer;
    BuiltinFunctionDef IsAlive;
    BuiltinFunctionDef IsCorpse;
    BuiltinFunctionDef IsVisibleByPlayer;
    BuiltinFunctionDef IsUnderwater;
    BuiltinFunctionDef IsSwimming;
    BuiltinFunctionDef IsLocalClientDead;
    BuiltinFunctionDef GetWeaponAmmoClip;
    BuiltinFunctionDef IsThrowingGrenade;
    BuiltinFunctionDef IsMeleeing;
    BuiltinFunctionDef IsSwitchingWeapons;
    BuiltinFunctionDef IsOnTurret;
    BuiltinFunctionDef ForceTModeVisible;
    BuiltinFunctionDef MarkAsDirty;
    BuiltinFunctionDef GetWeapon;
    BuiltinFunctionDef GetAttachmentCosmeticVariantIndexes;
    BuiltinFunctionDef GetWeaponWithAttachments;
    BuiltinFunctionDef GetWeaponChargeLevel;
    BuiltinFunctionDef SetShowcaseWeaponPaintshopXUID;
    BuiltinFunctionDef GetItemGroupForWeaponName;
    BuiltinFunctionDef GetBubbleGumPack;
    BuiltinFunctionDef PlayRumbleOnPosition;
    BuiltinFunctionDef SetWaveWaterHeight;
    BuiltinFunctionDef SetWaveWaterEnabled;
    BuiltinFunctionDef IsInVehicle;
    BuiltinFunctionDef GetPlayerVehicle;
    BuiltinFunctionDef CurrentSessionMode;
    BuiltinFunctionDef SessionModeIsOnlineGame;
    BuiltinFunctionDef SessionModeIsPrivateOnlineGame;
    BuiltinFunctionDef SessionModeIsPrivate;
    BuiltinFunctionDef SessionModeIsSystemlink;
    BuiltinFunctionDef SessionModeIsZombiesGame;
    BuiltinFunctionDef SessionModeIsCampaignGame;
    BuiltinFunctionDef SessionModeIsMultiplayerGame;
    BuiltinFunctionDef SessionModeIsCampaignDeadOpsGame;
    BuiltinFunctionDef GameModeIsMode;
    BuiltinFunctionDef GameModeIsUsingXP;
    BuiltinFunctionDef GameModeIsUsingStats;
    BuiltinFunctionDef SetSModelSAnimShot;
    BuiltinFunctionDef SModelAnimCmd;
    BuiltinFunctionDef LoadSiegeAnim;
    BuiltinFunctionDef UnloadSiegeAnim;
    BuiltinFunctionDef GetFootstepStrings;
    BuiltinFunctionDef FootstepDoNothing;
    BuiltinFunctionDef FootstepDoEverything;
    BuiltinFunctionDef FootstepDoSound;
    BuiltinFunctionDef FootstepDoFootstepFX;
    BuiltinFunctionDef RopePulse;
    BuiltinFunctionDef GetCamPosByLocalClientNum;
    BuiltinFunctionDef GetCamAnglesByLocalClientNum;
    BuiltinFunctionDef GetVRCamPosByLocalClientNum;
    BuiltinFunctionDef GetVRCamAnglesByLocalClientNum;
    BuiltinFunctionDef IsInScrCam;
    BuiltinFunctionDef DemoIsAnyFreeMoveCamera;
    BuiltinFunctionDef DemoIsMovieCamera;
    BuiltinFunctionDef DemoIsEditCamera;
    BuiltinFunctionDef DemoIsDollyCamera;
    BuiltinFunctionDef RegisterClientField;
    BuiltinFunctionDef CodeGetClientField;
    BuiltinFunctionDef CodeGetPlayerStateClientField;
    BuiltinFunctionDef CodeGetUIModelClientField;
    BuiltinFunctionDef CodeGetWorldClientField;
    BuiltinFunctionDef SetupClientFieldLUICodeCallbacks;
    BuiltinFunctionDef SetupClientFieldCodeCallbacks;
    BuiltinFunctionDef GetClientFieldVersion;
    BuiltinFunctionDef GetServerHighestClientFieldVersion;
    BuiltinFunctionDef GetStartOrigin;
    BuiltinFunctionDef GetStartAngles;
    BuiltinFunctionDef SetBlurByLocalClientNum;
    BuiltinFunctionDef SetBurn;
    BuiltinFunctionDef SetElectrified;
    BuiltinFunctionDef GadgetSetInfrared;
    BuiltinFunctionDef EvSetRanges;
    BuiltinFunctionDef MisdirectionEnable;
    BuiltinFunctionDef IsDemoPlaying;
    BuiltinFunctionDef GetDemoVersion;
    BuiltinFunctionDef GetGametypeSetting;
    BuiltinFunctionDef GetShoutcasterSetting;
    BuiltinFunctionDef IsShoutcaster;
    BuiltinFunctionDef IsSpectating;
    BuiltinFunctionDef PlayerBeingSpectated;
    BuiltinFunctionDef GetLocalPlayerTeam;
    BuiltinFunctionDef IsCameraSpikeToggled;
    BuiltinFunctionDef GetAnimLength;
    BuiltinFunctionDef ForceGameModeMappings;
    BuiltinFunctionDef GetInKillcam;
    BuiltinFunctionDef IsADS;
    BuiltinFunctionDef GetCurrentWeapon;
    BuiltinFunctionDef GetCurrentWeaponIncludingMelee;
    BuiltinFunctionDef WeaponFriendlyHacking;
    BuiltinFunctionDef HasWeapon;
    BuiltinFunctionDef GetTotalAmmo;
    BuiltinFunctionDef IsWeaponOverheating;
    BuiltinFunctionDef GetWeaponHackRatio;
    BuiltinFunctionDef GetGadgetPower;
    BuiltinFunctionDef GetVisionPulseRadius;
    BuiltinFunctionDef GetVisionPulseMaxRadius;
    BuiltinFunctionDef GetRevealPulseRadius;
    BuiltinFunctionDef GetRevealPulseMaxRadius;
    BuiltinFunctionDef GetRevealPulseOrigin;
    BuiltinFunctionDef EnableSpeedBlur;
    BuiltinFunctionDef DisableSpeedBlur;
    BuiltinFunctionDef RadioActive_FX;
    BuiltinFunctionDef BlurAndTint_FX;
    BuiltinFunctionDef GetHealthOverlayTime;
    BuiltinFunctionDef RenderHealthOverlay;
    BuiltinFunctionDef RenderHealthOverlayHealth;
    BuiltinFunctionDef SetLocalRadarEnabled;
    BuiltinFunctionDef SetLocalRadarPosition;
    BuiltinFunctionDef IsInHelicopter;
    BuiltinFunctionDef GetDynEnt;
    BuiltinFunctionDef GetDynEntArray;
    BuiltinFunctionDef SpawnDynEnt;
    BuiltinFunctionDef LaunchDynEnt;
    BuiltinFunctionDef CreateDynEntAndLaunch;
    BuiltinFunctionDef SetDynEntEnabled;
    BuiltinFunctionDef CleanupSpawnedDynEnts;
    BuiltinFunctionDef SetDynEntBodyRenderOptionsPacked;
    BuiltinFunctionDef IsDynEntValid;
    BuiltinFunctionDef SetupClientFieldAnimSpeedCallbacks;
    BuiltinFunctionDef UseAlternateReviveIcon;
    BuiltinFunctionDef SetTeamReviveIcon;
    BuiltinFunctionDef AllowRoundAnimation;
    BuiltinFunctionDef SetBeastModeIconMaterial;
    BuiltinFunctionDef ResetZombieBoxWeapons;
    BuiltinFunctionDef AddZombieBoxWeapon;
    BuiltinFunctionDef RemoveZombieBoxWeapon;
    BuiltinFunctionDef ResetWeaponCosts;
    BuiltinFunctionDef SetWeaponCosts;
    BuiltinFunctionDef SetBGBCost;
    BuiltinFunctionDef SetMapLatLong;
    BuiltinFunctionDef InitClientObjectives;
    BuiltinFunctionDef Objective_Add;
    BuiltinFunctionDef Objective_SetIconSize;
    BuiltinFunctionDef Objective_SetStencil;
    BuiltinFunctionDef Objective_OnEntity;
    BuiltinFunctionDef Objective_Delete;
    BuiltinFunctionDef Objective_FlipIcon;
    BuiltinFunctionDef Objective_State;
    BuiltinFunctionDef Objective_SetIcon;
    BuiltinFunctionDef ServerObjective_GetObjective;
    BuiltinFunctionDef ServerObjective_GetObjectiveEntity;
    BuiltinFunctionDef ServerObjective_GetObjectiveGameModeFlags;
    BuiltinFunctionDef ServerObjective_GetObjectiveTeam;
    BuiltinFunctionDef ServerObjective_GetObjectiveProgress;
    BuiltinFunctionDef ServerObjective_GetObjectiveOrigin;
    BuiltinFunctionDef RopeGetPosition;
    BuiltinFunctionDef GetRope;
    BuiltinFunctionDef SetFlagAsAway;
    BuiltinFunctionDef PhysicsExplosionSphere;
    BuiltinFunctionDef PhysicsExplosionCylinder;
    BuiltinFunctionDef ForceStreamXModel;
    BuiltinFunctionDef StopForceStreamingXModel;
    BuiltinFunctionDef ForceStreamMaterial;
    BuiltinFunctionDef StopForceStreamingMaterial;
    BuiltinFunctionDef GetStreamingProgress;
    BuiltinFunctionDef StreamerNotify;
    BuiltinFunctionDef StreamTextureList;
    BuiltinFunctionDef StopForcingStreamer;
    BuiltinFunctionDef ForceStreamBundle;
    BuiltinFunctionDef ForceStreamWeapons;
    BuiltinFunctionDef ForceStreamWeaponRenderOptions;
    BuiltinFunctionDef SetScriptStreamBias;
    BuiltinFunctionDef SetStreamerRequest;
    BuiltinFunctionDef ClearStreamerRequest;
    BuiltinFunctionDef GetStreamerRequestProgress;
    BuiltinFunctionDef IsStreamerReady;
    BuiltinFunctionDef CalcWeaponOptions;
    BuiltinFunctionDef GetHeroes;
    BuiltinFunctionDef GetHeroGender;
    BuiltinFunctionDef GetHeadGender;
    BuiltinFunctionDef GetFirstHeroOfGender;
    BuiltinFunctionDef GetFirstHeadOfGender;
    BuiltinFunctionDef GetHeroBodyModelIndices;
    BuiltinFunctionDef GetHeroHelmetModelIndices;
    BuiltinFunctionDef GetHeroHeadModelIndices;
    BuiltinFunctionDef GetEquippedBodyForHero;
    BuiltinFunctionDef GetEquippedHelmetForHero;
    BuiltinFunctionDef GetEquippedHeroIndex;
    BuiltinFunctionDef GetEquippedHeroMode;
    BuiltinFunctionDef GetEquippedBodyIndexForHero;
    BuiltinFunctionDef GetEquippedHelmetIndexForHero;
    BuiltinFunctionDef GetEquippedHeadIndexForHero;
    BuiltinFunctionDef GetEquippedShowcaseWeaponForHero;
    BuiltinFunctionDef GetBodyAccentColorCountForHero;
    BuiltinFunctionDef GetHelmetAccentColorCountForHero;
    BuiltinFunctionDef GetEquippedBodyAccentColorForHero;
    BuiltinFunctionDef GetEquippedLoadoutItemForHero;
    BuiltinFunctionDef GetEquippedHelmetAccentColorForHero;
    BuiltinFunctionDef GetEquippedCharacterIndexForLobbyClientHero;
    BuiltinFunctionDef GetTopPlayersTeam;
    BuiltinFunctionDef GetTopPlayersBodyModel;
    BuiltinFunctionDef GetTopPlayersBodyRenderOptions;
    BuiltinFunctionDef GetTopPlayersHelmetModel;
    BuiltinFunctionDef GetTopPlayersHelmetRenderOptions;
    BuiltinFunctionDef GetTopPlayersWeaponRenderOptions;
    BuiltinFunctionDef GetTopPlayersWeaponModel;
    BuiltinFunctionDef GetTopPlayersWeaponInfo;
    BuiltinFunctionDef GetTopPlayersGesture;
    BuiltinFunctionDef GetTopPlayersTaunt;
    BuiltinFunctionDef GetNumClientsInScoreboard;
    BuiltinFunctionDef GetTopScorerCount;
    BuiltinFunctionDef GetCharacterCustomizationForXUID;
    BuiltinFunctionDef IsAnimLooping;
    BuiltinFunctionDef GetNotetracksInDelta;
    BuiltinFunctionDef FlushSubtitles;
    BuiltinFunctionDef SubtitlePrint;
    BuiltinFunctionDef SetControllerLightbarColor;
    BuiltinFunctionDef SetAllControllersLightbarColor;
    BuiltinFunctionDef ClearLastUpdatedCollectibles;
    BuiltinFunctionDef GetMigrationStatus;
    BuiltinFunctionDef GetRoundsPlayed;
    BuiltinFunctionDef AddDebugCommand;
    BuiltinFunctionDef EnableVR;
  };

  static inline constexpr size_t COUNT = 251;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "SpawnStruct",
      "BulletTrace",
      "BeamTrace",
      "TracePoint",
      "SetThirdPerson",
      "IsThirdPerson",
      "SetFriendlyNameDraw",
      "GetDStat",
      "GetLocalClientPos",
      "GetLocalClientEyePos",
      "GetLocalClientAngles",
      "GetLocalClientFOV",
      "GetMaxLocalClients",
      "GetActiveLocalClients",
      "LocalClientActive",
      "AimingAtFriendly",
      "GetControllerPosition",
      "VrIsActive",
      "SetVRHeadsetBaseOrientation",
      "GetVRControllerPosition",
      "GetVRControllerAngles",
      "GetVRControllerGripButton",
      "GetVRControllerXButton",
      "GetXCamMouseControl",
      "AllowActionSlotInput",
      "AllowScoreboard",
      "Spawn",
      "SpawnPlane",
      "GetNumFreeEntities",
      "GetEnt",
      "GetEntByNum",
      "GetEntArray",
      "GetEntArrayByType",
      "GetClones",
      "GetPlayers",
      "GetLocalPlayers",
      "GetLocalPlayer",
      "GetNonPredictedLocalPlayer",
      "IsAlive",
      "IsCorpse",
      "IsVisibleByPlayer",
      "IsUnderwater",
      "IsSwimming",
      "IsLocalClientDead",
      "GetWeaponAmmoClip",
      "IsThrowingGrenade",
      "IsMeleeing",
      "IsSwitchingWeapons",
      "IsOnTurret",
      "ForceTModeVisible",
      "MarkAsDirty",
      "GetWeapon",
      "GetAttachmentCosmeticVariantIndexes",
      "GetWeaponWithAttachments",
      "GetWeaponChargeLevel",
      "SetShowcaseWeaponPaintshopXUID",
      "GetItemGroupForWeaponName",
      "GetBubbleGumPack",
      "PlayRumbleOnPosition",
      "SetWaveWaterHeight",
      "SetWaveWaterEnabled",
      "IsInVehicle",
      "GetPlayerVehicle",
      "CurrentSessionMode",
      "SessionModeIsOnlineGame",
      "SessionModeIsPrivateOnlineGame",
      "SessionModeIsPrivate",
      "SessionModeIsSystemlink",
      "SessionModeIsZombiesGame",
      "SessionModeIsCampaignGame",
      "SessionModeIsMultiplayerGame",
      "SessionModeIsCampaignDeadOpsGame",
      "GameModeIsMode",
      "GameModeIsUsingXP",
      "GameModeIsUsingStats",
      "SetSModelSAnimShot",
      "SModelAnimCmd",
      "LoadSiegeAnim",
      "UnloadSiegeAnim",
      "GetFootstepStrings",
      "FootstepDoNothing",
      "FootstepDoEverything",
      "FootstepDoSound",
      "FootstepDoFootstepFX",
      "RopePulse",
      "GetCamPosByLocalClientNum",
      "GetCamAnglesByLocalClientNum",
      "GetVRCamPosByLocalClientNum",
      "GetVRCamAnglesByLocalClientNum",
      "IsInScrCam",
      "DemoIsAnyFreeMoveCamera",
      "DemoIsMovieCamera",
      "DemoIsEditCamera",
      "DemoIsDollyCamera",
      "RegisterClientField",
      "CodeGetClientField",
      "CodeGetPlayerStateClientField",
      "CodeGetUIModelClientField",
      "CodeGetWorldClientField",
      "SetupClientFieldLUICodeCallbacks",
      "SetupClientFieldCodeCallbacks",
      "GetClientFieldVersion",
      "GetServerHighestClientFieldVersion",
      "GetStartOrigin",
      "GetStartAngles",
      "SetBlurByLocalClientNum",
      "SetBurn",
      "SetElectrified",
      "GadgetSetInfrared",
      "EvSetRanges",
      "MisdirectionEnable",
      "IsDemoPlaying",
      "GetDemoVersion",
      "GetGametypeSetting",
      "GetShoutcasterSetting",
      "IsShoutcaster",
      "IsSpectating",
      "PlayerBeingSpectated",
      "GetLocalPlayerTeam",
      "IsCameraSpikeToggled",
      "GetAnimLength",
      "ForceGameModeMappings",
      "GetInKillcam",
      "IsADS",
      "GetCurrentWeapon",
      "GetCurrentWeaponIncludingMelee",
      "WeaponFriendlyHacking",
      "HasWeapon",
      "GetTotalAmmo",
      "IsWeaponOverheating",
      "GetWeaponHackRatio",
      "GetGadgetPower",
      "GetVisionPulseRadius",
      "GetVisionPulseMaxRadius",
      "GetRevealPulseRadius",
      "GetRevealPulseMaxRadius",
      "GetRevealPulseOrigin",
      "EnableSpeedBlur",
      "DisableSpeedBlur",
      "RadioActive_FX",
      "BlurAndTint_FX",
      "GetHealthOverlayTime",
      "RenderHealthOverlay",
      "RenderHealthOverlayHealth",
      "SetLocalRadarEnabled",
      "SetLocalRadarPosition",
      "IsInHelicopter",
      "GetDynEnt",
      "GetDynEntArray",
      "SpawnDynEnt",
      "LaunchDynEnt",
      "CreateDynEntAndLaunch",
      "SetDynEntEnabled",
      "CleanupSpawnedDynEnts",
      "SetDynEntBodyRenderOptionsPacked",
      "IsDynEntValid",
      "SetupClientFieldAnimSpeedCallbacks",
      "UseAlternateReviveIcon",
      "SetTeamReviveIcon",
      "AllowRoundAnimation",
      "SetBeastModeIconMaterial",
      "ResetZombieBoxWeapons",
      "AddZombieBoxWeapon",
      "RemoveZombieBoxWeapon",
      "ResetWeaponCosts",
      "SetWeaponCosts",
      "SetBGBCost",
      "SetMapLatLong",
      "InitClientObjectives",
      "Objective_Add",
      "Objective_SetIconSize",
      "Objective_SetStencil",
      "Objective_OnEntity",
      "Objective_Delete",
      "Objective_FlipIcon",
      "Objective_State",
      "Objective_SetIcon",
      "ServerObjective_GetObjective",
      "ServerObjective_GetObjectiveEntity",
      "ServerObjective_GetObjectiveGameModeFlags",
      "ServerObjective_GetObjectiveTeam",
      "ServerObjective_GetObjectiveProgress",
      "ServerObjective_GetObjectiveOrigin",
      "RopeGetPosition",
      "GetRope",
      "SetFlagAsAway",
      "PhysicsExplosionSphere",
      "PhysicsExplosionCylinder",
      "ForceStreamXModel",
      "StopForceStreamingXModel",
      "ForceStreamMaterial",
      "StopForceStreamingMaterial",
      "GetStreamingProgress",
      "StreamerNotify",
      "StreamTextureList",
      "StopForcingStreamer",
      "ForceStreamBundle",
      "ForceStreamWeapons",
      "ForceStreamWeaponRenderOptions",
      "SetScriptStreamBias",
      "SetStreamerRequest",
      "ClearStreamerRequest",
      "GetStreamerRequestProgress",
      "IsStreamerReady",
      "CalcWeaponOptions",
      "GetHeroes",
      "GetHeroGender",
      "GetHeadGender",
      "GetFirstHeroOfGender",
      "GetFirstHeadOfGender",
      "GetHeroBodyModelIndices",
      "GetHeroHelmetModelIndices",
      "GetHeroHeadModelIndices",
      "GetEquippedBodyForHero",
      "GetEquippedHelmetForHero",
      "GetEquippedHeroIndex",
      "GetEquippedHeroMode",
      "GetEquippedBodyIndexForHero",
      "GetEquippedHelmetIndexForHero",
      "GetEquippedHeadIndexForHero",
      "GetEquippedShowcaseWeaponForHero",
      "GetBodyAccentColorCountForHero",
      "GetHelmetAccentColorCountForHero",
      "GetEquippedBodyAccentColorForHero",
      "GetEquippedLoadoutItemForHero",
      "GetEquippedHelmetAccentColorForHero",
      "GetEquippedCharacterIndexForLobbyClientHero",
      "GetTopPlayersTeam",
      "GetTopPlayersBodyModel",
      "GetTopPlayersBodyRenderOptions",
      "GetTopPlayersHelmetModel",
      "GetTopPlayersHelmetRenderOptions",
      "GetTopPlayersWeaponRenderOptions",
      "GetTopPlayersWeaponModel",
      "GetTopPlayersWeaponInfo",
      "GetTopPlayersGesture",
      "GetTopPlayersTaunt",
      "GetNumClientsInScoreboard",
      "GetTopScorerCount",
      "GetCharacterCustomizationForXUID",
      "IsAnimLooping",
      "GetNotetracksInDelta",
      "FlushSubtitles",
      "SubtitlePrint",
      "SetControllerLightbarColor",
      "SetAllControllersLightbarColor",
      "ClearLastUpdatedCollectibles",
      "GetMigrationStatus",
      "GetRoundsPlayed",
      "AddDebugCommand",
      "EnableVR"};

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(BuiltinFunctionTable, sizeof(BuiltinFunctionTable::functions));

union BuiltinMethodTable {
  struct {
    BuiltinMethodDef Delete;
    BuiltinMethodDef ForceDelete;
    BuiltinMethodDef GetEntNum;
    BuiltinMethodDef GetEntityNumber;
    BuiltinMethodDef SetModel;
    BuiltinMethodDef MakeFakeAI;
    BuiltinMethodDef SetDrawName;
    BuiltinMethodDef SetDrawOwnerName;
    BuiltinMethodDef SetHighDetail;
    BuiltinMethodDef SetDedicatedShadow;
    BuiltinMethodDef SetScale;
    BuiltinMethodDef SetSAnim;
    BuiltinMethodDef SiegeCmd;
    BuiltinMethodDef Attach;
    BuiltinMethodDef AttachWeapon;
    BuiltinMethodDef IsAttached;
    BuiltinMethodDef Detach;
    BuiltinMethodDef LinkToCamera;
    BuiltinMethodDef IsTouching;
    BuiltinMethodDef GetSpeed;
    BuiltinMethodDef GetVelocity;
    BuiltinMethodDef Underwater;
    BuiltinMethodDef IsPlayer;
    BuiltinMethodDef IsPlayerCorpse;
    BuiltinMethodDef IsLocalPlayer;
    BuiltinMethodDef IsPlayerTalking;
    BuiltinMethodDef IsPlayerDead;
    BuiltinMethodDef IsPlayerFiring;
    BuiltinMethodDef IsPlayerSwimming;
    BuiltinMethodDef IsPlayerSwimmingOnSurface;
    BuiltinMethodDef IsPlayerSwimmingUnderwater;
    BuiltinMethodDef IsPlayerJumping;
    BuiltinMethodDef IsPlayerDoubleJumping;
    BuiltinMethodDef IsPlayerSliding;
    BuiltinMethodDef IsPlayerWallRunning;
    BuiltinMethodDef IsPlayerWallRunningRight;
    BuiltinMethodDef IsPlayerSprinting;
    BuiltinMethodDef IsInScritpedAnim;
    BuiltinMethodDef GetPlayerLastOutWaterTime;
    BuiltinMethodDef IsAI;
    BuiltinMethodDef IsRobot;
    BuiltinMethodDef HasDObj;
    BuiltinMethodDef SetCompassIcon;
    BuiltinMethodDef EnableOnRadar;
    BuiltinMethodDef DisableOnRadar;
    BuiltinMethodDef LaunchRagdoll;
    BuiltinMethodDef IsRagdoll;
    BuiltinMethodDef Solid;
    BuiltinMethodDef NotSolid;
    BuiltinMethodDef Show;
    BuiltinMethodDef Hide;
    BuiltinMethodDef IsHidden;
    BuiltinMethodDef SetMaterialOverride;
    BuiltinMethodDef ClearMaterialOverride;
    BuiltinMethodDef SetStepTriggerSound;
    BuiltinMethodDef ClearStepTriggerSound;
    BuiltinMethodDef GetMovementType;
    BuiltinMethodDef IsInsideHeightLock;
    BuiltinMethodDef IsGrenadeDud;
    BuiltinMethodDef IsEMPJammed;
    BuiltinMethodDef IsFriendly;
    BuiltinMethodDef GetDistanceFromScreenCenter;
    BuiltinMethodDef GetHorizontalOffsetFromScreenCenter;
    BuiltinMethodDef IsStreamed;
    BuiltinMethodDef AreMeshesStreamed;
    BuiltinMethodDef AreTexturesStreamed;
    BuiltinMethodDef GetCharacterIndex;
    BuiltinMethodDef GetPlayerName;
    BuiltinMethodDef MagicBullet;
    BuiltinMethodDef LinkTo;
    BuiltinMethodDef Unlink;
    BuiltinMethodDef IsEntityLinkedToTag;
    BuiltinMethodDef IsLocalPlayerViewLinked;
    BuiltinMethodDef GetThrottle;
    BuiltinMethodDef GetBrake;
    BuiltinMethodDef GetMaxSpeed;
    BuiltinMethodDef GetSteering;
    BuiltinMethodDef GetMaxReverseSpeed;
    BuiltinMethodDef IsLocalClientDriver;
    BuiltinMethodDef GetLocalClientDriver;
    BuiltinMethodDef GetWheelSurface;
    BuiltinMethodDef IsPeelingOut;
    BuiltinMethodDef IsWheelSliding;
    BuiltinMethodDef IsWheelPeelingOut;
    BuiltinMethodDef IsWheelColliding;
    BuiltinMethodDef GetLocalGunnerAngles;
    BuiltinMethodDef SetStunned;
    BuiltinMethodDef IsDriving;
    BuiltinMethodDef DisableVehicleSounds;
    BuiltinMethodDef EnableVehicleSounds;
    BuiltinMethodDef RotatePitch;
    BuiltinMethodDef RotateYaw;
    BuiltinMethodDef RotateRoll;
    BuiltinMethodDef RotateTo;
    BuiltinMethodDef MoveTo;
    BuiltinMethodDef MoveGravity;
    BuiltinMethodDef MoveX;
    BuiltinMethodDef MoveY;
    BuiltinMethodDef MoveZ;
    BuiltinMethodDef RotateVelocity;
    BuiltinMethodDef PlayRumbleOnEntity;
    BuiltinMethodDef PlayRumbleLoopOnEntity;
    BuiltinMethodDef StopRumble;
    BuiltinMethodDef SetControllerLightbarColor;
    BuiltinMethodDef StartPoisoning;
    BuiltinMethodDef StopPoisoning;
    BuiltinMethodDef IsPoisoned;
    BuiltinMethodDef IsSplitScreenHost;
    /*
      Also called `GetStance`. Calls a different function than the prior
      `GetStance` field called `CScrCmd_GetStance`.

      In practice, this function will never be used -
      the prior `GetStance` function will be found first in lookup,
      so will be the only `GetStance` method in this table that is used.
    */
    BuiltinMethodDef GetStance2;
    BuiltinMethodDef GetWeaponPosFrac;
    BuiltinMethodDef SetDamageDirectionIndicator;
    BuiltinMethodDef AddAwarenessIndicator;
    BuiltinMethodDef UseAlternateHud;
    BuiltinMethodDef SetInTacticalHud;
    BuiltinMethodDef GetCybercomType;
    BuiltinMethodDef GetPlayerCorpse;
    BuiltinMethodDef IsLocalPlayerWeaponViewOnlyLinked;
    BuiltinMethodDef IsContentScreenFilterPlaying;
    BuiltinMethodDef GetCamPos;
    BuiltinMethodDef GetCamAngles;
    BuiltinMethodDef UseAnimTree;
    BuiltinMethodDef HasAnimTree;
    BuiltinMethodDef ClearAnim;
    BuiltinMethodDef ClearAnimLimited;
    BuiltinMethodDef SetAnim;
    BuiltinMethodDef SetAnimLimited;
    BuiltinMethodDef SetAnimRestart;
    BuiltinMethodDef SetAnimLimitedRestart;
    BuiltinMethodDef SetFlaggedAnim;
    BuiltinMethodDef SetFlaggedAnimLimited;
    BuiltinMethodDef SetFlaggedAnimRestart;
    BuiltinMethodDef SetFlaggedAnimLimitedRestart;
    BuiltinMethodDef Animscripted;
    BuiltinMethodDef GetCurrentAnimScriptedName;
    BuiltinMethodDef SetHideWhenScriptedAnimationCompleted;
    BuiltinMethodDef SetAnimKnob;
    BuiltinMethodDef SetAnimKnobLimited;
    BuiltinMethodDef SetAnimKnobRestart;
    BuiltinMethodDef SetAnimKnobLimitedRestart;
    BuiltinMethodDef SetAnimKnobAll;
    BuiltinMethodDef SetAnimKnobAllLimited;
    BuiltinMethodDef SetAnimKnobAllRestart;
    BuiltinMethodDef SetAnimKnobAllLimitedRestart;
    BuiltinMethodDef SetFlaggedAnimKnob;
    BuiltinMethodDef SetFlaggedAnimKnobLimited;
    BuiltinMethodDef SetFlaggedAnimKnobRestart;
    BuiltinMethodDef SetFlaggedAnimKnobLimitedRestart;
    BuiltinMethodDef SetFlaggedAnimKnobAll;
    BuiltinMethodDef SetFlaggedAnimKnobAllRestart;
    BuiltinMethodDef GetAnimTime;
    BuiltinMethodDef SetAnimTime;
    BuiltinMethodDef SetAnimTimeByName;
    BuiltinMethodDef GetAnimCurrFrameCount;
    BuiltinMethodDef SetEntityAnimRate;
    BuiltinMethodDef GetEntityAnimRate;
    BuiltinMethodDef AnimGetChildAt;
    BuiltinMethodDef AnimGetNumChildren;
    BuiltinMethodDef GetPrimaryDeltaAnim;
    BuiltinMethodDef SetInfraredVisionSet;
    BuiltinMethodDef SetUnderwaterVisionSet;
    BuiltinMethodDef GetClientTime;
    BuiltinMethodDef GetEye;
    BuiltinMethodDef GetEyeApprox;
    BuiltinMethodDef GetLocalClientNumber;
    BuiltinMethodDef MapShaderConstant;
    BuiltinMethodDef SetShaderConstant;
    BuiltinMethodDef SetForceNotSimple;
    BuiltinMethodDef ProcessClientFieldsAsIfNew;
    BuiltinMethodDef Camera_Set_Lens_ID;
    BuiltinMethodDef CameraSetPosition;
    BuiltinMethodDef CameraSetLookAt;
    BuiltinMethodDef CameraSetUpdateCallback;
    BuiltinMethodDef CameraForceDisableScriptCam;
    BuiltinMethodDef GadgetPulseRevealed;
    BuiltinMethodDef GadgetPulseResetReveal;
    BuiltinMethodDef GadgetPulseGetOwner;
    BuiltinMethodDef GetTargetLockEntity;
    BuiltinMethodDef GetTargetLockEntityArray;
    BuiltinMethodDef GetTagOrigin;
    BuiltinMethodDef GetTagAngles;
    BuiltinMethodDef GetInKillcam;
    BuiltinMethodDef GetKillCamEntity;
    BuiltinMethodDef GetOwner;
    BuiltinMethodDef SetRenderInThirdPersonSpectate;
    BuiltinMethodDef SetWaterDisturbanceParams;
    BuiltinMethodDef GetAnimStateCategory;
    BuiltinMethodDef GetHeliDamageState;
    BuiltinMethodDef IsRemoteControlling;
    BuiltinMethodDef OverrideLightingOrigin;
    BuiltinMethodDef HasPerk;
    BuiltinMethodDef GetPerks;
    BuiltinMethodDef GetStance;
    BuiltinMethodDef ShellShock;
    BuiltinMethodDef PlayerSetGroundReferenceEnt;
    BuiltinMethodDef Earthquake;
    BuiltinMethodDef GetLinkedEnt;
    BuiltinMethodDef SetEnemyGlobalScrambler;
    BuiltinMethodDef SetEnemyScramblerAmount;
    BuiltinMethodDef GetEnemyScramblerAmount;
    BuiltinMethodDef IsScrambled;
    BuiltinMethodDef SetFriendlyScramblerAmount;
    BuiltinMethodDef GetFriendlyScramblerAmount;
    BuiltinMethodDef AddFriendlyScrambler;
    BuiltinMethodDef ClearNearestEnemyScrambler;
    BuiltinMethodDef SetNearestEnemyScrambler;
    BuiltinMethodDef RemoveFriendlyScrambler;
    BuiltinMethodDef RemoveAllFriendlyScramblers;
    BuiltinMethodDef AddSensorGrenadeArea;
    BuiltinMethodDef RemoveSensorGrenadeArea;
    BuiltinMethodDef RemoveAllSensorGrenadeAreas;
    BuiltinMethodDef SetFlagAsAway;
    BuiltinMethodDef GetParentEntity;
    BuiltinMethodDef SetBodyRenderOptions;
    BuiltinMethodDef GetBodyRenderOptionsPacked;
    BuiltinMethodDef SetBodyRenderOptionsPacked;
    BuiltinMethodDef GetCharacterBodyType;
    BuiltinMethodDef SetCorpseGibState;
    BuiltinMethodDef UseWeaponHideTags;
    BuiltinMethodDef UseWeaponModel;
    BuiltinMethodDef UseBuildKitWeaponModel;
    BuiltinMethodDef UseAlternateAimParams;
    BuiltinMethodDef ClearAlternateAimParams;
    BuiltinMethodDef SetSonarAttachmentEnabled;
    BuiltinMethodDef ZBarrierGetPiece;
    BuiltinMethodDef GetNumZBarrierPieces;
    BuiltinMethodDef SetPlayerCybercomAbility;
    BuiltinMethodDef GetCyberComAbilityName;
    BuiltinMethodDef IsCybercomIndexEnabled;
    BuiltinMethodDef ASMIsTerminating;
    BuiltinMethodDef ASMIsShootLayerActive;
    BuiltinMethodDef ASMIsAimLayerActive;
    BuiltinMethodDef ASMGetStatus;
    BuiltinMethodDef ASMGetCurrentState;
    BuiltinMethodDef GetAIFxName;
    BuiltinMethodDef SetEntBeastModeIconType;
    BuiltinMethodDef GetMpDialogName;
    BuiltinMethodDef GetTopPlayersIndex;
    BuiltinMethodDef GetPlayerGibDef;
    BuiltinMethodDef FindDRFilter;
    BuiltinMethodDef HidePart;
    BuiltinMethodDef HasPart;
    BuiltinMethodDef ShowPart;
    BuiltinMethodDef ShowAllParts;
    BuiltinMethodDef ShowViewLegs;
    BuiltinMethodDef HideViewLegs;
    BuiltinMethodDef SuppressRagdollSelfCollision;
  };

  static inline constexpr size_t COUNT = 246;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "Delete",
      "ForceDelete",
      "GetEntNum",
      "GetEntityNumber",
      "SetModel",
      "MakeFakeAI",
      "SetDrawName",
      "SetDrawOwnerName",
      "SetHighDetail",
      "SetDedicatedShadow",
      "SetScale",
      "SetSAnim",
      "SiegeCmd",
      "Attach",
      "AttachWeapon",
      "IsAttached",
      "Detach",
      "LinkToCamera",
      "IsTouching",
      "GetSpeed",
      "GetVelocity",
      "Underwater",
      "IsPlayer",
      "IsPlayerCorpse",
      "IsLocalPlayer",
      "IsPlayerTalking",
      "IsPlayerDead",
      "IsPlayerFiring",
      "IsPlayerSwimming",
      "IsPlayerSwimmingOnSurface",
      "IsPlayerSwimmingUnderwater",
      "IsPlayerJumping",
      "IsPlayerDoubleJumping",
      "IsPlayerSliding",
      "IsPlayerWallRunning",
      "IsPlayerWallRunningRight",
      "IsPlayerSprinting",
      "IsInScritpedAnim",
      "GetPlayerLastOutWaterTime",
      "IsAI",
      "IsRobot",
      "HasDObj",
      "SetCompassIcon",
      "EnableOnRadar",
      "DisableOnRadar",
      "LaunchRagdoll",
      "IsRagdoll",
      "Solid",
      "NotSolid",
      "Show",
      "Hide",
      "IsHidden",
      "SetMaterialOverride",
      "ClearMaterialOverride",
      "SetStepTriggerSound",
      "ClearStepTriggerSound",
      "GetMovementType",
      "IsInsideHeightLock",
      "IsGrenadeDud",
      "IsEMPJammed",
      "IsFriendly",
      "GetDistanceFromScreenCenter",
      "GetHorizontalOffsetFromScreenCenter",
      "IsStreamed",
      "AreMeshesStreamed",
      "AreTexturesStreamed",
      "GetCharacterIndex",
      "GetPlayerName",
      "MagicBullet",
      "LinkTo",
      "Unlink",
      "IsEntityLinkedToTag",
      "IsLocalPlayerViewLinked",
      "GetThrottle",
      "GetBrake",
      "GetMaxSpeed",
      "GetSteering",
      "GetMaxReverseSpeed",
      "IsLocalClientDriver",
      "GetLocalClientDriver",
      "GetWheelSurface",
      "IsPeelingOut",
      "IsWheelSliding",
      "IsWheelPeelingOut",
      "IsWheelColliding",
      "GetLocalGunnerAngles",
      "SetStunned",
      "IsDriving",
      "DisableVehicleSounds",
      "EnableVehicleSounds",
      "RotatePitch",
      "RotateYaw",
      "RotateRoll",
      "RotateTo",
      "MoveTo",
      "MoveGravity",
      "MoveX",
      "MoveY",
      "MoveZ",
      "RotateVelocity",
      "PlayRumbleOnEntity",
      "PlayRumbleLoopOnEntity",
      "StopRumble",
      "SetControllerLightbarColor",
      "StartPoisoning",
      "StopPoisoning",
      "IsPoisoned",
      "IsSplitScreenHost",
      "GetStance2",
      "GetWeaponPosFrac",
      "SetDamageDirectionIndicator",
      "AddAwarenessIndicator",
      "UseAlternateHud",
      "SetInTacticalHud",
      "GetCybercomType",
      "GetPlayerCorpse",
      "IsLocalPlayerWeaponViewOnlyLinked",
      "IsContentScreenFilterPlaying",
      "GetCamPos",
      "GetCamAngles",
      "UseAnimTree",
      "HasAnimTree",
      "ClearAnim",
      "ClearAnimLimited",
      "SetAnim",
      "SetAnimLimited",
      "SetAnimRestart",
      "SetAnimLimitedRestart",
      "SetFlaggedAnim",
      "SetFlaggedAnimLimited",
      "SetFlaggedAnimRestart",
      "SetFlaggedAnimLimitedRestart",
      "Animscripted",
      "GetCurrentAnimScriptedName",
      "SetHideWhenScriptedAnimationCompleted",
      "SetAnimKnob",
      "SetAnimKnobLimited",
      "SetAnimKnobRestart",
      "SetAnimKnobLimitedRestart",
      "SetAnimKnobAll",
      "SetAnimKnobAllLimited",
      "SetAnimKnobAllRestart",
      "SetAnimKnobAllLimitedRestart",
      "SetFlaggedAnimKnob",
      "SetFlaggedAnimKnobLimited",
      "SetFlaggedAnimKnobRestart",
      "SetFlaggedAnimKnobLimitedRestart",
      "SetFlaggedAnimKnobAll",
      "SetFlaggedAnimKnobAllRestart",
      "GetAnimTime",
      "SetAnimTime",
      "SetAnimTimeByName",
      "GetAnimCurrFrameCount",
      "SetEntityAnimRate",
      "GetEntityAnimRate",
      "AnimGetChildAt",
      "AnimGetNumChildren",
      "GetPrimaryDeltaAnim",
      "SetInfraredVisionSet",
      "SetUnderwaterVisionSet",
      "GetClientTime",
      "GetEye",
      "GetEyeApprox",
      "GetLocalClientNumber",
      "MapShaderConstant",
      "SetShaderConstant",
      "SetForceNotSimple",
      "ProcessClientFieldsAsIfNew",
      "Camera_Set_Lens_ID",
      "CameraSetPosition",
      "CameraSetLookAt",
      "CameraSetUpdateCallback",
      "CameraForceDisableScriptCam",
      "GadgetPulseRevealed",
      "GadgetPulseResetReveal",
      "GadgetPulseGetOwner",
      "GetTargetLockEntity",
      "GetTargetLockEntityArray",
      "GetTagOrigin",
      "GetTagAngles",
      "GetInKillcam",
      "GetKillCamEntity",
      "GetOwner",
      "SetRenderInThirdPersonSpectate",
      "SetWaterDisturbanceParams",
      "GetAnimStateCategory",
      "GetHeliDamageState",
      "IsRemoteControlling",
      "OverrideLightingOrigin",
      "HasPerk",
      "GetPerks",
      "GetStance",
      "ShellShock",
      "PlayerSetGroundReferenceEnt",
      "Earthquake",
      "GetLinkedEnt",
      "SetEnemyGlobalScrambler",
      "SetEnemyScramblerAmount",
      "GetEnemyScramblerAmount",
      "IsScrambled",
      "SetFriendlyScramblerAmount",
      "GetFriendlyScramblerAmount",
      "AddFriendlyScrambler",
      "ClearNearestEnemyScrambler",
      "SetNearestEnemyScrambler",
      "RemoveFriendlyScrambler",
      "RemoveAllFriendlyScramblers",
      "AddSensorGrenadeArea",
      "RemoveSensorGrenadeArea",
      "RemoveAllSensorGrenadeAreas",
      "SetFlagAsAway",
      "GetParentEntity",
      "SetBodyRenderOptions",
      "GetBodyRenderOptionsPacked",
      "SetBodyRenderOptionsPacked",
      "GetCharacterBodyType",
      "SetCorpseGibState",
      "UseWeaponHideTags",
      "UseWeaponModel",
      "UseBuildKitWeaponModel",
      "UseAlternateAimParams",
      "ClearAlternateAimParams",
      "SetSonarAttachmentEnabled",
      "ZBarrierGetPiece",
      "GetNumZBarrierPieces",
      "SetPlayerCybercomAbility",
      "GetCyberComAbilityName",
      "IsCybercomIndexEnabled",
      "ASMIsTerminating",
      "ASMIsShootLayerActive",
      "ASMIsAimLayerActive",
      "ASMGetStatus",
      "ASMGetCurrentState",
      "GetAIFxName",
      "SetEntBeastModeIconType",
      "GetMpDialogName",
      "GetTopPlayersIndex",
      "GetPlayerGibDef",
      "FindDRFilter",
      "HidePart",
      "HasPart",
      "ShowPart",
      "ShowAllParts",
      "ShowViewLegs",
      "HideViewLegs",
      "SuppressRagdollSelfCollision",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(BuiltinMethodTable, sizeof(BuiltinMethodTable::methods));

union GfxFunctionTable {
  struct {
    BuiltinFunctionDef PlayFX;
    BuiltinFunctionDef PlayFXOnTag;
    BuiltinFunctionDef PlayTagFXSet;
    BuiltinFunctionDef PlayFXOnCamera;
    BuiltinFunctionDef PlayLoopedFX;
    BuiltinFunctionDef StopFX;
    BuiltinFunctionDef KillFX;
    BuiltinFunctionDef IsFXPlaying;
    BuiltinFunctionDef SetFXOutdoor;
    BuiltinFunctionDef BulletTracer;
    BuiltinFunctionDef TriggerFX;
    BuiltinFunctionDef SetFXTeam;
    BuiltinFunctionDef RegisterRewindFX;
    BuiltinFunctionDef SetFXIgnorePause;
    BuiltinFunctionDef PlayRadiantExploder;
    BuiltinFunctionDef StopRadiantExploder;
    BuiltinFunctionDef KillRadiantExploder;
    BuiltinFunctionDef BeamLaunch;
    BuiltinFunctionDef BeamKill;
    BuiltinFunctionDef VisionSetNaked;
    BuiltinFunctionDef VisionSetNakedLerp;
    BuiltinFunctionDef GetVisionSetNaked;
    BuiltinFunctionDef VisionSetLastStandLerp;
    BuiltinFunctionDef GetFXFromSurfaceTable;
    BuiltinFunctionDef SetRippleWave;
    BuiltinFunctionDef ViewmodelHasTag;
    BuiltinFunctionDef PlayViewmodelFX;
    BuiltinFunctionDef PlayFXOnDynEnt;
    BuiltinFunctionDef SpawnFX;
    BuiltinFunctionDef DeleteFX;
    BuiltinFunctionDef SetRimIntensity;
    BuiltinFunctionDef SetGenericSceneValue;
    BuiltinFunctionDef StartWaterSheetingFX;
    BuiltinFunctionDef StopWaterSheetingFX;
    BuiltinFunctionDef AddBoltedFXExclusionVolume;
    BuiltinFunctionDef AddFXExclusionVolume;
    BuiltinFunctionDef RemoveFXExclusionVolume;
    BuiltinFunctionDef PlayMainCamXCam;
    BuiltinFunctionDef StopMainCamXCam;
    BuiltinFunctionDef SetAllowXCamRightStickRotation;
    BuiltinFunctionDef GetWeaponXCam;
    BuiltinFunctionDef SetClientVolumetricFog;
    BuiltinFunctionDef SwitchToServerVolumetricFog;
    BuiltinFunctionDef SwitchToClientVolumetricFog;
    BuiltinFunctionDef SetServerVolumetricFogDensity;
    BuiltinFunctionDef GetServerVolumetricFogDensity;
    BuiltinFunctionDef SetExposureActiveBank;
    BuiltinFunctionDef SetExposureIgnoreTeleport;
    BuiltinFunctionDef SetExposureInstantSnap;
    BuiltinFunctionDef SetLutVolumeActiveBank;
    BuiltinFunctionDef SetLutScriptIndex;
    BuiltinFunctionDef SetWorldFogActiveBank;
    BuiltinFunctionDef GetWorldFogScriptID;
    BuiltinFunctionDef FindStaticModelIndex;
    BuiltinFunctionDef FindStaticModelIndexArray;
    BuiltinFunctionDef HideStaticModel;
    BuiltinFunctionDef UnhideStaticModel;
    BuiltinFunctionDef FindVolumeDecalIndexArray;
    BuiltinFunctionDef HideVolumeDecal;
    BuiltinFunctionDef UnhideVolumeDecal;
    BuiltinFunctionDef Print;
    BuiltinFunctionDef Print3D;
    BuiltinFunctionDef PrintLn;
    BuiltinFunctionDef PrintTopRightln;
    BuiltinFunctionDef IPrintLnBold;
    BuiltinFunctionDef Line;
    BuiltinFunctionDef Box;
    BuiltinFunctionDef DebugStar;
    BuiltinFunctionDef Circle;
    BuiltinFunctionDef Sphere;
    BuiltinFunctionDef IsStereoOn;
    BuiltinFunctionDef GetSurfaceStrings;
    BuiltinFunctionDef IsSplitScreenHost;
    BuiltinFunctionDef IsSplitScreen;
    BuiltinFunctionDef SetLitFogBank;
    BuiltinFunctionDef SetUkkoScriptIndex;
    BuiltinFunctionDef SetPBGActiveBank;
    BuiltinFunctionDef SetFilterPassMaterial;
    BuiltinFunctionDef SetFilterPassCodeTexture;
    BuiltinFunctionDef SetFilterPassEnabled;
    BuiltinFunctionDef SetFilterPassQuads;
    BuiltinFunctionDef SetFilterPassConstant;
    BuiltinFunctionDef SetFilterBitFlag;
    BuiltinFunctionDef SetOverlayEnabled;
    BuiltinFunctionDef SetOverlayMaterial;
    BuiltinFunctionDef SetOverlayConstant;
    BuiltinFunctionDef EnableThermalDraw;
    BuiltinFunctionDef CreateSceneCodeImage;
    BuiltinFunctionDef FreeCodeImage;
    BuiltinFunctionDef CaptureFrame;
    BuiltinFunctionDef MapMaterialIndex;
    BuiltinFunctionDef ZombieShooterSpotted;
    BuiltinFunctionDef TmodeStatus;
    BuiltinFunctionDef EVStatus;
    BuiltinFunctionDef IGCActive;
    BuiltinFunctionDef IsIGCActive;
    BuiltinFunctionDef EnableFrontendStreamingOverlay;
    BuiltinFunctionDef EnableFrontendLockedWeaponOverlay;
    BuiltinFunctionDef EnableFrontendTokenLockedWeaponOverlay;
    BuiltinFunctionDef UmbraGate_Set;
    BuiltinFunctionDef Umbra_SetDistanceScale;
    BuiltinFunctionDef Umbra_SetAccurateOcclusionThreshold;
    BuiltinFunctionDef Umbra_SetMinimumContributionThreshold;
    BuiltinFunctionDef Umbra_SetTomeTrigger;
    BuiltinFunctionDef Umbra_SetTomeTriggerOnce;
    BuiltinFunctionDef Umbra_ClearPersistentTomeTrigger;
  };

  static inline constexpr size_t COUNT = 106;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "PlayFX",
      "PlayFXOnTag",
      "PlayTagFXSet",
      "PlayFXOnCamera",
      "PlayLoopedFX",
      "StopFX",
      "KillFX",
      "IsFXPlaying",
      "SetFXOutdoor",
      "BulletTracer",
      "TriggerFX",
      "SetFXTeam",
      "RegisterRewindFX",
      "SetFXIgnorePause",
      "PlayRadiantExploder",
      "StopRadiantExploder",
      "KillRadiantExploder",
      "BeamLaunch",
      "BeamKill",
      "VisionSetNaked",
      "VisionSetNakedLerp",
      "GetVisionSetNaked",
      "VisionSetLastStandLerp",
      "GetFXFromSurfaceTable",
      "SetRippleWave",
      "ViewmodelHasTag",
      "PlayViewmodelFX",
      "PlayFXOnDynEnt",
      "SpawnFX",
      "DeleteFX",
      "SetRimIntensity",
      "SetGenericSceneValue",
      "StartWaterSheetingFX",
      "StopWaterSheetingFX",
      "AddBoltedFXExclusionVolume",
      "AddFXExclusionVolume",
      "RemoveFXExclusionVolume",
      "PlayMainCamXCam",
      "StopMainCamXCam",
      "SetAllowXCamRightStickRotation",
      "GetWeaponXCam",
      "SetClientVolumetricFog",
      "SwitchToServerVolumetricFog",
      "SwitchToClientVolumetricFog",
      "SetServerVolumetricFogDensity",
      "GetServerVolumetricFogDensity",
      "SetExposureActiveBank",
      "SetExposureIgnoreTeleport",
      "SetExposureInstantSnap",
      "SetLutVolumeActiveBank",
      "SetLutScriptIndex",
      "SetWorldFogActiveBank",
      "GetWorldFogScriptID",
      "FindStaticModelIndex",
      "FindStaticModelIndexArray",
      "HideStaticModel",
      "UnhideStaticModel",
      "FindVolumeDecalIndexArray",
      "HideVolumeDecal",
      "UnhideVolumeDecal",
      "Print",
      "Print3D",
      "PrintLn",
      "PrintTopRightln",
      "IPrintLnBold",
      "Line",
      "Box",
      "DebugStar",
      "Circle",
      "Sphere",
      "IsStereoOn",
      "GetSurfaceStrings",
      "IsSplitScreenHost",
      "IsSplitScreen",
      "SetLitFogBank",
      "SetUkkoScriptIndex",
      "SetPBGActiveBank",
      "SetFilterPassMaterial",
      "SetFilterPassCodeTexture",
      "SetFilterPassEnabled",
      "SetFilterPassQuads",
      "SetFilterPassConstant",
      "SetFilterBitFlag",
      "SetOverlayEnabled",
      "SetOverlayMaterial",
      "SetOverlayConstant",
      "EnableThermalDraw",
      "CreateSceneCodeImage",
      "FreeCodeImage",
      "CaptureFrame",
      "MapMaterialIndex",
      "ZombieShooterSpotted",
      "TmodeStatus",
      "EVStatus",
      "IGCActive",
      "IsIGCActive",
      "EnableFrontendStreamingOverlay",
      "EnableFrontendLockedWeaponOverlay",
      "EnableFrontendTokenLockedWeaponOverlay",
      "UmbraGate_Set",
      "Umbra_SetDistanceScale",
      "Umbra_SetAccurateOcclusionThreshold",
      "Umbra_SetMinimumContributionThreshold",
      "Umbra_SetTomeTrigger",
      "Umbra_SetTomeTriggerOnce",
      "Umbra_ClearPersistentTomeTrigger",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(GfxFunctionTable, sizeof(GfxFunctionTable::functions));

union GfxMethodTable {
  struct {
    BuiltinMethodDef OED_Nightvision_Reset;
    BuiltinMethodDef OED_Nightvision_SetTextureLevel;
    BuiltinMethodDef OED_Nightvision_Enable;
    BuiltinMethodDef OED_SitRepScan_Enable;
    BuiltinMethodDef OED_SitRepScan_SetDesat;
    BuiltinMethodDef OED_SitRepScan_SetSolid;
    BuiltinMethodDef OED_SitRepScan_SetOutline;
    BuiltinMethodDef OED_SitRepScan_SetLineWidth;
    BuiltinMethodDef OED_SitRepScan_SetRadius;
    BuiltinMethodDef OED_SitRepScan_SetFallOff;
    BuiltinMethodDef CODCaster_Keyline_Enable;
    BuiltinMethodDef RandomFade;
    BuiltinMethodDef Weakpoint_Enable;
    BuiltinMethodDef TmodeEnable;
    BuiltinMethodDef EVEnable;
    BuiltinMethodDef EACPathSet;
    BuiltinMethodDef DragonStrike_Enable;
    BuiltinMethodDef DragonStrike_SetPosition;
    BuiltinMethodDef DragonStrike_SetColorRadiusSpinPulse;
    BuiltinMethodDef MotionPulse_Enable;
    BuiltinMethodDef MotionPulse_SetDesat;
    BuiltinMethodDef MotionPulse_SetSolid;
    BuiltinMethodDef MotionPulse_SetOutline;
    BuiltinMethodDef MotionPulse_SetLineWidth;
    BuiltinMethodDef MotionPulse_SetRadius;
    BuiltinMethodDef MotionPulse_SetFallOff;
    BuiltinMethodDef MotionPulse_SetFadeOut;
    BuiltinMethodDef SetBlur;
    BuiltinMethodDef SetExtraCam;
    BuiltinMethodDef SetExtraCamFocalLength;
    BuiltinMethodDef ClearExtraCam;
    BuiltinMethodDef PlayExtraCamXCam;
    BuiltinMethodDef StopExtraCamXCam;
    BuiltinMethodDef SetWeaponRenderOptions;
    BuiltinMethodDef SetAttachmentCosmeticVariantIndex;
    BuiltinMethodDef AddDuplicateRenderOption;
    BuiltinMethodDef DisableDuplicateRendering;
    BuiltinMethodDef SetArmPulse;
    BuiltinMethodDef SetArmPulsePosition;
    BuiltinMethodDef SetHotZone;
    BuiltinMethodDef TmodeSetFlag;
    BuiltinMethodDef TmodeClearFlag;
  };
  static inline constexpr size_t COUNT = 42;

  BuiltinMethodDef methods[COUNT];
  static inline constexpr std::array<const char *, COUNT> names = {
      "OED_Nightvision_Reset",
      "OED_Nightvision_SetTextureLevel",
      "OED_Nightvision_Enable",
      "OED_SitRepScan_Enable",
      "OED_SitRepScan_SetDesat",
      "OED_SitRepScan_SetSolid",
      "OED_SitRepScan_SetOutline",
      "OED_SitRepScan_SetLineWidth",
      "OED_SitRepScan_SetRadius",
      "OED_SitRepScan_SetFallOff",
      "CODCaster_Keyline_Enable",
      "RandomFade",
      "Weakpoint_Enable",
      "TmodeEnable",
      "EVEnable",
      "EACPathSet",
      "DragonStrike_Enable",
      "DragonStrike_SetPosition",
      "DragonStrike_SetColorRadiusSpinPulse",
      "MotionPulse_Enable",
      "MotionPulse_SetDesat",
      "MotionPulse_SetSolid",
      "MotionPulse_SetOutline",
      "MotionPulse_SetLineWidth",
      "MotionPulse_SetRadius",
      "MotionPulse_SetFallOff",
      "MotionPulse_SetFadeOut",
      "SetBlur",
      "SetExtraCam",
      "SetExtraCamFocalLength",
      "ClearExtraCam",
      "PlayExtraCamXCam",
      "StopExtraCamXCam",
      "SetWeaponRenderOptions",
      "SetAttachmentCosmeticVariantIndex",
      "AddDuplicateRenderOption",
      "DisableDuplicateRendering",
      "SetArmPulse",
      "SetArmPulsePosition",
      "SetHotZone",
      "TmodeSetFlag",
      "TmodeClearFlag",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(GfxMethodTable, sizeof(GfxMethodTable::methods));

union MathFunctionTable {
  struct {
    BuiltinFunctionDef RandomInt;
    BuiltinFunctionDef RandomFloat;
    BuiltinFunctionDef RandomIntRange;
    BuiltinFunctionDef RandomFloatRange;
    BuiltinFunctionDef Project2DTo3D;
    BuiltinFunctionDef Project3DTo2D;
    BuiltinFunctionDef ViewAspect;
    BuiltinFunctionDef GetAngleFromBits;
    BuiltinFunctionDef GetBitsForAngle;
  };

  static inline constexpr size_t COUNT = 9;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "RandomInt",        "RandomFloat",      "RandomIntRange",
      "RandomFloatRange", "Project2DTo3D",    "Project3DTo2D",
      "ViewAspect",       "GetAngleFromBits", "GetBitsForAngle",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(MathFunctionTable, sizeof(MathFunctionTable::functions));

union SoundFunctionTable {
  struct {
    BuiltinFunctionDef PlaySound;
    BuiltinFunctionDef GetSoundFromSurfaceTable;
    BuiltinFunctionDef SoundPlaying;
    BuiltinFunctionDef SetSoundVolume;
    BuiltinFunctionDef SetSoundPitch;
    BuiltinFunctionDef SetSoundVolumeRate;
    BuiltinFunctionDef SetSoundPitchRate;
    BuiltinFunctionDef StopSound;
    BuiltinFunctionDef GetRealTime;
    BuiltinFunctionDef GetPlaybackTime;
    BuiltinFunctionDef GetKnownLength;
    BuiltinFunctionDef SetGroupSnapshot;
    BuiltinFunctionDef SetGlobalFutz;
    BuiltinFunctionDef SetAmbientSnapshot;
    BuiltinFunctionDef SetHealthSnapshot;
    BuiltinFunctionDef SetLevelFadeSnapshot;
    BuiltinFunctionDef SoundSetMusicState;
    BuiltinFunctionDef SoundLoopEmitter;
    BuiltinFunctionDef SoundRattleSetup;
    BuiltinFunctionDef SoundRattle;
    BuiltinFunctionDef SoundPlayAutoFX;
    BuiltinFunctionDef SetSoundContext;
    BuiltinFunctionDef SoundStopLoopEmitter;
    BuiltinFunctionDef SoundLineEmitter;
    BuiltinFunctionDef SoundStopLineEmitter;
    BuiltinFunctionDef SoundUpdateLineEmitter;
    BuiltinFunctionDef StopLocalSound;
    BuiltinFunctionDef SoundTimeScale;
    BuiltinFunctionDef AllocateSoundRandoms;
    BuiltinFunctionDef CreateSoundRandom;
    BuiltinFunctionDef ForceAmbientRoom;
  };

  static inline constexpr size_t COUNT = 31;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "PlaySound",
      "GetSoundFromSurfaceTable",
      "SoundPlaying",
      "SetSoundVolume",
      "SetSoundPitch",
      "SetSoundVolumeRate",
      "SetSoundPitchRate",
      "StopSound",
      "GetRealTime",
      "GetPlaybackTime",
      "GetKnownLength",
      "SetGroupSnapshot",
      "SetGlobalFutz",
      "SetAmbientSnapshot",
      "SetHealthSnapshot",
      "SetLevelFadeSnapshot",
      "SoundSetMusicState",
      "SoundLoopEmitter",
      "SoundRattleSetup",
      "SoundRattle",
      "SoundPlayAutoFX",
      "SetSoundContext",
      "SoundStopLoopEmitter",
      "SoundLineEmitter",
      "SoundStopLineEmitter",
      "SoundUpdateLineEmitter",
      "StopLocalSound",
      "SoundTimeScale",
      "AllocateSoundRandoms",
      "CreateSoundRandom",
      "ForceAmbientRoom",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(SoundFunctionTable, sizeof(SoundFunctionTable::functions));

union SoundMethodTable {
  struct {
    BuiltinMethodDef PlaySound;
    BuiltinMethodDef StopSounds;
    BuiltinMethodDef PlayLoopSound;
    BuiltinMethodDef StopLoopSound;
    BuiltinMethodDef StopAllLoopSounds;
    BuiltinMethodDef IsPlayingLoopSound;
    BuiltinMethodDef SetSoundEntContext;
    BuiltinMethodDef SetLoopState;
  };

  static inline constexpr size_t COUNT = 8;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "PlaySound",          "StopSounds",        "PlayLoopSound",
      "StopLoopSound",      "StopAllLoopSounds", "IsPlayingLoopSound",
      "SetSoundEntContext", "SetLoopState",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(SoundMethodTable, sizeof(SoundMethodTable::methods));

union UIFunctionTable {
  struct {
    BuiltinFunctionDef LUILoad;
    BuiltinFunctionDef LUIEnable;
    BuiltinFunctionDef LUIDisable;
    BuiltinFunctionDef IsLUIEnabled;
    BuiltinFunctionDef CreateLUIMenu;
    BuiltinFunctionDef OpenLUIMenu;
    BuiltinFunctionDef GetLUIMenu;
    BuiltinFunctionDef SetLUIMenuData;
    BuiltinFunctionDef GetLUIMenuData;
    BuiltinFunctionDef OpenLUIPopup;
    BuiltinFunctionDef CloseLUIMenu;
    BuiltinFunctionDef PopulateScriptDebugMenu;
    BuiltinFunctionDef CreateUIModel;
    BuiltinFunctionDef FreeUIModel;
    BuiltinFunctionDef GetGlobalUIModel;
    BuiltinFunctionDef GetUIModel;
    BuiltinFunctionDef GetUIModelForController;
    BuiltinFunctionDef GetUIModelValue;
    BuiltinFunctionDef SetUIModelValue;
    BuiltinFunctionDef StopSound;
  };

  static inline constexpr size_t COUNT = 20;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "LUILoad",
      "LUIEnable",
      "LUIDisable",
      "IsLUIEnabled",
      "CreateLUIMenu",
      "OpenLUIMenu",
      "GetLUIMenu",
      "SetLUIMenuData",
      "GetLUIMenuData",
      "OpenLUIPopup",
      "CloseLUIMenu",
      "PopulateScriptDebugMenu",
      "CreateUIModel",
      "FreeUIModel",
      "GetGlobalUIModel",
      "GetUIModel",
      "GetUIModelForController",
      "GetUIModelValue",
      "SetUIModelValue",
      "StopSound",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(UIFunctionTable, sizeof(UIFunctionTable::functions));

union UtilFunctionTable {
  struct {
    BuiltinFunctionDef ReportStubUsage;
    BuiltinFunctionDef PIXBeginEvent;
    BuiltinFunctionDef PIXEndEvent;
    BuiltinFunctionDef HasDLCAvailable;
    BuiltinFunctionDef IsMatureContentEnabled;
    BuiltinFunctionDef IsShowBloodEnabled;
    BuiltinFunctionDef IsShowGibsEnabled;
    BuiltinFunctionDef SplitArgs;
    BuiltinFunctionDef ReCacheLeaderboards;
    BuiltinFunctionDef ClientHasSnapShot;
    BuiltinFunctionDef GetSystemTime;
    BuiltinFunctionDef GetServerTime;
    BuiltinFunctionDef GetMapCenter;
    BuiltinFunctionDef GetSkiptoName;
    BuiltinFunctionDef GetSkipTOS;
    BuiltinFunctionDef GetUTC;
    BuiltinFunctionDef SetDvar;
    BuiltinFunctionDef SetSavedDvar;
    BuiltinFunctionDef GetLightingState;
  };

  static inline constexpr size_t COUNT = 19;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "ReportStubUsage",
      "PIXBeginEvent",
      "PIXEndEvent",
      "HasDLCAvailable",
      "IsMatureContentEnabled",
      "IsShowBloodEnabled",
      "IsShowGibsEnabled",
      "SplitArgs",
      "ReCacheLeaderboards",
      "ClientHasSnapShot",
      "GetSystemTime",
      "GetServerTime",
      "GetMapCenter",
      "GetSkiptoName",
      "GetSkipTOS",
      "GetUTC",
      "SetDvar",
      "SetSavedDvar",
      "GetLightingState",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(BuiltinFunctionTable, sizeof(BuiltinFunctionTable::functions));

union UtilMethodTable {
  struct {
    BuiltinMethodDef ButtonPressed;
    // `PlayerCmd` variant of the method of same name in `BuiltinMethodTable`
    BuiltinMethodDef GetControllerPosition;
  };

  static inline constexpr size_t COUNT = 2;
  BuiltinMethodDef methods[COUNT];
  static inline constexpr std::array<const char *, COUNT> names = {
      "ButtonPressed",
      "GetControllerPosition",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");
  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(UtilMethodTable, sizeof(UtilMethodTable::methods));

} // namespace cscr
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game