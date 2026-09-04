#pragma once

#include <game/structs/scr/builtin/core.hpp>
#include <game/structs/scr/builtin/table/macros.hpp>

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace gscr {
union BuiltinFunctionTable {
  struct {
    BuiltinFunctionDef Spawn;
    BuiltinFunctionDef SpawnCollision;
    BuiltinFunctionDef SpawnTimedFX;
    BuiltinFunctionDef SpawnActor;
    BuiltinFunctionDef SpawnVehicle;
    BuiltinFunctionDef SpawnPlane;
    BuiltinFunctionDef SpawnTurret;
    BuiltinFunctionDef AddStruct;
    BuiltinFunctionDef SpawnHelicopter;
    BuiltinFunctionDef GetEntByNum;
    BuiltinFunctionDef GetBrushModelCenter;
    BuiltinFunctionDef GetDemoFileID;
    BuiltinFunctionDef GetPlayers;
    BuiltinFunctionDef CountPlayers;
    BuiltinFunctionDef GetDamageableEntArray;
    BuiltinFunctionDef MaySpawnFakeEntity;
    BuiltinFunctionDef MaySpawnEntity;
    BuiltinFunctionDef Objective_Add;
    BuiltinFunctionDef Objective_Delete;
    BuiltinFunctionDef Objective_State;
    BuiltinFunctionDef Objective_Icon;
    BuiltinFunctionDef Objective_Position;
    BuiltinFunctionDef Objective_OnEntity;
    BuiltinFunctionDef Objective_ClearEntity;
    BuiltinFunctionDef Objective_SetVisibleToPlayer;
    BuiltinFunctionDef Objective_SetInvisibleToPlayer;
    BuiltinFunctionDef Objective_SetVisibleToPlayerByIndex;
    BuiltinFunctionDef Objective_SetInvisibleToPlayerByIndex;
    BuiltinFunctionDef Objective_SetVisibleToAll;
    BuiltinFunctionDef Objective_SetInvisibleToAll;
    BuiltinFunctionDef Objective_SetSize;
    BuiltinFunctionDef Objective_SetColor;
    BuiltinFunctionDef Objective_SetProgress;
    BuiltinFunctionDef Objective_SetGamemodeFlags;
    BuiltinFunctionDef Objective_SetFlag;
    BuiltinFunctionDef Objective_GetGamemodeFlags;
    BuiltinFunctionDef Objective_SetPlayerUsing;
    BuiltinFunctionDef Objective_ClearPlayerUsing;
    BuiltinFunctionDef Objective_ClearAllUsing;
    BuiltinFunctionDef Objective_Set3D;
    BuiltinFunctionDef Objective_Team;
    BuiltinFunctionDef Objective_VisibleTeams;
    BuiltinFunctionDef Objective_SetUIModelValue;
    BuiltinFunctionDef Missile_CreateAttractorEnt;
    BuiltinFunctionDef Missile_CreateAttractorOrigin;
    BuiltinFunctionDef Missile_CreateRepulsorEnt;
    BuiltinFunctionDef Missile_CreateRepulsorOrigin;
    BuiltinFunctionDef Missile_DeleteAttractor;
    BuiltinFunctionDef BulletTrace;
    BuiltinFunctionDef GroundTrace;
    BuiltinFunctionDef BeamTrace;
    BuiltinFunctionDef BulletTracePassed;
    BuiltinFunctionDef SightTracePassed;
    BuiltinFunctionDef PhysicsTrace;
    BuiltinFunctionDef PlayerPhysicsTrace;
    BuiltinFunctionDef PlayerPhysicsTraceIgnoreEnt;
    BuiltinFunctionDef PlayerGrappleTrace;
    BuiltinFunctionDef PlayerBulletTrace;
    BuiltinFunctionDef WorldTrace;
    BuiltinFunctionDef PlayerPositionValid;
    BuiltinFunctionDef PlayerPositionValidIgnoreEnt;
    BuiltinFunctionDef GetMoveDelta;
    BuiltinFunctionDef GetAngleDelta;
    BuiltinFunctionDef GetNorthYaw;
    BuiltinFunctionDef PlayFX;
    BuiltinFunctionDef PlayFXOnTag;
    BuiltinFunctionDef PlayFXOnCamera;
    BuiltinFunctionDef GetWaterHeight;
    BuiltinFunctionDef PlayLoopedFX;
    BuiltinFunctionDef SpawnFX;
    BuiltinFunctionDef TriggerFX;
    BuiltinFunctionDef FXBlockSight;
    BuiltinFunctionDef ResetGlass;
    BuiltinFunctionDef ShatterAllGlass;
    BuiltinFunctionDef PhysicsExplosionSphere;
    BuiltinFunctionDef PhysicsExplosionCylinder;
    BuiltinFunctionDef PhysicsJolt;
    BuiltinFunctionDef PhysicsJetThrust;
    BuiltinFunctionDef CreateStreamerHint;
    BuiltinFunctionDef AreTexturesLoaded;
    BuiltinFunctionDef SetExpFog;
    BuiltinFunctionDef SetVolFog;
    BuiltinFunctionDef SetCullDist;
    BuiltinFunctionDef GrenadeExplosionEffect;
    BuiltinFunctionDef MagicBullet;
    BuiltinFunctionDef RadiusDamage;
    BuiltinFunctionDef SetPlayerIgnoreRadiusDamage;
    BuiltinFunctionDef GlassRadiusDamage;
    BuiltinFunctionDef GetNumParts;
    BuiltinFunctionDef GetPartName;
    BuiltinFunctionDef Earthquake;
    BuiltinFunctionDef ScreenShake;
    BuiltinFunctionDef BulletSpread;
    BuiltinFunctionDef NewHudElem;
    BuiltinFunctionDef NewClientHudElem;
    BuiltinFunctionDef NewDamageIndicatorHudElem;
    BuiltinFunctionDef NewTeamHudElem;
    BuiltinFunctionDef NewScoreHudElem;
    BuiltinFunctionDef NewDebugHudElem;
    BuiltinFunctionDef ResetTimeOut;
    BuiltinFunctionDef GetAttachmentNames;
    BuiltinFunctionDef GetWatcherWeapons;
    BuiltinFunctionDef GetRetrievableWeapons;
    BuiltinFunctionDef IsLaserOn;
    BuiltinFunctionDef IsTurretFiring;
    BuiltinFunctionDef WeaponHasAttachment;
    BuiltinFunctionDef GetWeaponAttachments;
    BuiltinFunctionDef IsItemRestricted;
    BuiltinFunctionDef GetEquipmentHeadObjective;
    BuiltinFunctionDef GetCrateHeadObjective;
    BuiltinFunctionDef IsPlayerNumber;
    BuiltinFunctionDef SetWinningPlayer;
    BuiltinFunctionDef SetWinningTeam;
    BuiltinFunctionDef Announcement;
    BuiltinFunctionDef ClientAnnouncement;
    BuiltinFunctionDef GetTeamScore;
    BuiltinFunctionDef SetTeamScore;
    BuiltinFunctionDef TeamOpsShowHUD;
    BuiltinFunctionDef TeamOpsStart;
    BuiltinFunctionDef TeamOpsUpdateProgress;
    BuiltinFunctionDef SetClientNameMode;
    BuiltinFunctionDef UpdateClientNames;
    BuiltinFunctionDef GetTeamPlayersAlive;
    BuiltinFunctionDef GetDroppedWeapons;
    BuiltinFunctionDef ArtilleryIconLocation;
    BuiltinFunctionDef LogPrint;
    BuiltinFunctionDef WorldEntNumber;
    BuiltinFunctionDef Obituary;
    BuiltinFunctionDef ReviveObituary;
    BuiltinFunctionDef AddDemoBookmark;
    BuiltinFunctionDef StopDemoRecording;
    BuiltinFunctionDef PositionWouldTelefrag;
    BuiltinFunctionDef BoundsWouldTelefrag;
    BuiltinFunctionDef RecordUsedSpawnPoint;
    BuiltinFunctionDef TestSpawnPoint;
    BuiltinFunctionDef GetStartTime;
    BuiltinFunctionDef MapRestart;
    BuiltinFunctionDef MissionRestart;
    BuiltinFunctionDef MissionFailed;
    BuiltinFunctionDef ExitLevel;
    BuiltinFunctionDef KillServer;
    BuiltinFunctionDef AddTestClient;
    BuiltinFunctionDef SetObjectivePointStatus;
    BuiltinFunctionDef SetBombTimer;
    BuiltinFunctionDef SetRoundsPlayed;
    BuiltinFunctionDef SetInitialPlayersConnected;
    BuiltinFunctionDef SetMatchFlag;
    BuiltinFunctionDef SetMatchTalkFlag;
    BuiltinFunctionDef SetArchive;
    BuiltinFunctionDef AllClientsPrint;
    BuiltinFunctionDef ClientPrint;
    BuiltinFunctionDef MapExists;
    BuiltinFunctionDef IsValidGameType;
    BuiltinFunctionDef SkillUpdate;
    BuiltinFunctionDef RecordLeagueWinner;
    BuiltinFunctionDef SetTeamSpyplane;
    BuiltinFunctionDef GetTeamSpyplane;
    BuiltinFunctionDef SetTeamSatellite;
    BuiltinFunctionDef GetTeamSatellite;
    BuiltinFunctionDef GetAssignedTeam;
    BuiltinFunctionDef GetAssignedTeamName;
    BuiltinFunctionDef GetDefaultClassSlot;
    BuiltinFunctionDef GetItemAttachment;
    BuiltinFunctionDef GetItemAttachmentAllocationCost;
    BuiltinFunctionDef GetRefFromItemIndex;
    BuiltinFunctionDef GetItemGroupFromItemIndex;
    BuiltinFunctionDef GetBaseWeaponItemIndex;
    BuiltinFunctionDef GetGameTypeEnumFromName;
    BuiltinFunctionDef SetScoreboardColumns;
    BuiltinFunctionDef SetTopScorer;
    BuiltinFunctionDef ClearTopScorers;
    BuiltinFunctionDef RecordNumZombieRounds;
    BuiltinFunctionDef RecordGameResult;
    BuiltinFunctionDef FinalizeMatchRecord;
    BuiltinFunctionDef MatchRecordRoundEnd;
    BuiltinFunctionDef MatchRecordRoundStart;
    BuiltinFunctionDef MatchRecordOvertimeRound;
    BuiltinFunctionDef GetTouchingVolume;
    BuiltinFunctionDef GetFirstTouchFraction;
    BuiltinFunctionDef RecordMatchSummaryZombieEndGameData;
    BuiltinFunctionDef MatchRecorderIncrementHeaderStat;
    BuiltinFunctionDef RecordMatchInit;
    BuiltinFunctionDef InvalidateMatchRecord;
    BuiltinFunctionDef RecordZombieRoundStart;
    BuiltinFunctionDef RecordZombieRoundEnd;
    BuiltinFunctionDef GetCustomTeamName;
    BuiltinFunctionDef ClearPlayerCorpses;
    BuiltinFunctionDef RecordPlayerStats;
    BuiltinFunctionDef RecordPlayerMatchEnd;
    BuiltinFunctionDef RecordMatchBegin;
    BuiltinFunctionDef RecordBreadcrumbDataForPlayer;
    BuiltinFunctionDef MatchRecordNewPlayer;
    BuiltinFunctionDef MatchRecordLogAdditionalDeathInfo;
    BuiltinFunctionDef MatchRecordLogSpecialMoveDataForLife;
    BuiltinFunctionDef MatchRecordLogChallengeComplete;
    BuiltinFunctionDef MatchRecordSetCurrentLevelComplete;
    BuiltinFunctionDef MatchRecordSetLevelDifficultyForIndex;
    BuiltinFunctionDef MatchRecordGetWeaponIndex;
    BuiltinFunctionDef SetVoteString;
    BuiltinFunctionDef SetVoteTime;
    BuiltinFunctionDef SetVoteYesCount;
    BuiltinFunctionDef SetVoteNoCount;
    BuiltinFunctionDef ArenaGetSlot;
    BuiltinFunctionDef ArenaGetCurrentSeason;
    BuiltinFunctionDef ReportMTU;
    BuiltinFunctionDef Kick;
    BuiltinFunctionDef Ban;
    BuiltinFunctionDef Map;
    BuiltinFunctionDef PlayRumbleOnPosition;
    BuiltinFunctionDef PlayRumbleLoopOnPosition;
    BuiltinFunctionDef StopAllRumbles;
    BuiltinFunctionDef SoundExists;
    BuiltinFunctionDef SoundGetPlaybackTime;
    BuiltinFunctionDef SoundGetAlias;
    BuiltinFunctionDef IsSplitscreen;
    BuiltinFunctionDef IsLocalGame;
    BuiltinFunctionDef IsGlobalStatsServer;
    BuiltinFunctionDef SetMinimap;
    BuiltinFunctionDef SetMapCenter;
    BuiltinFunctionDef SetDemoIntermissionPoint;
    BuiltinFunctionDef SetGameEndTime;
    BuiltinFunctionDef SetSlowMotion;
    BuiltinFunctionDef SetPauseWorld;
    BuiltinFunctionDef IsWorldPaused;
    BuiltinFunctionDef NumRemoteClients;
    BuiltinFunctionDef VisionSetNaked;
    BuiltinFunctionDef VisionSetLastStand;
    BuiltinFunctionDef SetGravity;
    BuiltinFunctionDef SetWaveWaterGeneratorAmplitude;
    BuiltinFunctionDef EndLobby;
    BuiltinFunctionDef GetLobbyClientCount;
    BuiltinFunctionDef EnableLobbyJoins;
    BuiltinFunctionDef ClientSysRegister;
    BuiltinFunctionDef ClientSysSetState;
    BuiltinFunctionDef GetSnapshotIndexArray;
    BuiltinFunctionDef SnapshotAcknowledged;
    BuiltinFunctionDef GetAITriggerFlags;
    BuiltinFunctionDef GetMaxVehicles;
    BuiltinFunctionDef DisableDestructiblePieces;
    BuiltinFunctionDef EnableAllDestructiblePieces;
    BuiltinFunctionDef CreateDynEntAndLaunch;
    BuiltinFunctionDef GetVehicleTriggerFlags;
    BuiltinFunctionDef GetEntNavMaterial;
    BuiltinFunctionDef CollisionTestPointsInSphere;
    BuiltinFunctionDef CollisionTestPointsInCylinder;
    BuiltinFunctionDef CollisionTestPointsInPill;
    BuiltinFunctionDef CollisionTestPointsInCone;
    BuiltinFunctionDef CollisionTestPointsInBox;
    BuiltinFunctionDef QSortScoredSpawnPointsAscending;
    BuiltinFunctionDef Matrix4x4TransformPoints;
    BuiltinFunctionDef EnableSpawnPointList;
    BuiltinFunctionDef DisableSpawnPointList;
    BuiltinFunctionDef SetSpawnPointRandomVariation;
    BuiltinFunctionDef ClearSpawnPoints;
    BuiltinFunctionDef PlaceSpawnPoint;
    BuiltinFunctionDef AddSpawnPoints;
    BuiltinFunctionDef GetBestSpawnPoint;
    BuiltinFunctionDef ClearSpawnPointsBaseWeight;
    BuiltinFunctionDef SetSpawnPointsBaseWeight;
    BuiltinFunctionDef GetPlayerSpawnId;
    BuiltinFunctionDef IsSpawnPointVisible;
    BuiltinFunctionDef AddInfluencer;
    BuiltinFunctionDef AddEntityInfluencer;
    BuiltinFunctionDef AddOrientedInfluencer;
    BuiltinFunctionDef RemoveInfluencer;
    BuiltinFunctionDef EnableInfluencer;
    BuiltinFunctionDef SetInfluencerTeamMask;
    BuiltinFunctionDef SetInfluencerTimeOut;
    BuiltinFunctionDef GetInfluencerTimeOutRemaining;
    BuiltinFunctionDef SetDebugSideSwitch;
    BuiltinFunctionDef GetInfluencerPreset;
    BuiltinFunctionDef Target_Set;
    BuiltinFunctionDef Target_SetOffset;
    BuiltinFunctionDef Target_GetOffset;
    BuiltinFunctionDef Target_Remove;
    BuiltinFunctionDef Target_SetShader;
    BuiltinFunctionDef Target_SetOffscreenShader;
    BuiltinFunctionDef Target_IsInRect;
    BuiltinFunctionDef Target_IsInCircle;
    BuiltinFunctionDef Target_ScaleMinMaxRadius;
    BuiltinFunctionDef Target_OriginIsInCircle;
    BuiltinFunctionDef Target_BoundingIsUnderReticle;
    BuiltinFunctionDef Target_StartReticleLockOn;
    BuiltinFunctionDef Target_ClearReticleLockOn;
    BuiltinFunctionDef Target_GetArray;
    BuiltinFunctionDef Target_IsTarget;
    BuiltinFunctionDef Target_SetAttackMode;
    BuiltinFunctionDef Target_SetJavelinOnly;
    BuiltinFunctionDef Target_SetTurretAquire;
    BuiltinFunctionDef Target_SetAllowHighSteering;
    BuiltinFunctionDef GetMaxActiveContracts;
    BuiltinFunctionDef GetContractStatType;
    BuiltinFunctionDef GetContractStatName;
    BuiltinFunctionDef GetContractRewardXP;
    BuiltinFunctionDef GetContractRewardCP;
    BuiltinFunctionDef GetContractRequirements;
    BuiltinFunctionDef GetContractName;
    BuiltinFunctionDef GetContractRequiredCount;
    BuiltinFunctionDef GetContractResetConditions;
    BuiltinFunctionDef GetFogSettings;
    BuiltinFunctionDef PIXBeginEvent;
    BuiltinFunctionDef PIXEndEvent;
    BuiltinFunctionDef PIXMarker;
    BuiltinFunctionDef ChangeAdvertisedStatus;
    BuiltinFunctionDef SetQoSGameDataPayload;
    BuiltinFunctionDef ResetQoSGameDataPayload;
    BuiltinFunctionDef IncrementCounter;
    BuiltinFunctionDef GetCounterTotal;
    BuiltinFunctionDef ForceUploadCounters;
    BuiltinFunctionDef EnableOccluder;
    BuiltinFunctionDef SetHostMigrationStatus;
    BuiltinFunctionDef StartHostMigration;
    BuiltinFunctionDef GameRepThresholdExceeded;
    BuiltinFunctionDef ZeroGravityVolumeOn;
    BuiltinFunctionDef ZeroGravityVolumeOff;
    BuiltinFunctionDef GetUTC;
    BuiltinFunctionDef IsUsingT7Melee;
    BuiltinFunctionDef RegisterSkipto;
    BuiltinFunctionDef GetSkiptoName;
    BuiltinFunctionDef GetSkipTOS;
    BuiltinFunctionDef SetSkipTOS;
    BuiltinFunctionDef AIProfile_BeginEntry;
    BuiltinFunctionDef AIProfile_EndEntry;
    BuiltinFunctionDef StreamerModelHint;
    BuiltinFunctionDef StreamerRequest;
    BuiltinFunctionDef GetCyberComWeapon;
    BuiltinFunctionDef GetCyberComAbilityName;
    BuiltinFunctionDef CloneAndRemoveEntity;
    BuiltinFunctionDef LoadSentientEventParameters;
    BuiltinFunctionDef HkaiGetTimerTestPathfindStartPos;
    BuiltinFunctionDef HkaiGetTimerTestPathfindEndPos;
    BuiltinFunctionDef HkaiSetTimerTestEnt;
    BuiltinFunctionDef GetTotalServerPauseTime;
    BuiltinFunctionDef HashString;
    BuiltinFunctionDef PROTECTED(SetBGBUnlocked);
    BuiltinFunctionDef PROTECTED(GetBGBUnlocked);
  };
  static inline constexpr size_t COUNT = 336;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
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
      PROTECTED_STR(SetBGBUnlocked),
      PROTECTED_STR(GetBGBUnlocked),
  };

  DEFINE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(BuiltinFunctionTable, sizeof(BuiltinFunctionTable::functions));

union BuiltinMethodTable {
  struct {
    BuiltinMethodDef SetStowedWeapon;
    BuiltinMethodDef GetStowedWeapon;
    BuiltinMethodDef ClearStowedWeapon;
    BuiltinMethodDef GetAmmoCount;
    BuiltinMethodDef SetVisibleToPlayer;
    BuiltinMethodDef SetInvisibleToPlayer;
    BuiltinMethodDef SetVisibleToAll;
    BuiltinMethodDef SetInvisibleToAll;
    BuiltinMethodDef SetVisibleToTeam;
    BuiltinMethodDef SetVisibleToAllExceptTeam;
    BuiltinMethodDef SetForceNoCull;
    BuiltinMethodDef RemoveForceNoCull;
    BuiltinMethodDef SetNoSunShadow;
    BuiltinMethodDef RemoveNoSunShadow;
    BuiltinMethodDef IsLinkedTo;
    BuiltinMethodDef AllowTacticalInsertion;
    BuiltinMethodDef AllowBotTargetting;
    BuiltinMethodDef GetLinkedEnt;
    BuiltinMethodDef GetOrigin;
    BuiltinMethodDef GetAngles;
    BuiltinMethodDef GetMins;
    BuiltinMethodDef GetMaxs;
    BuiltinMethodDef GetAbsMins;
    BuiltinMethodDef GetAbsMaxs;
    BuiltinMethodDef GetPointInBounds;
    BuiltinMethodDef GetEye;
    BuiltinMethodDef GetEyeApprox;
    BuiltinMethodDef UseBy;
    BuiltinMethodDef SetStableMissile;
    BuiltinMethodDef PlayerSetGroundReferenceEnt;
    BuiltinMethodDef IsTouching;
    BuiltinMethodDef IsTouchingSwept;
    BuiltinMethodDef IsTouchingVolume;
    BuiltinMethodDef PlaySound;
    BuiltinMethodDef PlaySoundWithNotify;
    BuiltinMethodDef PlaySoundOnTag;
    BuiltinMethodDef PlaySoundToTeam;
    BuiltinMethodDef PlayBattleChatterToTeam;
    BuiltinMethodDef PlayLoopSound;
    BuiltinMethodDef StopLoopSound;
    BuiltinMethodDef StopSounds;
    BuiltinMethodDef StopSound;
    BuiltinMethodDef PlayRumbleOnEntity;
    BuiltinMethodDef PlayRumbleLoopOnEntity;
    BuiltinMethodDef StopRumble;
    BuiltinMethodDef Delete;
    BuiltinMethodDef SetModel;
    BuiltinMethodDef SetHighDetail;
    BuiltinMethodDef SetDedicatedShadow;
    BuiltinMethodDef SetEnemyModel;
    BuiltinMethodDef DoDamage;
    BuiltinMethodDef ForcePainOn;
    BuiltinMethodDef Kill;
    BuiltinMethodDef GetNormalHealth;
    BuiltinMethodDef SetNormalHealth;
    BuiltinMethodDef SetMaxHealth;
    BuiltinMethodDef Show;
    BuiltinMethodDef Hide;
    BuiltinMethodDef IsHidden;
    BuiltinMethodDef Ghost;
    BuiltinMethodDef GhostInDemo;
    BuiltinMethodDef ShowInDemo;
    BuiltinMethodDef ShowToTeam;
    BuiltinMethodDef HideFromTeam;
    BuiltinMethodDef LaserOn;
    BuiltinMethodDef LaserOff;
    BuiltinMethodDef ShowToPlayer;
    BuiltinMethodDef SetContents;
    BuiltinMethodDef Solid;
    BuiltinMethodDef NotSolid;
    BuiltinMethodDef SolidCapsule;
    BuiltinMethodDef NotSolidCapsule;
    BuiltinMethodDef StartFiring;
    BuiltinMethodDef StopFiring;
    BuiltinMethodDef ShootTurret;
    BuiltinMethodDef StopShootTurret;
    BuiltinMethodDef SetMode;
    BuiltinMethodDef GetTurretOwner;
    BuiltinMethodDef GetTurretArcLimits;
    BuiltinMethodDef SetPlayerSpread;
    BuiltinMethodDef SetAISpread;
    BuiltinMethodDef ClearTargetEntity;
    BuiltinMethodDef MakeTurretUsable;
    BuiltinMethodDef MakeTurretUnusable;
    BuiltinMethodDef SetTurretAccuracy;
    BuiltinMethodDef GetTurretTarget;
    BuiltinMethodDef DisconnectPaths;
    BuiltinMethodDef ConnectPaths;
    BuiltinMethodDef EnableObstacle;
    BuiltinMethodDef GetStance;
    BuiltinMethodDef SetStance;
    BuiltinMethodDef SetCursorHint;
    BuiltinMethodDef SetReviveHintString;
    BuiltinMethodDef SetHintStringForPerk;
    BuiltinMethodDef SetHintString;
    BuiltinMethodDef SetHintStringForPlayer;
    BuiltinMethodDef GetTriggerAccumulate;
    BuiltinMethodDef SetFOVForKillcam;
    BuiltinMethodDef SetHintLowPriority;
    BuiltinMethodDef UseTriggerRequireLookAt;
    BuiltinMethodDef UseTriggerRequireLookToward;
    BuiltinMethodDef UseTriggerIgnoreUseHoldTime;
    BuiltinMethodDef TriggerIgnoreTeam;
    BuiltinMethodDef TriggerEnable;
    BuiltinMethodDef IsTriggerEnabled;
    BuiltinMethodDef ShellShock;
    BuiltinMethodDef GetWeaponForwardDir;
    BuiltinMethodDef GetWeaponMuzzlePoint;
    BuiltinMethodDef GetEntNum;
    BuiltinMethodDef StopShellShock;
    BuiltinMethodDef SetDepthOfField;
    BuiltinMethodDef SetBurn;
    BuiltinMethodDef SetElectrified;
    BuiltinMethodDef SpawnNapalmGroundFlame;
    BuiltinMethodDef NeedsRevive;
    BuiltinMethodDef IsInSecondChance;
    BuiltinMethodDef DepthInWater;
    BuiltinMethodDef ShootUp;
    BuiltinMethodDef DepthOfPlayerInWater;
    BuiltinMethodDef IsPlayerSwimming;
    BuiltinMethodDef WalkUnderwater;
    BuiltinMethodDef IsPlayerUnderwater;
    BuiltinMethodDef GetLastOutWaterTime;
    BuiltinMethodDef UseWeaponHideTags;
    BuiltinMethodDef UseWeaponModel;
    BuiltinMethodDef UseBuildKitWeaponModel;
    BuiltinMethodDef SetWaterDrops;
    BuiltinMethodDef IsFiringTurret;
    BuiltinMethodDef IsTurretLockedOn;
    BuiltinMethodDef SetViewModelDepthOfField;
    BuiltinMethodDef ViewKick;
    BuiltinMethodDef EntityRadiusDamage;
    BuiltinMethodDef CylinderDamage;
    BuiltinMethodDef Detonate;
    BuiltinMethodDef DamageConeTrace;
    BuiltinMethodDef SightConeTrace;
    BuiltinMethodDef HeliTurretSightTrace;
    BuiltinMethodDef HeliTurretDogTrace;
    BuiltinMethodDef PlayerSightTrace;
    BuiltinMethodDef VisionSetLerpRatio;
    BuiltinMethodDef DoCowardsWayAnims;
    BuiltinMethodDef StartPoisoning;
    BuiltinMethodDef StopPoisoning;
    BuiltinMethodDef StartBinocs;
    BuiltinMethodDef StopBinocs;
    BuiltinMethodDef IsFlared;
    BuiltinMethodDef IsPoisoned;
    BuiltinMethodDef SetCameraSpikeActive;
    BuiltinMethodDef IsInsideHeightLock;
    BuiltinMethodDef IsMissileInsideHeightLock;
    BuiltinMethodDef IsOnGround;
    BuiltinMethodDef GetGroundEnt;
    BuiltinMethodDef GetMoverEnt;
    BuiltinMethodDef IsSprinting;
    BuiltinMethodDef IsWallRunning;
    BuiltinMethodDef GetWallRunWallNormal;
    BuiltinMethodDef IsDoubleJumping;
    BuiltinMethodDef SetDoubleJumpEnergy;
    BuiltinMethodDef ResetDoubleJumpRechargeTime;
    BuiltinMethodDef IsSliding;
    BuiltinMethodDef IsTraversing;
    BuiltinMethodDef IsSlamming;
    BuiltinMethodDef GetPathfindingRadius;
    BuiltinMethodDef GetAnimTime;
    BuiltinMethodDef SetAnimTime;
    BuiltinMethodDef SetAnimTimeByName;
    BuiltinMethodDef UseAnimTree;
    BuiltinMethodDef AnimScripted;
    BuiltinMethodDef AnimRelative;
    BuiltinMethodDef StopAnimScripted;
    BuiltinMethodDef IsPlayingAnimScripted;
    BuiltinMethodDef SetEntityAnimRate;
    BuiltinMethodDef GetEntityAnimRate;
    BuiltinMethodDef GetCentroid;
    BuiltinMethodDef SetLightingOnly;
    BuiltinMethodDef SetIncludeMeshes;
    BuiltinMethodDef WeaponHasAttachmentAndUnlocked;
    BuiltinMethodDef AllWeaponAttachmentsUnlocked;
    BuiltinMethodDef GetTotalUnlockedWeaponAttachments;
    BuiltinMethodDef GetEntityNumber;
    BuiltinMethodDef GetEntityType;
    BuiltinMethodDef EnableGrenadeTouchDamage;
    BuiltinMethodDef DisableGrenadeTouchDamage;
    BuiltinMethodDef EnableAimAssist;
    BuiltinMethodDef DisableAimAssist;
    BuiltinMethodDef PlaceSpawnPointOld;
    BuiltinMethodDef SetSpawnClientFlag;
    BuiltinMethodDef DirectionalHitIndicator;
    BuiltinMethodDef SendKillstreakDamageEvent;
    BuiltinMethodDef GetPlayerSpeed;
    BuiltinMethodDef GetPlayerGravity;
    BuiltinMethodDef SetPlayerGravity;
    BuiltinMethodDef ClearPlayerGravity;
    BuiltinMethodDef SendFaceEvent;
    BuiltinMethodDef SetTeamForTrigger;
    BuiltinMethodDef SetExcludeTeamForTrigger;
    BuiltinMethodDef SetPerkForTrigger;
    BuiltinMethodDef SetIgnoreEntForTrigger;
    BuiltinMethodDef ClientClaimTrigger;
    BuiltinMethodDef ClientReleaseTrigger;
    BuiltinMethodDef ReleaseClaimedTrigger;
    BuiltinMethodDef IsItemLocked;
    BuiltinMethodDef IsItemPurchased;
    BuiltinMethodDef UploadLeaderboards;
    BuiltinMethodDef SetNemesisXUID;
    BuiltinMethodDef GetLoadoutItem;
    BuiltinMethodDef IsBonusCardActive;
    BuiltinMethodDef BonusCardActiveCount;
    BuiltinMethodDef IsSpecialistUnlocked;
    BuiltinMethodDef GetSpecialistIndex;
    BuiltinMethodDef GetLoadoutItemRef;
    BuiltinMethodDef SetPlayerStateLoadoutBonusCards;
    BuiltinMethodDef SetPlayerStateLoadoutWeapons;
    BuiltinMethodDef GetLoadoutWeapon;
    BuiltinMethodDef GetAttachmentCosmeticVariantForWeapon;
    BuiltinMethodDef GetLoadoutPerks;
    BuiltinMethodDef GetLoadoutAllocation;
    BuiltinMethodDef GetLoadoutGunSmithVariantIndex;
    BuiltinMethodDef ConsumableIncrement;
    BuiltinMethodDef ConsumableDecrement;
    BuiltinMethodDef ConsumableGet;
    BuiltinMethodDef ConsumableSet;
    BuiltinMethodDef GetBubbleGumPack;
    BuiltinMethodDef GetBGBRemaining;
    BuiltinMethodDef GetBGBTokensGained;
    BuiltinMethodDef IncrementBGBTokensGained;
    BuiltinMethodDef GetBGBTokensUsed;
    BuiltinMethodDef IncrementBGBTokensUsed;
    BuiltinMethodDef GetBGBTokensRemaining;
    BuiltinMethodDef GetBuildKitWeapon;
    BuiltinMethodDef GetBuildKitWeaponOptions;
    BuiltinMethodDef GetBuildKitAttachmentCosmeticVariantIndexes;
    BuiltinMethodDef SetBodyRenderOptions;
    BuiltinMethodDef SetMoveSpeedScale;
    BuiltinMethodDef GetMoveSpeedScale;
    BuiltinMethodDef Missile_SetTarget;
    BuiltinMethodDef Missile_GetTarget;
    BuiltinMethodDef Missile_DroneSetVisible;
    BuiltinMethodDef BallSetTarget;
    BuiltinMethodDef IsOnLadder;
    BuiltinMethodDef IsMantling;
    BuiltinMethodDef IsOnSlide;
    BuiltinMethodDef BloodImpact;
    BuiltinMethodDef StartRagDoll;
    BuiltinMethodDef IsRagdoll;
    BuiltinMethodDef LaunchRagdoll;
    BuiltinMethodDef LaunchVehicle;
    BuiltinMethodDef ApplyBallisticTarget;
    BuiltinMethodDef SetBlur;
    BuiltinMethodDef ResetMissileDetonationTime;
    BuiltinMethodDef GiveAchievement;
    BuiltinMethodDef SetZombieName;
    BuiltinMethodDef SetEntGravityTrajectory;
    BuiltinMethodDef SetTeam;
    BuiltinMethodDef GetTeam;
    BuiltinMethodDef SetOwner;
    BuiltinMethodDef SetMissileOwner;
    BuiltinMethodDef SetEntityWeapon;
    BuiltinMethodDef SetTurretOwner;
    BuiltinMethodDef SetTurretType;
    BuiltinMethodDef GetCorpseAnim;
    BuiltinMethodDef ItemWeaponSetAmmo;
    BuiltinMethodDef SetWeaponRenderOptions;
    BuiltinMethodDef SetAttachmentCosmeticVariants;
    BuiltinMethodDef SetAttachmentCosmeticVariantIndex;
    BuiltinMethodDef SetTurretCarried;
    BuiltinMethodDef SetTurretMinimapVisible;
    BuiltinMethodDef SetMissileCoasting;
    BuiltinMethodDef SetMissileBrake;
    BuiltinMethodDef SetWeapon;
    BuiltinMethodDef SetWeaponOptions;
    BuiltinMethodDef SetAttachmentCosmeticVariantIndexes;
    BuiltinMethodDef Gib;
    BuiltinMethodDef SetPlayerCollision;
    BuiltinMethodDef ReportUser;
    BuiltinMethodDef SetPlayerCurrentObjective;
    BuiltinMethodDef SetPlayerCurrentStreak;
    BuiltinMethodDef GetVelocity;
    BuiltinMethodDef SpawnFromSpawner;
    BuiltinMethodDef PredictSpawnPoint;
    BuiltinMethodDef SetDefaultDropPitch;
    BuiltinMethodDef SetScanningPitch;
    BuiltinMethodDef LaunchBomb;
    BuiltinMethodDef MagicMissile;
    BuiltinMethodDef MagicGrenadePlayer;
    BuiltinMethodDef MakeGrenadeDud;
    BuiltinMethodDef FakeFire;
    BuiltinMethodDef SetFakeFire;
    BuiltinMethodDef MakeUsable;
    BuiltinMethodDef MakeUnusable;
    BuiltinMethodDef PredictGrenade;
    BuiltinMethodDef SetDangerous;
    BuiltinMethodDef IsDangerous;
    BuiltinMethodDef GetIndexForActiveContract;
    BuiltinMethodDef GetActiveContractProgress;
    BuiltinMethodDef IncrementActiveContractProgress;
    BuiltinMethodDef IncrementActiveContractTime;
    BuiltinMethodDef IsActiveContractComplete;
    BuiltinMethodDef HasActiveContractExpired;
    BuiltinMethodDef GetActiveContractTimePassed;
    BuiltinMethodDef ResetActiveContractProgress;
    BuiltinMethodDef RecordLeaguePreLoser;
    BuiltinMethodDef LUINotifyEventToSpectators;
    BuiltinMethodDef IsTestClient;
    BuiltinMethodDef SetWorldFogActiveBank;
    BuiltinMethodDef IsZBarrier;
    BuiltinMethodDef SetClone;
    BuiltinMethodDef GetNumZBarrierPieces;
    BuiltinMethodDef GetZBarrierPieceState;
    BuiltinMethodDef SetZBarrierPieceState;
    BuiltinMethodDef IsZBarrierOpen;
    BuiltinMethodDef IsZBarrierClosed;
    BuiltinMethodDef HideZBarrierPiece;
    BuiltinMethodDef ShowZBarrierPiece;
    BuiltinMethodDef ZBarrierPieceUseDefaultModel;
    BuiltinMethodDef ZBarrierPieceUseAlternateModel;
    BuiltinMethodDef ZBarrierPieceUseUpgradedModel;
    BuiltinMethodDef ZBarrierPieceUseBoxRiseLogic;
    BuiltinMethodDef ZBarrierPieceUseAttachWeapon;
    BuiltinMethodDef GetUpgradedPieceNumLives;
    BuiltinMethodDef GetZBarrierPieceIndicesInState;
    BuiltinMethodDef GetZBarrierPieceAnimLengthForState;
    BuiltinMethodDef GetZBarrierPieceAnimState;
    BuiltinMethodDef GetZBarrierPieceAnimSubState;
    BuiltinMethodDef ZBarrierSupportsZombieTaunts;
    BuiltinMethodDef ZBarrierSupportsZombieReachThroughAttacks;
    BuiltinMethodDef GetZBarrierTauntAnimState;
    BuiltinMethodDef GetZBarrierReachThroughAttackAnimState;
    BuiltinMethodDef GetZBarrierNumAttackSlots;
    BuiltinMethodDef GetZBarrierAttackSlotHorzOffset;
    BuiltinMethodDef SetZBarrierColModel;
    BuiltinMethodDef GetWeaponOptic;
    BuiltinMethodDef ArenaBeginMatch;
    BuiltinMethodDef ArenaEndMatch;
    BuiltinMethodDef GetPathMetric;
    BuiltinMethodDef SetIgnorePauseWorld;
    BuiltinMethodDef SetEntityPaused;
    BuiltinMethodDef IsPaused;
    BuiltinMethodDef IsStreamerReady;
    BuiltinMethodDef IsClientCacheStable;
    BuiltinMethodDef IsLoadingCinematicPlaying;
    BuiltinMethodDef BotReleaseManualControl;
    BuiltinMethodDef SetBallPassAllowed;
    BuiltinMethodDef QueueMeleeActionState;
    BuiltinMethodDef StopJukeMove;
  };
  static inline constexpr size_t COUNT = 345;
  BuiltinMethodDef methods[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
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

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(methods);
};
ASSERT_SIZE(BuiltinMethodTable, sizeof(BuiltinMethodTable::methods));

} // namespace gscr
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game