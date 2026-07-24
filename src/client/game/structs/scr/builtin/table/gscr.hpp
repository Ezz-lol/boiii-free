#pragma once

#include "game/structs/scr/builtin/core.hpp"
#include "game/structs/scr/builtin/table/macros.hpp"

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace gscr {
union BuiltinFunctionTable {
  struct {
    const BuiltinFunctionDef Spawn;
    const BuiltinFunctionDef SpawnCollision;
    const BuiltinFunctionDef SpawnTimedFX;
    const BuiltinFunctionDef SpawnActor;
    const BuiltinFunctionDef SpawnVehicle;
    const BuiltinFunctionDef SpawnPlane;
    const BuiltinFunctionDef SpawnTurret;
    const BuiltinFunctionDef AddStruct;
    const BuiltinFunctionDef SpawnHelicopter;
    const BuiltinFunctionDef GetEntByNum;
    const BuiltinFunctionDef GetBrushModelCenter;
    const BuiltinFunctionDef GetDemoFileID;
    const BuiltinFunctionDef GetPlayers;
    const BuiltinFunctionDef CountPlayers;
    const BuiltinFunctionDef GetDamageableEntArray;
    const BuiltinFunctionDef MaySpawnFakeEntity;
    const BuiltinFunctionDef MaySpawnEntity;
    const BuiltinFunctionDef Objective_Add;
    const BuiltinFunctionDef Objective_Delete;
    const BuiltinFunctionDef Objective_State;
    const BuiltinFunctionDef Objective_Icon;
    const BuiltinFunctionDef Objective_Position;
    const BuiltinFunctionDef Objective_OnEntity;
    const BuiltinFunctionDef Objective_ClearEntity;
    const BuiltinFunctionDef Objective_SetVisibleToPlayer;
    const BuiltinFunctionDef Objective_SetInvisibleToPlayer;
    const BuiltinFunctionDef Objective_SetVisibleToPlayerByIndex;
    const BuiltinFunctionDef Objective_SetInvisibleToPlayerByIndex;
    const BuiltinFunctionDef Objective_SetVisibleToAll;
    const BuiltinFunctionDef Objective_SetInvisibleToAll;
    const BuiltinFunctionDef Objective_SetSize;
    const BuiltinFunctionDef Objective_SetColor;
    const BuiltinFunctionDef Objective_SetProgress;
    const BuiltinFunctionDef Objective_SetGamemodeFlags;
    const BuiltinFunctionDef Objective_SetFlag;
    const BuiltinFunctionDef Objective_GetGamemodeFlags;
    const BuiltinFunctionDef Objective_SetPlayerUsing;
    const BuiltinFunctionDef Objective_ClearPlayerUsing;
    const BuiltinFunctionDef Objective_ClearAllUsing;
    const BuiltinFunctionDef Objective_Set3D;
    const BuiltinFunctionDef Objective_Team;
    const BuiltinFunctionDef Objective_VisibleTeams;
    const BuiltinFunctionDef Objective_SetUIModelValue;
    const BuiltinFunctionDef Missile_CreateAttractorEnt;
    const BuiltinFunctionDef Missile_CreateAttractorOrigin;
    const BuiltinFunctionDef Missile_CreateRepulsorEnt;
    const BuiltinFunctionDef Missile_CreateRepulsorOrigin;
    const BuiltinFunctionDef Missile_DeleteAttractor;
    const BuiltinFunctionDef BulletTrace;
    const BuiltinFunctionDef GroundTrace;
    const BuiltinFunctionDef BeamTrace;
    const BuiltinFunctionDef BulletTracePassed;
    const BuiltinFunctionDef SightTracePassed;
    const BuiltinFunctionDef PhysicsTrace;
    const BuiltinFunctionDef PlayerPhysicsTrace;
    const BuiltinFunctionDef PlayerPhysicsTraceIgnoreEnt;
    const BuiltinFunctionDef PlayerGrappleTrace;
    const BuiltinFunctionDef PlayerBulletTrace;
    const BuiltinFunctionDef WorldTrace;
    const BuiltinFunctionDef PlayerPositionValid;
    const BuiltinFunctionDef PlayerPositionValidIgnoreEnt;
    const BuiltinFunctionDef GetMoveDelta;
    const BuiltinFunctionDef GetAngleDelta;
    const BuiltinFunctionDef GetNorthYaw;
    const BuiltinFunctionDef PlayFX;
    const BuiltinFunctionDef PlayFXOnTag;
    const BuiltinFunctionDef PlayFXOnCamera;
    const BuiltinFunctionDef GetWaterHeight;
    const BuiltinFunctionDef PlayLoopedFX;
    const BuiltinFunctionDef SpawnFX;
    const BuiltinFunctionDef TriggerFX;
    const BuiltinFunctionDef FXBlockSight;
    const BuiltinFunctionDef ResetGlass;
    const BuiltinFunctionDef ShatterAllGlass;
    const BuiltinFunctionDef PhysicsExplosionSphere;
    const BuiltinFunctionDef PhysicsExplosionCylinder;
    const BuiltinFunctionDef PhysicsJolt;
    const BuiltinFunctionDef PhysicsJetThrust;
    const BuiltinFunctionDef CreateStreamerHint;
    const BuiltinFunctionDef AreTexturesLoaded;
    const BuiltinFunctionDef SetExpFog;
    const BuiltinFunctionDef SetVolFog;
    const BuiltinFunctionDef SetCullDist;
    const BuiltinFunctionDef GrenadeExplosionEffect;
    const BuiltinFunctionDef MagicBullet;
    const BuiltinFunctionDef RadiusDamage;
    const BuiltinFunctionDef SetPlayerIgnoreRadiusDamage;
    const BuiltinFunctionDef GlassRadiusDamage;
    const BuiltinFunctionDef GetNumParts;
    const BuiltinFunctionDef GetPartName;
    const BuiltinFunctionDef Earthquake;
    const BuiltinFunctionDef ScreenShake;
    const BuiltinFunctionDef BulletSpread;
    const BuiltinFunctionDef NewHudElem;
    const BuiltinFunctionDef NewClientHudElem;
    const BuiltinFunctionDef NewDamageIndicatorHudElem;
    const BuiltinFunctionDef NewTeamHudElem;
    const BuiltinFunctionDef NewScoreHudElem;
    const BuiltinFunctionDef NewDebugHudElem;
    const BuiltinFunctionDef ResetTimeOut;
    const BuiltinFunctionDef GetAttachmentNames;
    const BuiltinFunctionDef GetWatcherWeapons;
    const BuiltinFunctionDef GetRetrievableWeapons;
    const BuiltinFunctionDef IsLaserOn;
    const BuiltinFunctionDef IsTurretFiring;
    const BuiltinFunctionDef WeaponHasAttachment;
    const BuiltinFunctionDef GetWeaponAttachments;
    const BuiltinFunctionDef IsItemRestricted;
    const BuiltinFunctionDef GetEquipmentHeadObjective;
    const BuiltinFunctionDef GetCrateHeadObjective;
    const BuiltinFunctionDef IsPlayerNumber;
    const BuiltinFunctionDef SetWinningPlayer;
    const BuiltinFunctionDef SetWinningTeam;
    const BuiltinFunctionDef Announcement;
    const BuiltinFunctionDef ClientAnnouncement;
    const BuiltinFunctionDef GetTeamScore;
    const BuiltinFunctionDef SetTeamScore;
    const BuiltinFunctionDef TeamOpsShowHUD;
    const BuiltinFunctionDef TeamOpsStart;
    const BuiltinFunctionDef TeamOpsUpdateProgress;
    const BuiltinFunctionDef SetClientNameMode;
    const BuiltinFunctionDef UpdateClientNames;
    const BuiltinFunctionDef GetTeamPlayersAlive;
    const BuiltinFunctionDef GetDroppedWeapons;
    const BuiltinFunctionDef ArtilleryIconLocation;
    const BuiltinFunctionDef LogPrint;
    const BuiltinFunctionDef WorldEntNumber;
    const BuiltinFunctionDef Obituary;
    const BuiltinFunctionDef ReviveObituary;
    const BuiltinFunctionDef AddDemoBookmark;
    const BuiltinFunctionDef StopDemoRecording;
    const BuiltinFunctionDef PositionWouldTelefrag;
    const BuiltinFunctionDef BoundsWouldTelefrag;
    const BuiltinFunctionDef RecordUsedSpawnPoint;
    const BuiltinFunctionDef TestSpawnPoint;
    const BuiltinFunctionDef GetStartTime;
    const BuiltinFunctionDef MapRestart;
    const BuiltinFunctionDef MissionRestart;
    const BuiltinFunctionDef MissionFailed;
    const BuiltinFunctionDef ExitLevel;
    const BuiltinFunctionDef KillServer;
    const BuiltinFunctionDef AddTestClient;
    const BuiltinFunctionDef SetObjectivePointStatus;
    const BuiltinFunctionDef SetBombTimer;
    const BuiltinFunctionDef SetRoundsPlayed;
    const BuiltinFunctionDef SetInitialPlayersConnected;
    const BuiltinFunctionDef SetMatchFlag;
    const BuiltinFunctionDef SetMatchTalkFlag;
    const BuiltinFunctionDef SetArchive;
    const BuiltinFunctionDef AllClientsPrint;
    const BuiltinFunctionDef ClientPrint;
    const BuiltinFunctionDef MapExists;
    const BuiltinFunctionDef IsValidGameType;
    const BuiltinFunctionDef SkillUpdate;
    const BuiltinFunctionDef RecordLeagueWinner;
    const BuiltinFunctionDef SetTeamSpyplane;
    const BuiltinFunctionDef GetTeamSpyplane;
    const BuiltinFunctionDef SetTeamSatellite;
    const BuiltinFunctionDef GetTeamSatellite;
    const BuiltinFunctionDef GetAssignedTeam;
    const BuiltinFunctionDef GetAssignedTeamName;
    const BuiltinFunctionDef GetDefaultClassSlot;
    const BuiltinFunctionDef GetItemAttachment;
    const BuiltinFunctionDef GetItemAttachmentAllocationCost;
    const BuiltinFunctionDef GetRefFromItemIndex;
    const BuiltinFunctionDef GetItemGroupFromItemIndex;
    const BuiltinFunctionDef GetBaseWeaponItemIndex;
    const BuiltinFunctionDef GetGameTypeEnumFromName;
    const BuiltinFunctionDef SetScoreboardColumns;
    const BuiltinFunctionDef SetTopScorer;
    const BuiltinFunctionDef ClearTopScorers;
    const BuiltinFunctionDef RecordNumZombieRounds;
    const BuiltinFunctionDef RecordGameResult;
    const BuiltinFunctionDef FinalizeMatchRecord;
    const BuiltinFunctionDef MatchRecordRoundEnd;
    const BuiltinFunctionDef MatchRecordRoundStart;
    const BuiltinFunctionDef MatchRecordOvertimeRound;
    const BuiltinFunctionDef GetTouchingVolume;
    const BuiltinFunctionDef GetFirstTouchFraction;
    const BuiltinFunctionDef RecordMatchSummaryZombieEndGameData;
    const BuiltinFunctionDef MatchRecorderIncrementHeaderStat;
    const BuiltinFunctionDef RecordMatchInit;
    const BuiltinFunctionDef InvalidateMatchRecord;
    const BuiltinFunctionDef RecordZombieRoundStart;
    const BuiltinFunctionDef RecordZombieRoundEnd;
    const BuiltinFunctionDef GetCustomTeamName;
    const BuiltinFunctionDef ClearPlayerCorpses;
    const BuiltinFunctionDef RecordPlayerStats;
    const BuiltinFunctionDef RecordPlayerMatchEnd;
    const BuiltinFunctionDef RecordMatchBegin;
    const BuiltinFunctionDef RecordBreadcrumbDataForPlayer;
    const BuiltinFunctionDef MatchRecordNewPlayer;
    const BuiltinFunctionDef MatchRecordLogAdditionalDeathInfo;
    const BuiltinFunctionDef MatchRecordLogSpecialMoveDataForLife;
    const BuiltinFunctionDef MatchRecordLogChallengeComplete;
    const BuiltinFunctionDef MatchRecordSetCurrentLevelComplete;
    const BuiltinFunctionDef MatchRecordSetLevelDifficultyForIndex;
    const BuiltinFunctionDef MatchRecordGetWeaponIndex;
    const BuiltinFunctionDef SetVoteString;
    const BuiltinFunctionDef SetVoteTime;
    const BuiltinFunctionDef SetVoteYesCount;
    const BuiltinFunctionDef SetVoteNoCount;
    const BuiltinFunctionDef ArenaGetSlot;
    const BuiltinFunctionDef ArenaGetCurrentSeason;
    const BuiltinFunctionDef ReportMTU;
    const BuiltinFunctionDef Kick;
    const BuiltinFunctionDef Ban;
    const BuiltinFunctionDef Map;
    const BuiltinFunctionDef PlayRumbleOnPosition;
    const BuiltinFunctionDef PlayRumbleLoopOnPosition;
    const BuiltinFunctionDef StopAllRumbles;
    const BuiltinFunctionDef SoundExists;
    const BuiltinFunctionDef SoundGetPlaybackTime;
    const BuiltinFunctionDef SoundGetAlias;
    const BuiltinFunctionDef IsSplitscreen;
    const BuiltinFunctionDef IsLocalGame;
    const BuiltinFunctionDef IsGlobalStatsServer;
    const BuiltinFunctionDef SetMinimap;
    const BuiltinFunctionDef SetMapCenter;
    const BuiltinFunctionDef SetDemoIntermissionPoint;
    const BuiltinFunctionDef SetGameEndTime;
    const BuiltinFunctionDef SetSlowMotion;
    const BuiltinFunctionDef SetPauseWorld;
    const BuiltinFunctionDef IsWorldPaused;
    const BuiltinFunctionDef NumRemoteClients;
    const BuiltinFunctionDef VisionSetNaked;
    const BuiltinFunctionDef VisionSetLastStand;
    const BuiltinFunctionDef SetGravity;
    const BuiltinFunctionDef SetWaveWaterGeneratorAmplitude;
    const BuiltinFunctionDef EndLobby;
    const BuiltinFunctionDef GetLobbyClientCount;
    const BuiltinFunctionDef EnableLobbyJoins;
    const BuiltinFunctionDef ClientSysRegister;
    const BuiltinFunctionDef ClientSysSetState;
    const BuiltinFunctionDef GetSnapshotIndexArray;
    const BuiltinFunctionDef SnapshotAcknowledged;
    const BuiltinFunctionDef GetAITriggerFlags;
    const BuiltinFunctionDef GetMaxVehicles;
    const BuiltinFunctionDef DisableDestructiblePieces;
    const BuiltinFunctionDef EnableAllDestructiblePieces;
    const BuiltinFunctionDef CreateDynEntAndLaunch;
    const BuiltinFunctionDef GetVehicleTriggerFlags;
    const BuiltinFunctionDef GetEntNavMaterial;
    const BuiltinFunctionDef CollisionTestPointsInSphere;
    const BuiltinFunctionDef CollisionTestPointsInCylinder;
    const BuiltinFunctionDef CollisionTestPointsInPill;
    const BuiltinFunctionDef CollisionTestPointsInCone;
    const BuiltinFunctionDef CollisionTestPointsInBox;
    const BuiltinFunctionDef QSortScoredSpawnPointsAscending;
    const BuiltinFunctionDef Matrix4x4TransformPoints;
    const BuiltinFunctionDef EnableSpawnPointList;
    const BuiltinFunctionDef DisableSpawnPointList;
    const BuiltinFunctionDef SetSpawnPointRandomVariation;
    const BuiltinFunctionDef ClearSpawnPoints;
    const BuiltinFunctionDef PlaceSpawnPoint;
    const BuiltinFunctionDef AddSpawnPoints;
    const BuiltinFunctionDef GetBestSpawnPoint;
    const BuiltinFunctionDef ClearSpawnPointsBaseWeight;
    const BuiltinFunctionDef SetSpawnPointsBaseWeight;
    const BuiltinFunctionDef GetPlayerSpawnId;
    const BuiltinFunctionDef IsSpawnPointVisible;
    const BuiltinFunctionDef AddInfluencer;
    const BuiltinFunctionDef AddEntityInfluencer;
    const BuiltinFunctionDef AddOrientedInfluencer;
    const BuiltinFunctionDef RemoveInfluencer;
    const BuiltinFunctionDef EnableInfluencer;
    const BuiltinFunctionDef SetInfluencerTeamMask;
    const BuiltinFunctionDef SetInfluencerTimeOut;
    const BuiltinFunctionDef GetInfluencerTimeOutRemaining;
    const BuiltinFunctionDef SetDebugSideSwitch;
    const BuiltinFunctionDef GetInfluencerPreset;
    const BuiltinFunctionDef Target_Set;
    const BuiltinFunctionDef Target_SetOffset;
    const BuiltinFunctionDef Target_GetOffset;
    const BuiltinFunctionDef Target_Remove;
    const BuiltinFunctionDef Target_SetShader;
    const BuiltinFunctionDef Target_SetOffscreenShader;
    const BuiltinFunctionDef Target_IsInRect;
    const BuiltinFunctionDef Target_IsInCircle;
    const BuiltinFunctionDef Target_ScaleMinMaxRadius;
    const BuiltinFunctionDef Target_OriginIsInCircle;
    const BuiltinFunctionDef Target_BoundingIsUnderReticle;
    const BuiltinFunctionDef Target_StartReticleLockOn;
    const BuiltinFunctionDef Target_ClearReticleLockOn;
    const BuiltinFunctionDef Target_GetArray;
    const BuiltinFunctionDef Target_IsTarget;
    const BuiltinFunctionDef Target_SetAttackMode;
    const BuiltinFunctionDef Target_SetJavelinOnly;
    const BuiltinFunctionDef Target_SetTurretAquire;
    const BuiltinFunctionDef Target_SetAllowHighSteering;
    const BuiltinFunctionDef GetMaxActiveContracts;
    const BuiltinFunctionDef GetContractStatType;
    const BuiltinFunctionDef GetContractStatName;
    const BuiltinFunctionDef GetContractRewardXP;
    const BuiltinFunctionDef GetContractRewardCP;
    const BuiltinFunctionDef GetContractRequirements;
    const BuiltinFunctionDef GetContractName;
    const BuiltinFunctionDef GetContractRequiredCount;
    const BuiltinFunctionDef GetContractResetConditions;
    const BuiltinFunctionDef GetFogSettings;
    const BuiltinFunctionDef PIXBeginEvent;
    const BuiltinFunctionDef PIXEndEvent;
    const BuiltinFunctionDef PIXMarker;
    const BuiltinFunctionDef ChangeAdvertisedStatus;
    const BuiltinFunctionDef SetQoSGameDataPayload;
    const BuiltinFunctionDef ResetQoSGameDataPayload;
    const BuiltinFunctionDef IncrementCounter;
    const BuiltinFunctionDef GetCounterTotal;
    const BuiltinFunctionDef ForceUploadCounters;
    const BuiltinFunctionDef EnableOccluder;
    const BuiltinFunctionDef SetHostMigrationStatus;
    const BuiltinFunctionDef StartHostMigration;
    const BuiltinFunctionDef GameRepThresholdExceeded;
    const BuiltinFunctionDef ZeroGravityVolumeOn;
    const BuiltinFunctionDef ZeroGravityVolumeOff;
    const BuiltinFunctionDef GetUTC;
    const BuiltinFunctionDef IsUsingT7Melee;
    const BuiltinFunctionDef RegisterSkipto;
    const BuiltinFunctionDef GetSkiptoName;
    const BuiltinFunctionDef GetSkipTOS;
    const BuiltinFunctionDef SetSkipTOS;
    const BuiltinFunctionDef AIProfile_BeginEntry;
    const BuiltinFunctionDef AIProfile_EndEntry;
    const BuiltinFunctionDef StreamerModelHint;
    const BuiltinFunctionDef StreamerRequest;
    const BuiltinFunctionDef GetCyberComWeapon;
    const BuiltinFunctionDef GetCyberComAbilityName;
    const BuiltinFunctionDef CloneAndRemoveEntity;
    const BuiltinFunctionDef LoadSentientEventParameters;
    const BuiltinFunctionDef HkaiGetTimerTestPathfindStartPos;
    const BuiltinFunctionDef HkaiGetTimerTestPathfindEndPos;
    const BuiltinFunctionDef HkaiSetTimerTestEnt;
    const BuiltinFunctionDef GetTotalServerPauseTime;
    const BuiltinFunctionDef HashString;
    const BuiltinFunctionDef PROTECTED(SetBGBUnlocked);
    const BuiltinFunctionDef PROTECTED(GetBGBUnlocked);
  };
  const BuiltinFunctionDef functions[336];

  static constexpr const char *names[336] = {
      "Spawn",
      "SpawnCollision",
      "SpawnTimedFX",
      "SpawnActor",
      "SpawnVehicle",
      "SpawnPlane",
      "SpawnTurret",
      "AddStruct",
      "SpawnHelicopter",
      "GetEntByNum",
      "GetBrushModelCenter",
      "GetDemoFileID",
      "GetPlayers",
      "CountPlayers",
      "GetDamageableEntArray",
      "MaySpawnFakeEntity",
      "MaySpawnEntity",
      "Objective_Add",
      "Objective_Delete",
      "Objective_State",
      "Objective_Icon",
      "Objective_Position",
      "Objective_OnEntity",
      "Objective_ClearEntity",
      "Objective_SetVisibleToPlayer",
      "Objective_SetInvisibleToPlayer",
      "Objective_SetVisibleToPlayerByIndex",
      "Objective_SetInvisibleToPlayerByIndex",
      "Objective_SetVisibleToAll",
      "Objective_SetInvisibleToAll",
      "Objective_SetSize",
      "Objective_SetColor",
      "Objective_SetProgress",
      "Objective_SetGamemodeFlags",
      "Objective_SetFlag",
      "Objective_GetGamemodeFlags",
      "Objective_SetPlayerUsing",
      "Objective_ClearPlayerUsing",
      "Objective_ClearAllUsing",
      "Objective_Set3D",
      "Objective_Team",
      "Objective_VisibleTeams",
      "Objective_SetUIModelValue",
      "Missile_CreateAttractorEnt",
      "Missile_CreateAttractorOrigin",
      "Missile_CreateRepulsorEnt",
      "Missile_CreateRepulsorOrigin",
      "Missile_DeleteAttractor",
      "BulletTrace",
      "GroundTrace",
      "BeamTrace",
      "BulletTracePassed",
      "SightTracePassed",
      "PhysicsTrace",
      "PlayerPhysicsTrace",
      "PlayerPhysicsTraceIgnoreEnt",
      "PlayerGrappleTrace",
      "PlayerBulletTrace",
      "WorldTrace",
      "PlayerPositionValid",
      "PlayerPositionValidIgnoreEnt",
      "GetMoveDelta",
      "GetAngleDelta",
      "GetNorthYaw",
      "PlayFX",
      "PlayFXOnTag",
      "PlayFXOnCamera",
      "GetWaterHeight",
      "PlayLoopedFX",
      "SpawnFX",
      "TriggerFX",
      "FXBlockSight",
      "ResetGlass",
      "ShatterAllGlass",
      "PhysicsExplosionSphere",
      "PhysicsExplosionCylinder",
      "PhysicsJolt",
      "PhysicsJetThrust",
      "CreateStreamerHint",
      "AreTexturesLoaded",
      "SetExpFog",
      "SetVolFog",
      "SetCullDist",
      "GrenadeExplosionEffect",
      "MagicBullet",
      "RadiusDamage",
      "SetPlayerIgnoreRadiusDamage",
      "GlassRadiusDamage",
      "GetNumParts",
      "GetPartName",
      "Earthquake",
      "ScreenShake",
      "BulletSpread",
      "NewHudElem",
      "NewClientHudElem",
      "NewDamageIndicatorHudElem",
      "NewTeamHudElem",
      "NewScoreHudElem",
      "NewDebugHudElem",
      "ResetTimeOut",
      "GetAttachmentNames",
      "GetWatcherWeapons",
      "GetRetrievableWeapons",
      "IsLaserOn",
      "IsTurretFiring",
      "WeaponHasAttachment",
      "GetWeaponAttachments",
      "IsItemRestricted",
      "GetEquipmentHeadObjective",
      "GetCrateHeadObjective",
      "IsPlayerNumber",
      "SetWinningPlayer",
      "SetWinningTeam",
      "Announcement",
      "ClientAnnouncement",
      "GetTeamScore",
      "SetTeamScore",
      "TeamOpsShowHUD",
      "TeamOpsStart",
      "TeamOpsUpdateProgress",
      "SetClientNameMode",
      "UpdateClientNames",
      "GetTeamPlayersAlive",
      "GetDroppedWeapons",
      "ArtilleryIconLocation",
      "LogPrint",
      "WorldEntNumber",
      "Obituary",
      "ReviveObituary",
      "AddDemoBookmark",
      "StopDemoRecording",
      "PositionWouldTelefrag",
      "BoundsWouldTelefrag",
      "RecordUsedSpawnPoint",
      "TestSpawnPoint",
      "GetStartTime",
      "MapRestart",
      "MissionRestart",
      "MissionFailed",
      "ExitLevel",
      "KillServer",
      "AddTestClient",
      "SetObjectivePointStatus",
      "SetBombTimer",
      "SetRoundsPlayed",
      "SetInitialPlayersConnected",
      "SetMatchFlag",
      "SetMatchTalkFlag",
      "SetArchive",
      "AllClientsPrint",
      "ClientPrint",
      "MapExists",
      "IsValidGameType",
      "SkillUpdate",
      "RecordLeagueWinner",
      "SetTeamSpyplane",
      "GetTeamSpyplane",
      "SetTeamSatellite",
      "GetTeamSatellite",
      "GetAssignedTeam",
      "GetAssignedTeamName",
      "GetDefaultClassSlot",
      "GetItemAttachment",
      "GetItemAttachmentAllocationCost",
      "GetRefFromItemIndex",
      "GetItemGroupFromItemIndex",
      "GetBaseWeaponItemIndex",
      "GetGameTypeEnumFromName",
      "SetScoreboardColumns",
      "SetTopScorer",
      "ClearTopScorers",
      "RecordNumZombieRounds",
      "RecordGameResult",
      "FinalizeMatchRecord",
      "MatchRecordRoundEnd",
      "MatchRecordRoundStart",
      "MatchRecordOvertimeRound",
      "GetTouchingVolume",
      "GetFirstTouchFraction",
      "RecordMatchSummaryZombieEndGameData",
      "MatchRecorderIncrementHeaderStat",
      "RecordMatchInit",
      "InvalidateMatchRecord",
      "RecordZombieRoundStart",
      "RecordZombieRoundEnd",
      "GetCustomTeamName",
      "ClearPlayerCorpses",
      "RecordPlayerStats",
      "RecordPlayerMatchEnd",
      "RecordMatchBegin",
      "RecordBreadcrumbDataForPlayer",
      "MatchRecordNewPlayer",
      "MatchRecordLogAdditionalDeathInfo",
      "MatchRecordLogSpecialMoveDataForLife",
      "MatchRecordLogChallengeComplete",
      "MatchRecordSetCurrentLevelComplete",
      "MatchRecordSetLevelDifficultyForIndex",
      "MatchRecordGetWeaponIndex",
      "SetVoteString",
      "SetVoteTime",
      "SetVoteYesCount",
      "SetVoteNoCount",
      "ArenaGetSlot",
      "ArenaGetCurrentSeason",
      "ReportMTU",
      "Kick",
      "Ban",
      "Map",
      "PlayRumbleOnPosition",
      "PlayRumbleLoopOnPosition",
      "StopAllRumbles",
      "SoundExists",
      "SoundGetPlaybackTime",
      "SoundGetAlias",
      "IsSplitscreen",
      "IsLocalGame",
      "IsGlobalStatsServer",
      "SetMinimap",
      "SetMapCenter",
      "SetDemoIntermissionPoint",
      "SetGameEndTime",
      "SetSlowMotion",
      "SetPauseWorld",
      "IsWorldPaused",
      "NumRemoteClients",
      "VisionSetNaked",
      "VisionSetLastStand",
      "SetGravity",
      "SetWaveWaterGeneratorAmplitude",
      "EndLobby",
      "GetLobbyClientCount",
      "EnableLobbyJoins",
      "ClientSysRegister",
      "ClientSysSetState",
      "GetSnapshotIndexArray",
      "SnapshotAcknowledged",
      "GetAITriggerFlags",
      "GetMaxVehicles",
      "DisableDestructiblePieces",
      "EnableAllDestructiblePieces",
      "CreateDynEntAndLaunch",
      "GetVehicleTriggerFlags",
      "GetEntNavMaterial",
      "CollisionTestPointsInSphere",
      "CollisionTestPointsInCylinder",
      "CollisionTestPointsInPill",
      "CollisionTestPointsInCone",
      "CollisionTestPointsInBox",
      "QSortScoredSpawnPointsAscending",
      "Matrix4x4TransformPoints",
      "EnableSpawnPointList",
      "DisableSpawnPointList",
      "SetSpawnPointRandomVariation",
      "ClearSpawnPoints",
      "PlaceSpawnPoint",
      "AddSpawnPoints",
      "GetBestSpawnPoint",
      "ClearSpawnPointsBaseWeight",
      "SetSpawnPointsBaseWeight",
      "GetPlayerSpawnId",
      "IsSpawnPointVisible",
      "AddInfluencer",
      "AddEntityInfluencer",
      "AddOrientedInfluencer",
      "RemoveInfluencer",
      "EnableInfluencer",
      "SetInfluencerTeamMask",
      "SetInfluencerTimeOut",
      "GetInfluencerTimeOutRemaining",
      "SetDebugSideSwitch",
      "GetInfluencerPreset",
      "Target_Set",
      "Target_SetOffset",
      "Target_GetOffset",
      "Target_Remove",
      "Target_SetShader",
      "Target_SetOffscreenShader",
      "Target_IsInRect",
      "Target_IsInCircle",
      "Target_ScaleMinMaxRadius",
      "Target_OriginIsInCircle",
      "Target_BoundingIsUnderReticle",
      "Target_StartReticleLockOn",
      "Target_ClearReticleLockOn",
      "Target_GetArray",
      "Target_IsTarget",
      "Target_SetAttackMode",
      "Target_SetJavelinOnly",
      "Target_SetTurretAquire",
      "Target_SetAllowHighSteering",
      "GetMaxActiveContracts",
      "GetContractStatType",
      "GetContractStatName",
      "GetContractRewardXP",
      "GetContractRewardCP",
      "GetContractRequirements",
      "GetContractName",
      "GetContractRequiredCount",
      "GetContractResetConditions",
      "GetFogSettings",
      "PIXBeginEvent",
      "PIXEndEvent",
      "PIXMarker",
      "ChangeAdvertisedStatus",
      "SetQoSGameDataPayload",
      "ResetQoSGameDataPayload",
      "IncrementCounter",
      "GetCounterTotal",
      "ForceUploadCounters",
      "EnableOccluder",
      "SetHostMigrationStatus",
      "StartHostMigration",
      "GameRepThresholdExceeded",
      "ZeroGravityVolumeOn",
      "ZeroGravityVolumeOff",
      "GetUTC",
      "IsUsingT7Melee",
      "RegisterSkipto",
      "GetSkiptoName",
      "GetSkipTOS",
      "SetSkipTOS",
      "AIProfile_BeginEntry",
      "AIProfile_EndEntry",
      "StreamerModelHint",
      "StreamerRequest",
      "GetCyberComWeapon",
      "GetCyberComAbilityName",
      "CloneAndRemoveEntity",
      "LoadSentientEventParameters",
      "HkaiGetTimerTestPathfindStartPos",
      "HkaiGetTimerTestPathfindEndPos",
      "HkaiSetTimerTestEnt",
      "GetTotalServerPauseTime",
      "HashString",
      "__protected__SetBGBUnlocked",
      "__protected__GetBGBUnlocked",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(BuiltinFunctionTable, sizeof(BuiltinFunctionTable::functions));

union BuiltinMethodTable {
  struct {
    const BuiltinMethodDef SetStowedWeapon;
    const BuiltinMethodDef GetStowedWeapon;
    const BuiltinMethodDef ClearStowedWeapon;
    const BuiltinMethodDef GetAmmoCount;
    const BuiltinMethodDef SetVisibleToPlayer;
    const BuiltinMethodDef SetInvisibleToPlayer;
    const BuiltinMethodDef SetVisibleToAll;
    const BuiltinMethodDef SetInvisibleToAll;
    const BuiltinMethodDef SetVisibleToTeam;
    const BuiltinMethodDef SetVisibleToAllExceptTeam;
    const BuiltinMethodDef SetForceNoCull;
    const BuiltinMethodDef RemoveForceNoCull;
    const BuiltinMethodDef SetNoSunShadow;
    const BuiltinMethodDef RemoveNoSunShadow;
    const BuiltinMethodDef IsLinkedTo;
    const BuiltinMethodDef AllowTacticalInsertion;
    const BuiltinMethodDef AllowBotTargetting;
    const BuiltinMethodDef GetLinkedEnt;
    const BuiltinMethodDef GetOrigin;
    const BuiltinMethodDef GetAngles;
    const BuiltinMethodDef GetMins;
    const BuiltinMethodDef GetMaxs;
    const BuiltinMethodDef GetAbsMins;
    const BuiltinMethodDef GetAbsMaxs;
    const BuiltinMethodDef GetPointInBounds;
    const BuiltinMethodDef GetEye;
    const BuiltinMethodDef GetEyeApprox;
    const BuiltinMethodDef UseBy;
    const BuiltinMethodDef SetStableMissile;
    const BuiltinMethodDef PlayerSetGroundReferenceEnt;
    const BuiltinMethodDef IsTouching;
    const BuiltinMethodDef IsTouchingSwept;
    const BuiltinMethodDef IsTouchingVolume;
    const BuiltinMethodDef PlaySound;
    const BuiltinMethodDef PlaySoundWithNotify;
    const BuiltinMethodDef PlaySoundOnTag;
    const BuiltinMethodDef PlaySoundToTeam;
    const BuiltinMethodDef PlayBattleChatterToTeam;
    const BuiltinMethodDef PlayLoopSound;
    const BuiltinMethodDef StopLoopSound;
    const BuiltinMethodDef StopSounds;
    const BuiltinMethodDef StopSound;
    const BuiltinMethodDef PlayRumbleOnEntity;
    const BuiltinMethodDef PlayRumbleLoopOnEntity;
    const BuiltinMethodDef StopRumble;
    const BuiltinMethodDef Delete;
    const BuiltinMethodDef SetModel;
    const BuiltinMethodDef SetHighDetail;
    const BuiltinMethodDef SetDedicatedShadow;
    const BuiltinMethodDef SetEnemyModel;
    const BuiltinMethodDef DoDamage;
    const BuiltinMethodDef ForcePainOn;
    const BuiltinMethodDef Kill;
    const BuiltinMethodDef GetNormalHealth;
    const BuiltinMethodDef SetNormalHealth;
    const BuiltinMethodDef SetMaxHealth;
    const BuiltinMethodDef Show;
    const BuiltinMethodDef Hide;
    const BuiltinMethodDef IsHidden;
    const BuiltinMethodDef Ghost;
    const BuiltinMethodDef GhostInDemo;
    const BuiltinMethodDef ShowInDemo;
    const BuiltinMethodDef ShowToTeam;
    const BuiltinMethodDef HideFromTeam;
    const BuiltinMethodDef LaserOn;
    const BuiltinMethodDef LaserOff;
    const BuiltinMethodDef ShowToPlayer;
    const BuiltinMethodDef SetContents;
    const BuiltinMethodDef Solid;
    const BuiltinMethodDef NotSolid;
    const BuiltinMethodDef SolidCapsule;
    const BuiltinMethodDef NotSolidCapsule;
    const BuiltinMethodDef StartFiring;
    const BuiltinMethodDef StopFiring;
    const BuiltinMethodDef ShootTurret;
    const BuiltinMethodDef StopShootTurret;
    const BuiltinMethodDef SetMode;
    const BuiltinMethodDef GetTurretOwner;
    const BuiltinMethodDef GetTurretArcLimits;
    const BuiltinMethodDef SetPlayerSpread;
    const BuiltinMethodDef SetAISpread;
    const BuiltinMethodDef ClearTargetEntity;
    const BuiltinMethodDef MakeTurretUsable;
    const BuiltinMethodDef MakeTurretUnusable;
    const BuiltinMethodDef SetTurretAccuracy;
    const BuiltinMethodDef GetTurretTarget;
    const BuiltinMethodDef DisconnectPaths;
    const BuiltinMethodDef ConnectPaths;
    const BuiltinMethodDef EnableObstacle;
    const BuiltinMethodDef GetStance;
    const BuiltinMethodDef SetStance;
    const BuiltinMethodDef SetCursorHint;
    const BuiltinMethodDef SetReviveHintString;
    const BuiltinMethodDef SetHintStringForPerk;
    const BuiltinMethodDef SetHintString;
    const BuiltinMethodDef SetHintStringForPlayer;
    const BuiltinMethodDef GetTriggerAccumulate;
    const BuiltinMethodDef SetFOVForKillcam;
    const BuiltinMethodDef SetHintLowPriority;
    const BuiltinMethodDef UseTriggerRequireLookAt;
    const BuiltinMethodDef UseTriggerRequireLookToward;
    const BuiltinMethodDef UseTriggerIgnoreUseHoldTime;
    const BuiltinMethodDef TriggerIgnoreTeam;
    const BuiltinMethodDef TriggerEnable;
    const BuiltinMethodDef IsTriggerEnabled;
    const BuiltinMethodDef ShellShock;
    const BuiltinMethodDef GetWeaponForwardDir;
    const BuiltinMethodDef GetWeaponMuzzlePoint;
    const BuiltinMethodDef GetEntNum;
    const BuiltinMethodDef StopShellShock;
    const BuiltinMethodDef SetDepthOfField;
    const BuiltinMethodDef SetBurn;
    const BuiltinMethodDef SetElectrified;
    const BuiltinMethodDef SpawnNapalmGroundFlame;
    const BuiltinMethodDef NeedsRevive;
    const BuiltinMethodDef IsInSecondChance;
    const BuiltinMethodDef DepthInWater;
    const BuiltinMethodDef ShootUp;
    const BuiltinMethodDef DepthOfPlayerInWater;
    const BuiltinMethodDef IsPlayerSwimming;
    const BuiltinMethodDef WalkUnderwater;
    const BuiltinMethodDef IsPlayerUnderwater;
    const BuiltinMethodDef GetLastOutWaterTime;
    const BuiltinMethodDef UseWeaponHideTags;
    const BuiltinMethodDef UseWeaponModel;
    const BuiltinMethodDef UseBuildKitWeaponModel;
    const BuiltinMethodDef SetWaterDrops;
    const BuiltinMethodDef IsFiringTurret;
    const BuiltinMethodDef IsTurretLockedOn;
    const BuiltinMethodDef SetViewModelDepthOfField;
    const BuiltinMethodDef ViewKick;
    const BuiltinMethodDef EntityRadiusDamage;
    const BuiltinMethodDef CylinderDamage;
    const BuiltinMethodDef Detonate;
    const BuiltinMethodDef DamageConeTrace;
    const BuiltinMethodDef SightConeTrace;
    const BuiltinMethodDef HeliTurretSightTrace;
    const BuiltinMethodDef HeliTurretDogTrace;
    const BuiltinMethodDef PlayerSightTrace;
    const BuiltinMethodDef VisionSetLerpRatio;
    const BuiltinMethodDef DoCowardsWayAnims;
    const BuiltinMethodDef StartPoisoning;
    const BuiltinMethodDef StopPoisoning;
    const BuiltinMethodDef StartBinocs;
    const BuiltinMethodDef StopBinocs;
    const BuiltinMethodDef IsFlared;
    const BuiltinMethodDef IsPoisoned;
    const BuiltinMethodDef SetCameraSpikeActive;
    const BuiltinMethodDef IsInsideHeightLock;
    const BuiltinMethodDef IsMissileInsideHeightLock;
    const BuiltinMethodDef IsOnGround;
    const BuiltinMethodDef GetGroundEnt;
    const BuiltinMethodDef GetMoverEnt;
    const BuiltinMethodDef IsSprinting;
    const BuiltinMethodDef IsWallRunning;
    const BuiltinMethodDef GetWallRunWallNormal;
    const BuiltinMethodDef IsDoubleJumping;
    const BuiltinMethodDef SetDoubleJumpEnergy;
    const BuiltinMethodDef ResetDoubleJumpRechargeTime;
    const BuiltinMethodDef IsSliding;
    const BuiltinMethodDef IsTraversing;
    const BuiltinMethodDef IsSlamming;
    const BuiltinMethodDef GetPathfindingRadius;
    const BuiltinMethodDef GetAnimTime;
    const BuiltinMethodDef SetAnimTime;
    const BuiltinMethodDef SetAnimTimeByName;
    const BuiltinMethodDef UseAnimTree;
    const BuiltinMethodDef AnimScripted;
    const BuiltinMethodDef AnimRelative;
    const BuiltinMethodDef StopAnimScripted;
    const BuiltinMethodDef IsPlayingAnimScripted;
    const BuiltinMethodDef SetEntityAnimRate;
    const BuiltinMethodDef GetEntityAnimRate;
    const BuiltinMethodDef GetCentroid;
    const BuiltinMethodDef SetLightingOnly;
    const BuiltinMethodDef SetIncludeMeshes;
    const BuiltinMethodDef WeaponHasAttachmentAndUnlocked;
    const BuiltinMethodDef AllWeaponAttachmentsUnlocked;
    const BuiltinMethodDef GetTotalUnlockedWeaponAttachments;
    const BuiltinMethodDef GetEntityNumber;
    const BuiltinMethodDef GetEntityType;
    const BuiltinMethodDef EnableGrenadeTouchDamage;
    const BuiltinMethodDef DisableGrenadeTouchDamage;
    const BuiltinMethodDef EnableAimAssist;
    const BuiltinMethodDef DisableAimAssist;
    const BuiltinMethodDef PlaceSpawnPointOld;
    const BuiltinMethodDef SetSpawnClientFlag;
    const BuiltinMethodDef DirectionalHitIndicator;
    const BuiltinMethodDef SendKillstreakDamageEvent;
    const BuiltinMethodDef GetPlayerSpeed;
    const BuiltinMethodDef GetPlayerGravity;
    const BuiltinMethodDef SetPlayerGravity;
    const BuiltinMethodDef ClearPlayerGravity;
    const BuiltinMethodDef SendFaceEvent;
    const BuiltinMethodDef SetTeamForTrigger;
    const BuiltinMethodDef SetExcludeTeamForTrigger;
    const BuiltinMethodDef SetPerkForTrigger;
    const BuiltinMethodDef SetIgnoreEntForTrigger;
    const BuiltinMethodDef ClientClaimTrigger;
    const BuiltinMethodDef ClientReleaseTrigger;
    const BuiltinMethodDef ReleaseClaimedTrigger;
    const BuiltinMethodDef IsItemLocked;
    const BuiltinMethodDef IsItemPurchased;
    const BuiltinMethodDef UploadLeaderboards;
    const BuiltinMethodDef SetNemesisXUID;
    const BuiltinMethodDef GetLoadoutItem;
    const BuiltinMethodDef IsBonusCardActive;
    const BuiltinMethodDef BonusCardActiveCount;
    const BuiltinMethodDef IsSpecialistUnlocked;
    const BuiltinMethodDef GetSpecialistIndex;
    const BuiltinMethodDef GetLoadoutItemRef;
    const BuiltinMethodDef SetPlayerStateLoadoutBonusCards;
    const BuiltinMethodDef SetPlayerStateLoadoutWeapons;
    const BuiltinMethodDef GetLoadoutWeapon;
    const BuiltinMethodDef GetAttachmentCosmeticVariantForWeapon;
    const BuiltinMethodDef GetLoadoutPerks;
    const BuiltinMethodDef GetLoadoutAllocation;
    const BuiltinMethodDef GetLoadoutGunSmithVariantIndex;
    const BuiltinMethodDef ConsumableIncrement;
    const BuiltinMethodDef ConsumableDecrement;
    const BuiltinMethodDef ConsumableGet;
    const BuiltinMethodDef ConsumableSet;
    const BuiltinMethodDef GetBubbleGumPack;
    const BuiltinMethodDef GetBGBRemaining;
    const BuiltinMethodDef GetBGBTokensGained;
    const BuiltinMethodDef IncrementBGBTokensGained;
    const BuiltinMethodDef GetBGBTokensUsed;
    const BuiltinMethodDef IncrementBGBTokensUsed;
    const BuiltinMethodDef GetBGBTokensRemaining;
    const BuiltinMethodDef GetBuildKitWeapon;
    const BuiltinMethodDef GetBuildKitWeaponOptions;
    const BuiltinMethodDef GetBuildKitAttachmentCosmeticVariantIndexes;
    const BuiltinMethodDef SetBodyRenderOptions;
    const BuiltinMethodDef SetMoveSpeedScale;
    const BuiltinMethodDef GetMoveSpeedScale;
    const BuiltinMethodDef Missile_SetTarget;
    const BuiltinMethodDef Missile_GetTarget;
    const BuiltinMethodDef Missile_DroneSetVisible;
    const BuiltinMethodDef BallSetTarget;
    const BuiltinMethodDef IsOnLadder;
    const BuiltinMethodDef IsMantling;
    const BuiltinMethodDef IsOnSlide;
    const BuiltinMethodDef BloodImpact;
    const BuiltinMethodDef StartRagDoll;
    const BuiltinMethodDef IsRagdoll;
    const BuiltinMethodDef LaunchRagdoll;
    const BuiltinMethodDef LaunchVehicle;
    const BuiltinMethodDef ApplyBallisticTarget;
    const BuiltinMethodDef SetBlur;
    const BuiltinMethodDef ResetMissileDetonationTime;
    const BuiltinMethodDef GiveAchievement;
    const BuiltinMethodDef SetZombieName;
    const BuiltinMethodDef SetEntGravityTrajectory;
    const BuiltinMethodDef SetTeam;
    const BuiltinMethodDef GetTeam;
    const BuiltinMethodDef SetOwner;
    const BuiltinMethodDef SetMissileOwner;
    const BuiltinMethodDef SetEntityWeapon;
    const BuiltinMethodDef SetTurretOwner;
    const BuiltinMethodDef SetTurretType;
    const BuiltinMethodDef GetCorpseAnim;
    const BuiltinMethodDef ItemWeaponSetAmmo;
    const BuiltinMethodDef SetWeaponRenderOptions;
    const BuiltinMethodDef SetAttachmentCosmeticVariants;
    const BuiltinMethodDef SetAttachmentCosmeticVariantIndex;
    const BuiltinMethodDef SetTurretCarried;
    const BuiltinMethodDef SetTurretMinimapVisible;
    const BuiltinMethodDef SetMissileCoasting;
    const BuiltinMethodDef SetMissileBrake;
    const BuiltinMethodDef SetWeapon;
    const BuiltinMethodDef SetWeaponOptions;
    const BuiltinMethodDef SetAttachmentCosmeticVariantIndexes;
    const BuiltinMethodDef Gib;
    const BuiltinMethodDef SetPlayerCollision;
    const BuiltinMethodDef ReportUser;
    const BuiltinMethodDef SetPlayerCurrentObjective;
    const BuiltinMethodDef SetPlayerCurrentStreak;
    const BuiltinMethodDef GetVelocity;
    const BuiltinMethodDef SpawnFromSpawner;
    const BuiltinMethodDef PredictSpawnPoint;
    const BuiltinMethodDef SetDefaultDropPitch;
    const BuiltinMethodDef SetScanningPitch;
    const BuiltinMethodDef LaunchBomb;
    const BuiltinMethodDef MagicMissile;
    const BuiltinMethodDef MagicGrenadePlayer;
    const BuiltinMethodDef MakeGrenadeDud;
    const BuiltinMethodDef FakeFire;
    const BuiltinMethodDef SetFakeFire;
    const BuiltinMethodDef MakeUsable;
    const BuiltinMethodDef MakeUnusable;
    const BuiltinMethodDef PredictGrenade;
    const BuiltinMethodDef SetDangerous;
    const BuiltinMethodDef IsDangerous;
    const BuiltinMethodDef GetIndexForActiveContract;
    const BuiltinMethodDef GetActiveContractProgress;
    const BuiltinMethodDef IncrementActiveContractProgress;
    const BuiltinMethodDef IncrementActiveContractTime;
    const BuiltinMethodDef IsActiveContractComplete;
    const BuiltinMethodDef HasActiveContractExpired;
    const BuiltinMethodDef GetActiveContractTimePassed;
    const BuiltinMethodDef ResetActiveContractProgress;
    const BuiltinMethodDef RecordLeaguePreLoser;
    const BuiltinMethodDef LUINotifyEventToSpectators;
    const BuiltinMethodDef IsTestClient;
    const BuiltinMethodDef SetWorldFogActiveBank;
    const BuiltinMethodDef IsZBarrier;
    const BuiltinMethodDef SetClone;
    const BuiltinMethodDef GetNumZBarrierPieces;
    const BuiltinMethodDef GetZBarrierPieceState;
    const BuiltinMethodDef SetZBarrierPieceState;
    const BuiltinMethodDef IsZBarrierOpen;
    const BuiltinMethodDef IsZBarrierClosed;
    const BuiltinMethodDef HideZBarrierPiece;
    const BuiltinMethodDef ShowZBarrierPiece;
    const BuiltinMethodDef ZBarrierPieceUseDefaultModel;
    const BuiltinMethodDef ZBarrierPieceUseAlternateModel;
    const BuiltinMethodDef ZBarrierPieceUseUpgradedModel;
    const BuiltinMethodDef ZBarrierPieceUseBoxRiseLogic;
    const BuiltinMethodDef ZBarrierPieceUseAttachWeapon;
    const BuiltinMethodDef GetUpgradedPieceNumLives;
    const BuiltinMethodDef GetZBarrierPieceIndicesInState;
    const BuiltinMethodDef GetZBarrierPieceAnimLengthForState;
    const BuiltinMethodDef GetZBarrierPieceAnimState;
    const BuiltinMethodDef GetZBarrierPieceAnimSubState;
    const BuiltinMethodDef ZBarrierSupportsZombieTaunts;
    const BuiltinMethodDef ZBarrierSupportsZombieReachThroughAttacks;
    const BuiltinMethodDef GetZBarrierTauntAnimState;
    const BuiltinMethodDef GetZBarrierReachThroughAttackAnimState;
    const BuiltinMethodDef GetZBarrierNumAttackSlots;
    const BuiltinMethodDef GetZBarrierAttackSlotHorzOffset;
    const BuiltinMethodDef SetZBarrierColModel;
    const BuiltinMethodDef GetWeaponOptic;
    const BuiltinMethodDef ArenaBeginMatch;
    const BuiltinMethodDef ArenaEndMatch;
    const BuiltinMethodDef GetPathMetric;
    const BuiltinMethodDef SetIgnorePauseWorld;
    const BuiltinMethodDef SetEntityPaused;
    const BuiltinMethodDef IsPaused;
    const BuiltinMethodDef IsStreamerReady;
    const BuiltinMethodDef IsClientCacheStable;
    const BuiltinMethodDef IsLoadingCinematicPlaying;
    const BuiltinMethodDef BotReleaseManualControl;
    const BuiltinMethodDef SetBallPassAllowed;
    const BuiltinMethodDef QueueMeleeActionState;
    const BuiltinMethodDef StopJukeMove;
  };
  const BuiltinMethodDef methods[345];

  static constexpr const char *names[345] = {
      "SetStowedWeapon",
      "GetStowedWeapon",
      "ClearStowedWeapon",
      "GetAmmoCount",
      "SetVisibleToPlayer",
      "SetInvisibleToPlayer",
      "SetVisibleToAll",
      "SetInvisibleToAll",
      "SetVisibleToTeam",
      "SetVisibleToAllExceptTeam",
      "SetForceNoCull",
      "RemoveForceNoCull",
      "SetNoSunShadow",
      "RemoveNoSunShadow",
      "IsLinkedTo",
      "AllowTacticalInsertion",
      "AllowBotTargetting",
      "GetLinkedEnt",
      "GetOrigin",
      "GetAngles",
      "GetMins",
      "GetMaxs",
      "GetAbsMins",
      "GetAbsMaxs",
      "GetPointInBounds",
      "GetEye",
      "GetEyeApprox",
      "UseBy",
      "SetStableMissile",
      "PlayerSetGroundReferenceEnt",
      "IsTouching",
      "IsTouchingSwept",
      "IsTouchingVolume",
      "PlaySound",
      "PlaySoundWithNotify",
      "PlaySoundOnTag",
      "PlaySoundToTeam",
      "PlayBattleChatterToTeam",
      "PlayLoopSound",
      "StopLoopSound",
      "StopSounds",
      "StopSound",
      "PlayRumbleOnEntity",
      "PlayRumbleLoopOnEntity",
      "StopRumble",
      "Delete",
      "SetModel",
      "SetHighDetail",
      "SetDedicatedShadow",
      "SetEnemyModel",
      "DoDamage",
      "ForcePainOn",
      "Kill",
      "GetNormalHealth",
      "SetNormalHealth",
      "SetMaxHealth",
      "Show",
      "Hide",
      "IsHidden",
      "Ghost",
      "GhostInDemo",
      "ShowInDemo",
      "ShowToTeam",
      "HideFromTeam",
      "LaserOn",
      "LaserOff",
      "ShowToPlayer",
      "SetContents",
      "Solid",
      "NotSolid",
      "SolidCapsule",
      "NotSolidCapsule",
      "StartFiring",
      "StopFiring",
      "ShootTurret",
      "StopShootTurret",
      "SetMode",
      "GetTurretOwner",
      "GetTurretArcLimits",
      "SetPlayerSpread",
      "SetAISpread",
      "ClearTargetEntity",
      "MakeTurretUsable",
      "MakeTurretUnusable",
      "SetTurretAccuracy",
      "GetTurretTarget",
      "DisconnectPaths",
      "ConnectPaths",
      "EnableObstacle",
      "GetStance",
      "SetStance",
      "SetCursorHint",
      "SetReviveHintString",
      "SetHintStringForPerk",
      "SetHintString",
      "SetHintStringForPlayer",
      "GetTriggerAccumulate",
      "SetFOVForKillcam",
      "SetHintLowPriority",
      "UseTriggerRequireLookAt",
      "UseTriggerRequireLookToward",
      "UseTriggerIgnoreUseHoldTime",
      "TriggerIgnoreTeam",
      "TriggerEnable",
      "IsTriggerEnabled",
      "ShellShock",
      "GetWeaponForwardDir",
      "GetWeaponMuzzlePoint",
      "GetEntNum",
      "StopShellShock",
      "SetDepthOfField",
      "SetBurn",
      "SetElectrified",
      "SpawnNapalmGroundFlame",
      "NeedsRevive",
      "IsInSecondChance",
      "DepthInWater",
      "ShootUp",
      "DepthOfPlayerInWater",
      "IsPlayerSwimming",
      "WalkUnderwater",
      "IsPlayerUnderwater",
      "GetLastOutWaterTime",
      "UseWeaponHideTags",
      "UseWeaponModel",
      "UseBuildKitWeaponModel",
      "SetWaterDrops",
      "IsFiringTurret",
      "IsTurretLockedOn",
      "SetViewModelDepthOfField",
      "ViewKick",
      "EntityRadiusDamage",
      "CylinderDamage",
      "Detonate",
      "DamageConeTrace",
      "SightConeTrace",
      "HeliTurretSightTrace",
      "HeliTurretDogTrace",
      "PlayerSightTrace",
      "VisionSetLerpRatio",
      "DoCowardsWayAnims",
      "StartPoisoning",
      "StopPoisoning",
      "StartBinocs",
      "StopBinocs",
      "IsFlared",
      "IsPoisoned",
      "SetCameraSpikeActive",
      "IsInsideHeightLock",
      "IsMissileInsideHeightLock",
      "IsOnGround",
      "GetGroundEnt",
      "GetMoverEnt",
      "IsSprinting",
      "IsWallRunning",
      "GetWallRunWallNormal",
      "IsDoubleJumping",
      "SetDoubleJumpEnergy",
      "ResetDoubleJumpRechargeTime",
      "IsSliding",
      "IsTraversing",
      "IsSlamming",
      "GetPathfindingRadius",
      "GetAnimTime",
      "SetAnimTime",
      "SetAnimTimeByName",
      "UseAnimTree",
      "AnimScripted",
      "AnimRelative",
      "StopAnimScripted",
      "IsPlayingAnimScripted",
      "SetEntityAnimRate",
      "GetEntityAnimRate",
      "GetCentroid",
      "SetLightingOnly",
      "SetIncludeMeshes",
      "WeaponHasAttachmentAndUnlocked",
      "AllWeaponAttachmentsUnlocked",
      "GetTotalUnlockedWeaponAttachments",
      "GetEntityNumber",
      "GetEntityType",
      "EnableGrenadeTouchDamage",
      "DisableGrenadeTouchDamage",
      "EnableAimAssist",
      "DisableAimAssist",
      "PlaceSpawnPointOld",
      "SetSpawnClientFlag",
      "DirectionalHitIndicator",
      "SendKillstreakDamageEvent",
      "GetPlayerSpeed",
      "GetPlayerGravity",
      "SetPlayerGravity",
      "ClearPlayerGravity",
      "SendFaceEvent",
      "SetTeamForTrigger",
      "SetExcludeTeamForTrigger",
      "SetPerkForTrigger",
      "SetIgnoreEntForTrigger",
      "ClientClaimTrigger",
      "ClientReleaseTrigger",
      "ReleaseClaimedTrigger",
      "IsItemLocked",
      "IsItemPurchased",
      "UploadLeaderboards",
      "SetNemesisXUID",
      "GetLoadoutItem",
      "IsBonusCardActive",
      "BonusCardActiveCount",
      "IsSpecialistUnlocked",
      "GetSpecialistIndex",
      "GetLoadoutItemRef",
      "SetPlayerStateLoadoutBonusCards",
      "SetPlayerStateLoadoutWeapons",
      "GetLoadoutWeapon",
      "GetAttachmentCosmeticVariantForWeapon",
      "GetLoadoutPerks",
      "GetLoadoutAllocation",
      "GetLoadoutGunSmithVariantIndex",
      "ConsumableIncrement",
      "ConsumableDecrement",
      "ConsumableGet",
      "ConsumableSet",
      "GetBubbleGumPack",
      "GetBGBRemaining",
      "GetBGBTokensGained",
      "IncrementBGBTokensGained",
      "GetBGBTokensUsed",
      "IncrementBGBTokensUsed",
      "GetBGBTokensRemaining",
      "GetBuildKitWeapon",
      "GetBuildKitWeaponOptions",
      "GetBuildKitAttachmentCosmeticVariantIndexes",
      "SetBodyRenderOptions",
      "SetMoveSpeedScale",
      "GetMoveSpeedScale",
      "Missile_SetTarget",
      "Missile_GetTarget",
      "Missile_DroneSetVisible",
      "BallSetTarget",
      "IsOnLadder",
      "IsMantling",
      "IsOnSlide",
      "BloodImpact",
      "StartRagDoll",
      "IsRagdoll",
      "LaunchRagdoll",
      "LaunchVehicle",
      "ApplyBallisticTarget",
      "SetBlur",
      "ResetMissileDetonationTime",
      "GiveAchievement",
      "SetZombieName",
      "SetEntGravityTrajectory",
      "SetTeam",
      "GetTeam",
      "SetOwner",
      "SetMissileOwner",
      "SetEntityWeapon",
      "SetTurretOwner",
      "SetTurretType",
      "GetCorpseAnim",
      "ItemWeaponSetAmmo",
      "SetWeaponRenderOptions",
      "SetAttachmentCosmeticVariants",
      "SetAttachmentCosmeticVariantIndex",
      "SetTurretCarried",
      "SetTurretMinimapVisible",
      "SetMissileCoasting",
      "SetMissileBrake",
      "SetWeapon",
      "SetWeaponOptions",
      "SetAttachmentCosmeticVariantIndexes",
      "Gib",
      "SetPlayerCollision",
      "ReportUser",
      "SetPlayerCurrentObjective",
      "SetPlayerCurrentStreak",
      "GetVelocity",
      "SpawnFromSpawner",
      "PredictSpawnPoint",
      "SetDefaultDropPitch",
      "SetScanningPitch",
      "LaunchBomb",
      "MagicMissile",
      "MagicGrenadePlayer",
      "MakeGrenadeDud",
      "FakeFire",
      "SetFakeFire",
      "MakeUsable",
      "MakeUnusable",
      "PredictGrenade",
      "SetDangerous",
      "IsDangerous",
      "GetIndexForActiveContract",
      "GetActiveContractProgress",
      "IncrementActiveContractProgress",
      "IncrementActiveContractTime",
      "IsActiveContractComplete",
      "HasActiveContractExpired",
      "GetActiveContractTimePassed",
      "ResetActiveContractProgress",
      "RecordLeaguePreLoser",
      "LUINotifyEventToSpectators",
      "IsTestClient",
      "SetWorldFogActiveBank",
      "IsZBarrier",
      "SetClone",
      "GetNumZBarrierPieces",
      "GetZBarrierPieceState",
      "SetZBarrierPieceState",
      "IsZBarrierOpen",
      "IsZBarrierClosed",
      "HideZBarrierPiece",
      "ShowZBarrierPiece",
      "ZBarrierPieceUseDefaultModel",
      "ZBarrierPieceUseAlternateModel",
      "ZBarrierPieceUseUpgradedModel",
      "ZBarrierPieceUseBoxRiseLogic",
      "ZBarrierPieceUseAttachWeapon",
      "GetUpgradedPieceNumLives",
      "GetZBarrierPieceIndicesInState",
      "GetZBarrierPieceAnimLengthForState",
      "GetZBarrierPieceAnimState",
      "GetZBarrierPieceAnimSubState",
      "ZBarrierSupportsZombieTaunts",
      "ZBarrierSupportsZombieReachThroughAttacks",
      "GetZBarrierTauntAnimState",
      "GetZBarrierReachThroughAttackAnimState",
      "GetZBarrierNumAttackSlots",
      "GetZBarrierAttackSlotHorzOffset",
      "SetZBarrierColModel",
      "GetWeaponOptic",
      "ArenaBeginMatch",
      "ArenaEndMatch",
      "GetPathMetric",
      "SetIgnorePauseWorld",
      "SetEntityPaused",
      "IsPaused",
      "IsStreamerReady",
      "IsClientCacheStable",
      "IsLoadingCinematicPlaying",
      "BotReleaseManualControl",
      "SetBallPassAllowed",
      "QueueMeleeActionState",
      "StopJukeMove",
  };
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(BuiltinMethodTable, sizeof(BuiltinMethodTable::methods));

} // namespace gscr
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game