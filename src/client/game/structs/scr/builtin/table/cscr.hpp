#pragma once

#include "game/structs/scr/builtin/core.hpp"

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace cscr {
union BuiltinFunctionTable {
  struct {
    const BuiltinFunctionDef SpawnStruct;
    const BuiltinFunctionDef BulletTrace;
    const BuiltinFunctionDef BeamTrace;
    const BuiltinFunctionDef TracePoint;
    const BuiltinFunctionDef SetThirdPerson;
    const BuiltinFunctionDef IsThirdPerson;
    const BuiltinFunctionDef SetFriendlyNameDraw;
    const BuiltinFunctionDef GetDStat;
    const BuiltinFunctionDef GetLocalClientPos;
    const BuiltinFunctionDef GetLocalClientEyePos;
    const BuiltinFunctionDef GetLocalClientAngles;
    const BuiltinFunctionDef GetLocalClientFOV;
    const BuiltinFunctionDef GetMaxLocalClients;
    const BuiltinFunctionDef GetActiveLocalClients;
    const BuiltinFunctionDef LocalClientActive;
    const BuiltinFunctionDef AimingAtFriendly;
    const BuiltinFunctionDef GetControllerPosition;
    const BuiltinFunctionDef VrIsActive;
    const BuiltinFunctionDef SetVRHeadsetBaseOrientation;
    const BuiltinFunctionDef GetVRControllerPosition;
    const BuiltinFunctionDef GetVRControllerAngles;
    const BuiltinFunctionDef GetVRControllerGripButton;
    const BuiltinFunctionDef GetVRControllerXButton;
    const BuiltinFunctionDef GetXCamMouseControl;
    const BuiltinFunctionDef AllowActionSlotInput;
    const BuiltinFunctionDef AllowScoreboard;
    const BuiltinFunctionDef Spawn;
    const BuiltinFunctionDef SpawnPlane;
    const BuiltinFunctionDef GetNumFreeEntities;
    const BuiltinFunctionDef GetEnt;
    const BuiltinFunctionDef GetEntByNum;
    const BuiltinFunctionDef GetEntArray;
    const BuiltinFunctionDef GetEntArrayByType;
    const BuiltinFunctionDef GetClones;
    const BuiltinFunctionDef GetPlayers;
    const BuiltinFunctionDef GetLocalPlayers;
    const BuiltinFunctionDef GetLocalPlayer;
    const BuiltinFunctionDef GetNonPredictedLocalPlayer;
    const BuiltinFunctionDef IsAlive;
    const BuiltinFunctionDef IsCorpse;
    const BuiltinFunctionDef IsVisibleByPlayer;
    const BuiltinFunctionDef IsUnderwater;
    const BuiltinFunctionDef IsSwimming;
    const BuiltinFunctionDef IsLocalClientDead;
    const BuiltinFunctionDef GetWeaponAmmoClip;
    const BuiltinFunctionDef IsThrowingGrenade;
    const BuiltinFunctionDef IsMeleeing;
    const BuiltinFunctionDef IsSwitchingWeapons;
    const BuiltinFunctionDef IsOnTurret;
    const BuiltinFunctionDef ForceTModeVisible;
    const BuiltinFunctionDef MarkAsDirty;
    const BuiltinFunctionDef GetWeapon;
    const BuiltinFunctionDef GetAttachmentCosmeticVariantIndexes;
    const BuiltinFunctionDef GetWeaponWithAttachments;
    const BuiltinFunctionDef GetWeaponChargeLevel;
    const BuiltinFunctionDef SetShowcaseWeaponPaintshopXUID;
    const BuiltinFunctionDef GetItemGroupForWeaponName;
    const BuiltinFunctionDef GetBubbleGumPack;
    const BuiltinFunctionDef PlayRumbleOnPosition;
    const BuiltinFunctionDef SetWaveWaterHeight;
    const BuiltinFunctionDef SetWaveWaterEnabled;
    const BuiltinFunctionDef IsInVehicle;
    const BuiltinFunctionDef GetPlayerVehicle;
    const BuiltinFunctionDef CurrentSessionMode;
    const BuiltinFunctionDef SessionModeIsOnlineGame;
    const BuiltinFunctionDef SessionModeIsPrivateOnlineGame;
    const BuiltinFunctionDef SessionModeIsPrivate;
    const BuiltinFunctionDef SessionModeIsSystemlink;
    const BuiltinFunctionDef SessionModeIsZombiesGame;
    const BuiltinFunctionDef SessionModeIsCampaignGame;
    const BuiltinFunctionDef SessionModeIsMultiplayerGame;
    const BuiltinFunctionDef SessionModeIsCampaignDeadOpsGame;
    const BuiltinFunctionDef GameModeIsMode;
    const BuiltinFunctionDef GameModeIsUsingXP;
    const BuiltinFunctionDef GameModeIsUsingStats;
    const BuiltinFunctionDef SetSModelSAnimShot;
    const BuiltinFunctionDef SModelAnimCmd;
    const BuiltinFunctionDef LoadSiegeAnim;
    const BuiltinFunctionDef UnloadSiegeAnim;
    const BuiltinFunctionDef GetFootstepStrings;
    const BuiltinFunctionDef FootstepDoNothing;
    const BuiltinFunctionDef FootstepDoEverything;
    const BuiltinFunctionDef FootstepDoSound;
    const BuiltinFunctionDef FootstepDoFootstepFX;
    const BuiltinFunctionDef RopePulse;
    const BuiltinFunctionDef GetCamPosByLocalClientNum;
    const BuiltinFunctionDef GetCamAnglesByLocalClientNum;
    const BuiltinFunctionDef GetVRCamPosByLocalClientNum;
    const BuiltinFunctionDef GetVRCamAnglesByLocalClientNum;
    const BuiltinFunctionDef IsInScrCam;
    const BuiltinFunctionDef DemoIsAnyFreeMoveCamera;
    const BuiltinFunctionDef DemoIsMovieCamera;
    const BuiltinFunctionDef DemoIsEditCamera;
    const BuiltinFunctionDef DemoIsDollyCamera;
    const BuiltinFunctionDef RegisterClientField;
    const BuiltinFunctionDef CodeGetClientField;
    const BuiltinFunctionDef CodeGetPlayerStateClientField;
    const BuiltinFunctionDef CodeGetUIModelClientField;
    const BuiltinFunctionDef CodeGetWorldClientField;
    const BuiltinFunctionDef SetupClientFieldLUICodeCallbacks;
    const BuiltinFunctionDef SetupClientFieldCodeCallbacks;
    const BuiltinFunctionDef GetClientFieldVersion;
    const BuiltinFunctionDef GetServerHighestClientFieldVersion;
    const BuiltinFunctionDef GetStartOrigin;
    const BuiltinFunctionDef GetStartAngles;
    const BuiltinFunctionDef SetBlurByLocalClientNum;
    const BuiltinFunctionDef SetBurn;
    const BuiltinFunctionDef SetElectrified;
    const BuiltinFunctionDef GadgetSetInfrared;
    const BuiltinFunctionDef EvSetRanges;
    const BuiltinFunctionDef MisdirectionEnable;
    const BuiltinFunctionDef IsDemoPlaying;
    const BuiltinFunctionDef GetDemoVersion;
    const BuiltinFunctionDef GetGametypeSetting;
    const BuiltinFunctionDef GetShoutcasterSetting;
    const BuiltinFunctionDef IsShoutcaster;
    const BuiltinFunctionDef IsSpectating;
    const BuiltinFunctionDef PlayerBeingSpectated;
    const BuiltinFunctionDef GetLocalPlayerTeam;
    const BuiltinFunctionDef IsCameraSpikeToggled;
    const BuiltinFunctionDef GetAnimLength;
    const BuiltinFunctionDef ForceGameModeMappings;
    const BuiltinFunctionDef GetInKillcam;
    const BuiltinFunctionDef IsADS;
    const BuiltinFunctionDef GetCurrentWeapon;
    const BuiltinFunctionDef GetCurrentWeaponIncludingMelee;
    const BuiltinFunctionDef WeaponFriendlyHacking;
    const BuiltinFunctionDef HasWeapon;
    const BuiltinFunctionDef GetTotalAmmo;
    const BuiltinFunctionDef IsWeaponOverheating;
    const BuiltinFunctionDef GetWeaponHackRatio;
    const BuiltinFunctionDef GetGadgetPower;
    const BuiltinFunctionDef GetVisionPulseRadius;
    const BuiltinFunctionDef GetVisionPulseMaxRadius;
    const BuiltinFunctionDef GetRevealPulseRadius;
    const BuiltinFunctionDef GetRevealPulseMaxRadius;
    const BuiltinFunctionDef GetRevealPulseOrigin;
    const BuiltinFunctionDef EnableSpeedBlur;
    const BuiltinFunctionDef DisableSpeedBlur;
    const BuiltinFunctionDef RadioActive_FX;
    const BuiltinFunctionDef BlurAndTint_FX;
    const BuiltinFunctionDef GetHealthOverlayTime;
    const BuiltinFunctionDef RenderHealthOverlay;
    const BuiltinFunctionDef RenderHealthOverlayHealth;
    const BuiltinFunctionDef SetLocalRadarEnabled;
    const BuiltinFunctionDef SetLocalRadarPosition;
    const BuiltinFunctionDef IsInHelicopter;
    const BuiltinFunctionDef GetDynEnt;
    const BuiltinFunctionDef GetDynEntArray;
    const BuiltinFunctionDef SpawnDynEnt;
    const BuiltinFunctionDef LaunchDynEnt;
    const BuiltinFunctionDef CreateDynEntAndLaunch;
    const BuiltinFunctionDef SetDynEntEnabled;
    const BuiltinFunctionDef CleanupSpawnedDynEnts;
    const BuiltinFunctionDef SetDynEntBodyRenderOptionsPacked;
    const BuiltinFunctionDef IsDynEntValid;
    const BuiltinFunctionDef SetupClientFieldAnimSpeedCallbacks;
    const BuiltinFunctionDef UseAlternateReviveIcon;
    const BuiltinFunctionDef SetTeamReviveIcon;
    const BuiltinFunctionDef AllowRoundAnimation;
    const BuiltinFunctionDef SetBeastModeIconMaterial;
    const BuiltinFunctionDef ResetZombieBoxWeapons;
    const BuiltinFunctionDef AddZombieBoxWeapon;
    const BuiltinFunctionDef RemoveZombieBoxWeapon;
    const BuiltinFunctionDef ResetWeaponCosts;
    const BuiltinFunctionDef SetWeaponCosts;
    const BuiltinFunctionDef SetBGBCost;
    const BuiltinFunctionDef SetMapLatLong;
    const BuiltinFunctionDef InitClientObjectives;
    const BuiltinFunctionDef Objective_Add;
    const BuiltinFunctionDef Objective_SetIconSize;
    const BuiltinFunctionDef Objective_SetStencil;
    const BuiltinFunctionDef Objective_OnEntity;
    const BuiltinFunctionDef Objective_Delete;
    const BuiltinFunctionDef Objective_FlipIcon;
    const BuiltinFunctionDef Objective_State;
    const BuiltinFunctionDef Objective_SetIcon;
    const BuiltinFunctionDef ServerObjective_GetObjective;
    const BuiltinFunctionDef ServerObjective_GetObjectiveEntity;
    const BuiltinFunctionDef ServerObjective_GetObjectiveGameModeFlags;
    const BuiltinFunctionDef ServerObjective_GetObjectiveTeam;
    const BuiltinFunctionDef ServerObjective_GetObjectiveProgress;
    const BuiltinFunctionDef ServerObjective_GetObjectiveOrigin;
    const BuiltinFunctionDef RopeGetPosition;
    const BuiltinFunctionDef GetRope;
    const BuiltinFunctionDef SetFlagAsAway;
    const BuiltinFunctionDef PhysicsExplosionSphere;
    const BuiltinFunctionDef PhysicsExplosionCylinder;
    const BuiltinFunctionDef ForceStreamXModel;
    const BuiltinFunctionDef StopForceStreamingXModel;
    const BuiltinFunctionDef ForceStreamMaterial;
    const BuiltinFunctionDef StopForceStreamingMaterial;
    const BuiltinFunctionDef GetStreamingProgress;
    const BuiltinFunctionDef StreamerNotify;
    const BuiltinFunctionDef StreamTextureList;
    const BuiltinFunctionDef StopForcingStreamer;
    const BuiltinFunctionDef ForceStreamBundle;
    const BuiltinFunctionDef ForceStreamWeapons;
    const BuiltinFunctionDef ForceStreamWeaponRenderOptions;
    const BuiltinFunctionDef SetScriptStreamBias;
    const BuiltinFunctionDef SetStreamerRequest;
    const BuiltinFunctionDef ClearStreamerRequest;
    const BuiltinFunctionDef GetStreamerRequestProgress;
    const BuiltinFunctionDef IsStreamerReady;
    const BuiltinFunctionDef CalcWeaponOptions;
    const BuiltinFunctionDef GetHeroes;
    const BuiltinFunctionDef GetHeroGender;
    const BuiltinFunctionDef GetHeadGender;
    const BuiltinFunctionDef GetFirstHeroOfGender;
    const BuiltinFunctionDef GetFirstHeadOfGender;
    const BuiltinFunctionDef GetHeroBodyModelIndices;
    const BuiltinFunctionDef GetHeroHelmetModelIndices;
    const BuiltinFunctionDef GetHeroHeadModelIndices;
    const BuiltinFunctionDef GetEquippedBodyForHero;
    const BuiltinFunctionDef GetEquippedHelmetForHero;
    const BuiltinFunctionDef GetEquippedHeroIndex;
    const BuiltinFunctionDef GetEquippedHeroMode;
    const BuiltinFunctionDef GetEquippedBodyIndexForHero;
    const BuiltinFunctionDef GetEquippedHelmetIndexForHero;
    const BuiltinFunctionDef GetEquippedHeadIndexForHero;
    const BuiltinFunctionDef GetEquippedShowcaseWeaponForHero;
    const BuiltinFunctionDef GetBodyAccentColorCountForHero;
    const BuiltinFunctionDef GetHelmetAccentColorCountForHero;
    const BuiltinFunctionDef GetEquippedBodyAccentColorForHero;
    const BuiltinFunctionDef GetEquippedLoadoutItemForHero;
    const BuiltinFunctionDef GetEquippedHelmetAccentColorForHero;
    const BuiltinFunctionDef GetEquippedCharacterIndexForLobbyClientHero;
    const BuiltinFunctionDef GetTopPlayersTeam;
    const BuiltinFunctionDef GetTopPlayersBodyModel;
    const BuiltinFunctionDef GetTopPlayersBodyRenderOptions;
    const BuiltinFunctionDef GetTopPlayersHelmetModel;
    const BuiltinFunctionDef GetTopPlayersHelmetRenderOptions;
    const BuiltinFunctionDef GetTopPlayersWeaponRenderOptions;
    const BuiltinFunctionDef GetTopPlayersWeaponModel;
    const BuiltinFunctionDef GetTopPlayersWeaponInfo;
    const BuiltinFunctionDef GetTopPlayersGesture;
    const BuiltinFunctionDef GetTopPlayersTaunt;
    const BuiltinFunctionDef GetNumClientsInScoreboard;
    const BuiltinFunctionDef GetTopScorerCount;
    const BuiltinFunctionDef GetCharacterCustomizationForXUID;
    const BuiltinFunctionDef IsAnimLooping;
    const BuiltinFunctionDef GetNotetracksInDelta;
    const BuiltinFunctionDef FlushSubtitles;
    const BuiltinFunctionDef SubtitlePrint;
    const BuiltinFunctionDef SetControllerLightbarColor;
    const BuiltinFunctionDef SetAllControllersLightbarColor;
    const BuiltinFunctionDef ClearLastUpdatedCollectibles;
    const BuiltinFunctionDef GetMigrationStatus;
    const BuiltinFunctionDef GetRoundsPlayed;
    const BuiltinFunctionDef AddDebugCommand;
    const BuiltinFunctionDef EnableVR;
  };
  const BuiltinFunctionDef functions[251];

  static constexpr const char *names[] = {
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
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(BuiltinFunctionTable, sizeof(BuiltinFunctionTable::functions));

union BuiltinMethodTable {
  struct {
    const BuiltinMethodDef Delete;
    const BuiltinMethodDef ForceDelete;
    const BuiltinMethodDef GetEntNum;
    const BuiltinMethodDef GetEntityNumber;
    const BuiltinMethodDef SetModel;
    const BuiltinMethodDef MakeFakeAI;
    const BuiltinMethodDef SetDrawName;
    const BuiltinMethodDef SetDrawOwnerName;
    const BuiltinMethodDef SetHighDetail;
    const BuiltinMethodDef SetDedicatedShadow;
    const BuiltinMethodDef SetScale;
    const BuiltinMethodDef SetSAnim;
    const BuiltinMethodDef SiegeCmd;
    const BuiltinMethodDef Attach;
    const BuiltinMethodDef AttachWeapon;
    const BuiltinMethodDef IsAttached;
    const BuiltinMethodDef Detach;
    const BuiltinMethodDef LinkToCamera;
    const BuiltinMethodDef IsTouching;
    const BuiltinMethodDef GetSpeed;
    const BuiltinMethodDef GetVelocity;
    const BuiltinMethodDef Underwater;
    const BuiltinMethodDef IsPlayer;
    const BuiltinMethodDef IsPlayerCorpse;
    const BuiltinMethodDef IsLocalPlayer;
    const BuiltinMethodDef IsPlayerTalking;
    const BuiltinMethodDef IsPlayerDead;
    const BuiltinMethodDef IsPlayerFiring;
    const BuiltinMethodDef IsPlayerSwimming;
    const BuiltinMethodDef IsPlayerSwimmingOnSurface;
    const BuiltinMethodDef IsPlayerSwimmingUnderwater;
    const BuiltinMethodDef IsPlayerJumping;
    const BuiltinMethodDef IsPlayerDoubleJumping;
    const BuiltinMethodDef IsPlayerSliding;
    const BuiltinMethodDef IsPlayerWallRunning;
    const BuiltinMethodDef IsPlayerWallRunningRight;
    const BuiltinMethodDef IsPlayerSprinting;
    const BuiltinMethodDef IsInScritpedAnim;
    const BuiltinMethodDef GetPlayerLastOutWaterTime;
    const BuiltinMethodDef IsAI;
    const BuiltinMethodDef IsRobot;
    const BuiltinMethodDef HasDObj;
    const BuiltinMethodDef SetCompassIcon;
    const BuiltinMethodDef EnableOnRadar;
    const BuiltinMethodDef DisableOnRadar;
    const BuiltinMethodDef LaunchRagdoll;
    const BuiltinMethodDef IsRagdoll;
    const BuiltinMethodDef Solid;
    const BuiltinMethodDef NotSolid;
    const BuiltinMethodDef Show;
    const BuiltinMethodDef Hide;
    const BuiltinMethodDef IsHidden;
    const BuiltinMethodDef SetMaterialOverride;
    const BuiltinMethodDef ClearMaterialOverride;
    const BuiltinMethodDef SetStepTriggerSound;
    const BuiltinMethodDef ClearStepTriggerSound;
    const BuiltinMethodDef GetMovementType;
    const BuiltinMethodDef IsInsideHeightLock;
    const BuiltinMethodDef IsGrenadeDud;
    const BuiltinMethodDef IsEMPJammed;
    const BuiltinMethodDef IsFriendly;
    const BuiltinMethodDef GetDistanceFromScreenCenter;
    const BuiltinMethodDef GetHorizontalOffsetFromScreenCenter;
    const BuiltinMethodDef IsStreamed;
    const BuiltinMethodDef AreMeshesStreamed;
    const BuiltinMethodDef AreTexturesStreamed;
    const BuiltinMethodDef GetCharacterIndex;
    const BuiltinMethodDef GetPlayerName;
    const BuiltinMethodDef MagicBullet;
    const BuiltinMethodDef LinkTo;
    const BuiltinMethodDef Unlink;
    const BuiltinMethodDef IsEntityLinkedToTag;
    const BuiltinMethodDef IsLocalPlayerViewLinked;
    const BuiltinMethodDef GetThrottle;
    const BuiltinMethodDef GetBrake;
    const BuiltinMethodDef GetMaxSpeed;
    const BuiltinMethodDef GetSteering;
    const BuiltinMethodDef GetMaxReverseSpeed;
    const BuiltinMethodDef IsLocalClientDriver;
    const BuiltinMethodDef GetLocalClientDriver;
    const BuiltinMethodDef GetWheelSurface;
    const BuiltinMethodDef IsPeelingOut;
    const BuiltinMethodDef IsWheelSliding;
    const BuiltinMethodDef IsWheelPeelingOut;
    const BuiltinMethodDef IsWheelColliding;
    const BuiltinMethodDef GetLocalGunnerAngles;
    const BuiltinMethodDef SetStunned;
    const BuiltinMethodDef IsDriving;
    const BuiltinMethodDef DisableVehicleSounds;
    const BuiltinMethodDef EnableVehicleSounds;
    const BuiltinMethodDef RotatePitch;
    const BuiltinMethodDef RotateYaw;
    const BuiltinMethodDef RotateRoll;
    const BuiltinMethodDef RotateTo;
    const BuiltinMethodDef MoveTo;
    const BuiltinMethodDef MoveGravity;
    const BuiltinMethodDef MoveX;
    const BuiltinMethodDef MoveY;
    const BuiltinMethodDef MoveZ;
    const BuiltinMethodDef RotateVelocity;
    const BuiltinMethodDef PlayRumbleOnEntity;
    const BuiltinMethodDef PlayRumbleLoopOnEntity;
    const BuiltinMethodDef StopRumble;
    const BuiltinMethodDef SetControllerLightbarColor;
    const BuiltinMethodDef StartPoisoning;
    const BuiltinMethodDef StopPoisoning;
    const BuiltinMethodDef IsPoisoned;
    const BuiltinMethodDef IsSplitScreenHost;
    /*
      Also called `GetStance`. Calls a different function than the prior
      `GetStance` field called `CScrCmd_GetStance`.

      In practice, this function will never be used -
      the prior `GetStance` function will be found first in lookup,
      so will be the only `GetStance` method in this table that is used.
    */
    const BuiltinMethodDef GetStance2;
    const BuiltinMethodDef GetWeaponPosFrac;
    const BuiltinMethodDef SetDamageDirectionIndicator;
    const BuiltinMethodDef AddAwarenessIndicator;
    const BuiltinMethodDef UseAlternateHud;
    const BuiltinMethodDef SetInTacticalHud;
    const BuiltinMethodDef GetCybercomType;
    const BuiltinMethodDef GetPlayerCorpse;
    const BuiltinMethodDef IsLocalPlayerWeaponViewOnlyLinked;
    const BuiltinMethodDef IsContentScreenFilterPlaying;
    const BuiltinMethodDef GetCamPos;
    const BuiltinMethodDef GetCamAngles;
    const BuiltinMethodDef UseAnimTree;
    const BuiltinMethodDef HasAnimTree;
    const BuiltinMethodDef ClearAnim;
    const BuiltinMethodDef ClearAnimLimited;
    const BuiltinMethodDef SetAnim;
    const BuiltinMethodDef SetAnimLimited;
    const BuiltinMethodDef SetAnimRestart;
    const BuiltinMethodDef SetAnimLimitedRestart;
    const BuiltinMethodDef SetFlaggedAnim;
    const BuiltinMethodDef SetFlaggedAnimLimited;
    const BuiltinMethodDef SetFlaggedAnimRestart;
    const BuiltinMethodDef SetFlaggedAnimLimitedRestart;
    const BuiltinMethodDef Animscripted;
    const BuiltinMethodDef GetCurrentAnimScriptedName;
    const BuiltinMethodDef SetHideWhenScriptedAnimationCompleted;
    const BuiltinMethodDef SetAnimKnob;
    const BuiltinMethodDef SetAnimKnobLimited;
    const BuiltinMethodDef SetAnimKnobRestart;
    const BuiltinMethodDef SetAnimKnobLimitedRestart;
    const BuiltinMethodDef SetAnimKnobAll;
    const BuiltinMethodDef SetAnimKnobAllLimited;
    const BuiltinMethodDef SetAnimKnobAllRestart;
    const BuiltinMethodDef SetAnimKnobAllLimitedRestart;
    const BuiltinMethodDef SetFlaggedAnimKnob;
    const BuiltinMethodDef SetFlaggedAnimKnobLimited;
    const BuiltinMethodDef SetFlaggedAnimKnobRestart;
    const BuiltinMethodDef SetFlaggedAnimKnobLimitedRestart;
    const BuiltinMethodDef SetFlaggedAnimKnobAll;
    const BuiltinMethodDef SetFlaggedAnimKnobAllRestart;
    const BuiltinMethodDef GetAnimTime;
    const BuiltinMethodDef SetAnimTime;
    const BuiltinMethodDef SetAnimTimeByName;
    const BuiltinMethodDef GetAnimCurrFrameCount;
    const BuiltinMethodDef SetEntityAnimRate;
    const BuiltinMethodDef GetEntityAnimRate;
    const BuiltinMethodDef AnimGetChildAt;
    const BuiltinMethodDef AnimGetNumChildren;
    const BuiltinMethodDef GetPrimaryDeltaAnim;
    const BuiltinMethodDef SetInfraredVisionSet;
    const BuiltinMethodDef SetUnderwaterVisionSet;
    const BuiltinMethodDef GetClientTime;
    const BuiltinMethodDef GetEye;
    const BuiltinMethodDef GetEyeApprox;
    const BuiltinMethodDef GetLocalClientNumber;
    const BuiltinMethodDef MapShaderConstant;
    const BuiltinMethodDef SetShaderConstant;
    const BuiltinMethodDef SetForceNotSimple;
    const BuiltinMethodDef ProcessClientFieldsAsIfNew;
    const BuiltinMethodDef Camera_Set_Lens_ID;
    const BuiltinMethodDef CameraSetPosition;
    const BuiltinMethodDef CameraSetLookAt;
    const BuiltinMethodDef CameraSetUpdateCallback;
    const BuiltinMethodDef CameraForceDisableScriptCam;
    const BuiltinMethodDef GadgetPulseRevealed;
    const BuiltinMethodDef GadgetPulseResetReveal;
    const BuiltinMethodDef GadgetPulseGetOwner;
    const BuiltinMethodDef GetTargetLockEntity;
    const BuiltinMethodDef GetTargetLockEntityArray;
    const BuiltinMethodDef GetTagOrigin;
    const BuiltinMethodDef GetTagAngles;
    const BuiltinMethodDef GetInKillcam;
    const BuiltinMethodDef GetKillCamEntity;
    const BuiltinMethodDef GetOwner;
    const BuiltinMethodDef SetRenderInThirdPersonSpectate;
    const BuiltinMethodDef SetWaterDisturbanceParams;
    const BuiltinMethodDef GetAnimStateCategory;
    const BuiltinMethodDef GetHeliDamageState;
    const BuiltinMethodDef IsRemoteControlling;
    const BuiltinMethodDef OverrideLightingOrigin;
    const BuiltinMethodDef HasPerk;
    const BuiltinMethodDef GetPerks;
    const BuiltinMethodDef GetStance;
    const BuiltinMethodDef ShellShock;
    const BuiltinMethodDef PlayerSetGroundReferenceEnt;
    const BuiltinMethodDef Earthquake;
    const BuiltinMethodDef GetLinkedEnt;
    const BuiltinMethodDef SetEnemyGlobalScrambler;
    const BuiltinMethodDef SetEnemyScramblerAmount;
    const BuiltinMethodDef GetEnemyScramblerAmount;
    const BuiltinMethodDef IsScrambled;
    const BuiltinMethodDef SetFriendlyScramblerAmount;
    const BuiltinMethodDef GetFriendlyScramblerAmount;
    const BuiltinMethodDef AddFriendlyScrambler;
    const BuiltinMethodDef ClearNearestEnemyScrambler;
    const BuiltinMethodDef SetNearestEnemyScrambler;
    const BuiltinMethodDef RemoveFriendlyScrambler;
    const BuiltinMethodDef RemoveAllFriendlyScramblers;
    const BuiltinMethodDef AddSensorGrenadeArea;
    const BuiltinMethodDef RemoveSensorGrenadeArea;
    const BuiltinMethodDef RemoveAllSensorGrenadeAreas;
    const BuiltinMethodDef SetFlagAsAway;
    const BuiltinMethodDef GetParentEntity;
    const BuiltinMethodDef SetBodyRenderOptions;
    const BuiltinMethodDef GetBodyRenderOptionsPacked;
    const BuiltinMethodDef SetBodyRenderOptionsPacked;
    const BuiltinMethodDef GetCharacterBodyType;
    const BuiltinMethodDef SetCorpseGibState;
    const BuiltinMethodDef UseWeaponHideTags;
    const BuiltinMethodDef UseWeaponModel;
    const BuiltinMethodDef UseBuildKitWeaponModel;
    const BuiltinMethodDef UseAlternateAimParams;
    const BuiltinMethodDef ClearAlternateAimParams;
    const BuiltinMethodDef SetSonarAttachmentEnabled;
    const BuiltinMethodDef ZBarrierGetPiece;
    const BuiltinMethodDef GetNumZBarrierPieces;
    const BuiltinMethodDef SetPlayerCybercomAbility;
    const BuiltinMethodDef GetCyberComAbilityName;
    const BuiltinMethodDef IsCybercomIndexEnabled;
    const BuiltinMethodDef ASMIsTerminating;
    const BuiltinMethodDef ASMIsShootLayerActive;
    const BuiltinMethodDef ASMIsAimLayerActive;
    const BuiltinMethodDef ASMGetStatus;
    const BuiltinMethodDef ASMGetCurrentState;
    const BuiltinMethodDef GetAIFxName;
    const BuiltinMethodDef SetEntBeastModeIconType;
    const BuiltinMethodDef GetMpDialogName;
    const BuiltinMethodDef GetTopPlayersIndex;
    const BuiltinMethodDef GetPlayerGibDef;
    const BuiltinMethodDef FindDRFilter;
    const BuiltinMethodDef HidePart;
    const BuiltinMethodDef HasPart;
    const BuiltinMethodDef ShowPart;
    const BuiltinMethodDef ShowAllParts;
    const BuiltinMethodDef ShowViewLegs;
    const BuiltinMethodDef HideViewLegs;
    const BuiltinMethodDef SuppressRagdollSelfCollision;
  };
  const BuiltinMethodDef methods[246];

  static constexpr const char *names[246] = {
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
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(BuiltinMethodTable, sizeof(BuiltinMethodTable::methods));

union GfxFunctionTable {
  struct {
    const BuiltinFunctionDef PlayFX;
    const BuiltinFunctionDef PlayFXOnTag;
    const BuiltinFunctionDef PlayTagFXSet;
    const BuiltinFunctionDef PlayFXOnCamera;
    const BuiltinFunctionDef PlayLoopedFX;
    const BuiltinFunctionDef StopFX;
    const BuiltinFunctionDef KillFX;
    const BuiltinFunctionDef IsFXPlaying;
    const BuiltinFunctionDef SetFXOutdoor;
    const BuiltinFunctionDef BulletTracer;
    const BuiltinFunctionDef TriggerFX;
    const BuiltinFunctionDef SetFXTeam;
    const BuiltinFunctionDef RegisterRewindFX;
    const BuiltinFunctionDef SetFXIgnorePause;
    const BuiltinFunctionDef PlayRadiantExploder;
    const BuiltinFunctionDef StopRadiantExploder;
    const BuiltinFunctionDef KillRadiantExploder;
    const BuiltinFunctionDef BeamLaunch;
    const BuiltinFunctionDef BeamKill;
    const BuiltinFunctionDef VisionSetNaked;
    const BuiltinFunctionDef VisionSetNakedLerp;
    const BuiltinFunctionDef GetVisionSetNaked;
    const BuiltinFunctionDef VisionSetLastStandLerp;
    const BuiltinFunctionDef GetFXFromSurfaceTable;
    const BuiltinFunctionDef SetRippleWave;
    const BuiltinFunctionDef ViewmodelHasTag;
    const BuiltinFunctionDef PlayViewmodelFX;
    const BuiltinFunctionDef PlayFXOnDynEnt;
    const BuiltinFunctionDef SpawnFX;
    const BuiltinFunctionDef DeleteFX;
    const BuiltinFunctionDef SetRimIntensity;
    const BuiltinFunctionDef SetGenericSceneValue;
    const BuiltinFunctionDef StartWaterSheetingFX;
    const BuiltinFunctionDef StopWaterSheetingFX;
    const BuiltinFunctionDef AddBoltedFXExclusionVolume;
    const BuiltinFunctionDef AddFXExclusionVolume;
    const BuiltinFunctionDef RemoveFXExclusionVolume;
    const BuiltinFunctionDef PlayMainCamXCam;
    const BuiltinFunctionDef StopMainCamXCam;
    const BuiltinFunctionDef SetAllowXCamRightStickRotation;
    const BuiltinFunctionDef GetWeaponXCam;
    const BuiltinFunctionDef SetClientVolumetricFog;
    const BuiltinFunctionDef SwitchToServerVolumetricFog;
    const BuiltinFunctionDef SwitchToClientVolumetricFog;
    const BuiltinFunctionDef SetServerVolumetricFogDensity;
    const BuiltinFunctionDef GetServerVolumetricFogDensity;
    const BuiltinFunctionDef SetExposureActiveBank;
    const BuiltinFunctionDef SetExposureIgnoreTeleport;
    const BuiltinFunctionDef SetExposureInstantSnap;
    const BuiltinFunctionDef SetLutVolumeActiveBank;
    const BuiltinFunctionDef SetLutScriptIndex;
    const BuiltinFunctionDef SetWorldFogActiveBank;
    const BuiltinFunctionDef GetWorldFogScriptID;
    const BuiltinFunctionDef FindStaticModelIndex;
    const BuiltinFunctionDef FindStaticModelIndexArray;
    const BuiltinFunctionDef HideStaticModel;
    const BuiltinFunctionDef UnhideStaticModel;
    const BuiltinFunctionDef FindVolumeDecalIndexArray;
    const BuiltinFunctionDef HideVolumeDecal;
    const BuiltinFunctionDef UnhideVolumeDecal;
    const BuiltinFunctionDef Print;
    const BuiltinFunctionDef Print3D;
    const BuiltinFunctionDef PrintLn;
    const BuiltinFunctionDef PrintTopRightln;
    const BuiltinFunctionDef IPrintLnBold;
    const BuiltinFunctionDef Line;
    const BuiltinFunctionDef Box;
    const BuiltinFunctionDef DebugStar;
    const BuiltinFunctionDef Circle;
    const BuiltinFunctionDef Sphere;
    const BuiltinFunctionDef IsStereoOn;
    const BuiltinFunctionDef GetSurfaceStrings;
    const BuiltinFunctionDef IsSplitScreenHost;
    const BuiltinFunctionDef IsSplitScreen;
    const BuiltinFunctionDef SetLitFogBank;
    const BuiltinFunctionDef SetUkkoScriptIndex;
    const BuiltinFunctionDef SetPBGActiveBank;
    const BuiltinFunctionDef SetFilterPassMaterial;
    const BuiltinFunctionDef SetFilterPassCodeTexture;
    const BuiltinFunctionDef SetFilterPassEnabled;
    const BuiltinFunctionDef SetFilterPassQuads;
    const BuiltinFunctionDef SetFilterPassConstant;
    const BuiltinFunctionDef SetFilterBitFlag;
    const BuiltinFunctionDef SetOverlayEnabled;
    const BuiltinFunctionDef SetOverlayMaterial;
    const BuiltinFunctionDef SetOverlayConstant;
    const BuiltinFunctionDef EnableThermalDraw;
    const BuiltinFunctionDef CreateSceneCodeImage;
    const BuiltinFunctionDef FreeCodeImage;
    const BuiltinFunctionDef CaptureFrame;
    const BuiltinFunctionDef MapMaterialIndex;
    const BuiltinFunctionDef ZombieShooterSpotted;
    const BuiltinFunctionDef TmodeStatus;
    const BuiltinFunctionDef EVStatus;
    const BuiltinFunctionDef IGCActive;
    const BuiltinFunctionDef IsIGCActive;
    const BuiltinFunctionDef EnableFrontendStreamingOverlay;
    const BuiltinFunctionDef EnableFrontendLockedWeaponOverlay;
    const BuiltinFunctionDef EnableFrontendTokenLockedWeaponOverlay;
    const BuiltinFunctionDef UmbraGate_Set;
    const BuiltinFunctionDef Umbra_SetDistanceScale;
    const BuiltinFunctionDef Umbra_SetAccurateOcclusionThreshold;
    const BuiltinFunctionDef Umbra_SetMinimumContributionThreshold;
    const BuiltinFunctionDef Umbra_SetTomeTrigger;
    const BuiltinFunctionDef Umbra_SetTomeTriggerOnce;
    const BuiltinFunctionDef Umbra_ClearPersistentTomeTrigger;
  };
  const BuiltinFunctionDef functions[106];

  static constexpr const char *names[106] = {
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
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(GfxFunctionTable, sizeof(GfxFunctionTable::functions));

union GfxMethodTable {
  struct {
    const BuiltinMethodDef OED_Nightvision_Reset;
    const BuiltinMethodDef OED_Nightvision_SetTextureLevel;
    const BuiltinMethodDef OED_Nightvision_Enable;
    const BuiltinMethodDef OED_SitRepScan_Enable;
    const BuiltinMethodDef OED_SitRepScan_SetDesat;
    const BuiltinMethodDef OED_SitRepScan_SetSolid;
    const BuiltinMethodDef HE_Intro_Civilian;
    const BuiltinMethodDef OED_SitRepScan_SetLineWidth;
    const BuiltinMethodDef OED_SitRepScan_SetRadius;
    const BuiltinMethodDef OED_SitRepScan_SetFallOff;
    const BuiltinMethodDef CODCaster_Keyline_Enable;
    const BuiltinMethodDef RandomFade;
    const BuiltinMethodDef Weakpoint_Enable;
    const BuiltinMethodDef TmodeEnable;
    const BuiltinMethodDef EVEnable;
    const BuiltinMethodDef EACPathSet;
    const BuiltinMethodDef DragonStrike_Enable;
    const BuiltinMethodDef DragonStrike_SetPosition;
    const BuiltinMethodDef DragonStrike_SetColorRadiusSpinPulse;
    const BuiltinMethodDef MotionPulse_Enable;
    const BuiltinMethodDef MotionPulse_SetDesat;
    const BuiltinMethodDef MotionPulse_SetSolid;
    const BuiltinMethodDef MotionPulse_SetOutline;
    const BuiltinMethodDef MotionPulse_SetLineWidth;
    const BuiltinMethodDef MotionPulse_SetRadius;
    const BuiltinMethodDef MotionPulse_SetFallOff;
    const BuiltinMethodDef MotionPulse_SetFadeOut;
    const BuiltinMethodDef SetBlur;
    const BuiltinMethodDef SetExtraCam;
    const BuiltinMethodDef SetExtraCamFocalLength;
    const BuiltinMethodDef ClearExtraCam;
    const BuiltinMethodDef PlayExtraCamXCam;
    const BuiltinMethodDef StopExtraCamXCam;
    const BuiltinMethodDef SetWeaponRenderOptions;
    const BuiltinMethodDef SetAttachmentCosmeticVariantIndex;
    const BuiltinMethodDef AddDuplicateRenderOption;
    const BuiltinMethodDef DisableDuplicateRendering;
    const BuiltinMethodDef SetArmPulse;
    const BuiltinMethodDef SetArmPulsePosition;
    const BuiltinMethodDef SetHotZone;
    const BuiltinMethodDef TmodeSetFlag;
    const BuiltinMethodDef TmodeClearFlag;
  };
  const BuiltinMethodDef methods[42];
  static constexpr const char *names[42] = {
      "OED_Nightvision_Reset",
      "OED_Nightvision_SetTextureLevel",
      "OED_Nightvision_Enable",
      "OED_SitRepScan_Enable",
      "OED_SitRepScan_SetDesat",
      "OED_SitRepScan_SetSolid",
      "HE_Intro_Civilian",
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
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(GfxMethodTable, sizeof(GfxMethodTable::methods));

union MathFunctionTable {
  struct {
    const BuiltinFunctionDef RandomInt;
    const BuiltinFunctionDef RandomFloat;
    const BuiltinFunctionDef RandomIntRange;
    const BuiltinFunctionDef RandomFloatRange;
    const BuiltinFunctionDef Project2DTo3D;
    const BuiltinFunctionDef Project3DTo2D;
    const BuiltinFunctionDef ViewAspect;
    const BuiltinFunctionDef GetAngleFromBits;
    const BuiltinFunctionDef GetBitsForAngle;
  };
  const BuiltinFunctionDef functions[9];

  static constexpr const char *names[9] = {
      "RandomInt",        "RandomFloat",      "RandomIntRange",
      "RandomFloatRange", "Project2DTo3D",    "Project3DTo2D",
      "ViewAspect",       "GetAngleFromBits", "GetBitsForAngle",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(MathFunctionTable, sizeof(MathFunctionTable::functions));

union SoundFunctionTable {
  struct {
    const BuiltinFunctionDef PlaySound;
    const BuiltinFunctionDef GetSoundFromSurfaceTable;
    const BuiltinFunctionDef SoundPlaying;
    const BuiltinFunctionDef SetSoundVolume;
    const BuiltinFunctionDef SetSoundPitch;
    const BuiltinFunctionDef SetSoundVolumeRate;
    const BuiltinFunctionDef SetSoundPitchRate;
    const BuiltinFunctionDef StopSound;
    const BuiltinFunctionDef GetRealTime;
    const BuiltinFunctionDef GetPlaybackTime;
    const BuiltinFunctionDef GetKnownLength;
    const BuiltinFunctionDef SetGroupSnapshot;
    const BuiltinFunctionDef SetGlobalFutz;
    const BuiltinFunctionDef SetAmbientSnapshot;
    const BuiltinFunctionDef SetHealthSnapshot;
    const BuiltinFunctionDef SetLevelFadeSnapshot;
    const BuiltinFunctionDef SoundSetMusicState;
    const BuiltinFunctionDef SoundLoopEmitter;
    const BuiltinFunctionDef SoundRattleSetup;
    const BuiltinFunctionDef SoundRattle;
    const BuiltinFunctionDef SoundPlayAutoFX;
    const BuiltinFunctionDef SetSoundContext;
    const BuiltinFunctionDef SoundStopLoopEmitter;
    const BuiltinFunctionDef SoundLineEmitter;
    const BuiltinFunctionDef SoundStopLineEmitter;
    const BuiltinFunctionDef SoundUpdateLineEmitter;
    const BuiltinFunctionDef StopLocalSound;
    const BuiltinFunctionDef SoundTimeScale;
    const BuiltinFunctionDef AllocateSoundRandoms;
    const BuiltinFunctionDef CreateSoundRandom;
    const BuiltinFunctionDef ForceAmbientRoom;
  };
  const BuiltinFunctionDef functions[31];

  static constexpr const char *names[31] = {
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
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(SoundFunctionTable, sizeof(SoundFunctionTable::functions));

union SoundMethodTable {
  struct {
    const BuiltinMethodDef PlaySound;
    const BuiltinMethodDef StopSounds;
    const BuiltinMethodDef PlayLoopSound;
    const BuiltinMethodDef StopLoopSound;
    const BuiltinMethodDef StopAllLoopSounds;
    const BuiltinMethodDef IsPlayingLoopSound;
    const BuiltinMethodDef SetSoundEntContext;
    const BuiltinMethodDef SetLoopState;
  };
  const BuiltinMethodDef methods[8];

  static constexpr const char *names[8] = {
      "PlaySound",          "StopSounds",        "PlayLoopSound",
      "StopLoopSound",      "StopAllLoopSounds", "IsPlayingLoopSound",
      "SetSoundEntContext", "SetLoopState",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(SoundMethodTable, sizeof(SoundMethodTable::methods));

union UIFunctionTable {
  struct {
    const BuiltinFunctionDef LUILoad;
    const BuiltinFunctionDef LUIEnable;
    const BuiltinFunctionDef LUIDisable;
    const BuiltinFunctionDef IsLUIEnabled;
    const BuiltinFunctionDef CreateLUIMenu;
    const BuiltinFunctionDef OpenLUIMenu;
    const BuiltinFunctionDef GetLUIMenu;
    const BuiltinFunctionDef SetLUIMenuData;
    const BuiltinFunctionDef GetLUIMenuData;
    const BuiltinFunctionDef OpenLUIPopup;
    const BuiltinFunctionDef CloseLUIMenu;
    const BuiltinFunctionDef PopulateScriptDebugMenu;
    const BuiltinFunctionDef CreateUIModel;
    const BuiltinFunctionDef FreeUIModel;
    const BuiltinFunctionDef GetGlobalUIModel;
    const BuiltinFunctionDef GetUIModel;
    const BuiltinFunctionDef GetUIModelForController;
    const BuiltinFunctionDef GetUIModelValue;
    const BuiltinFunctionDef SetUIModelValue;
    const BuiltinFunctionDef StopSound;
  };
  const BuiltinFunctionDef functions[20];

  static constexpr const char *names[20] = {
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
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(UIFunctionTable, sizeof(UIFunctionTable::functions));

union UtilFunctionTable {
  struct {
    const BuiltinFunctionDef ReportStubUsage;
    const BuiltinFunctionDef PIXBeginEvent;
    const BuiltinFunctionDef PIXEndEvent;
    const BuiltinFunctionDef HasDLCAvailable;
    const BuiltinFunctionDef IsMatureContentEnabled;
    const BuiltinFunctionDef IsShowBloodEnabled;
    const BuiltinFunctionDef IsShowGibsEnabled;
    const BuiltinFunctionDef SplitArgs;
    const BuiltinFunctionDef ReCacheLeaderboards;
    const BuiltinFunctionDef ClientHasSnapShot;
    const BuiltinFunctionDef GetSystemTime;
    const BuiltinFunctionDef GetServerTime;
    const BuiltinFunctionDef GetMapCenter;
    const BuiltinFunctionDef GetSkiptoName;
    const BuiltinFunctionDef GetSkipTOS;
    const BuiltinFunctionDef GetUTC;
    const BuiltinFunctionDef SetDvar;
    const BuiltinFunctionDef SetSavedDvar;
    const BuiltinFunctionDef GetLightingState;
  };
  const BuiltinFunctionDef functions[19];

  static constexpr const char *names[19] = {
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
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(BuiltinFunctionTable, sizeof(BuiltinFunctionTable::functions));

union UtilMethodTable {
  struct {
    const BuiltinMethodDef ButtonPressed;
    // `PlayerCmd` variant of the method of same name in `BuiltinMethodTable`
    const BuiltinMethodDef GetControllerPosition;
  };
  const BuiltinMethodDef methods[2];
  static constexpr const char *names[2] = {
      "ButtonPressed",
      "GetControllerPosition",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(UtilMethodTable, sizeof(UtilMethodTable::methods));

} // namespace cscr
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game