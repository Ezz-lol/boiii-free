#pragma once

#include <game/structs/scr/builtin/table/macros.hpp>
#include <game/structs/scr/weapon.hpp>

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace weapon {

using namespace game::scr::weapon;
// All fields below, except those with comments immediately above indicating
// otherwise, have `nullptr` `getter`s.
PACKED(union FieldTable {
  struct {
    scr_weapon_field_t AIFuseTime;
    scr_weapon_field_t ProjExplosionSound;
    scr_weapon_field_t ProjExplosionSoundPlayer;
    scr_weapon_field_t ProjSmokeStartSound;
    scr_weapon_field_t ProjSmokeLoopSound;
    scr_weapon_field_t ProjSmokeEndSound;
    scr_weapon_field_t MeleeChargeRange;
    scr_weapon_field_t MeleeLungeRange;
    scr_weapon_field_t DoGibbing;
    scr_weapon_field_t DoGibbingOnMelee;
    scr_weapon_field_t DoAnnihilate;
    scr_weapon_field_t DoBlowback;
    scr_weapon_field_t MaxGibDistance;
    scr_weapon_field_t LeftArc;
    scr_weapon_field_t RightArc;
    scr_weapon_field_t TopArc;
    scr_weapon_field_t BottomArc;
    scr_weapon_field_t ClipModel;
    scr_weapon_field_t FightDist;
    scr_weapon_field_t MaxDist;
    scr_weapon_field_t SpinUpTime;
    scr_weapon_field_t SpinDownTime;
    scr_weapon_field_t FuelLife;
    scr_weapon_field_t IsBoltAction;
    scr_weapon_field_t IsDisallowAtMatchStart;
    scr_weapon_field_t FireSound;
    scr_weapon_field_t FireSoundPlayer;
    scr_weapon_field_t BlocksProne;
    scr_weapon_field_t IsClipOnly;
    scr_weapon_field_t LockOnRadius;
    scr_weapon_field_t LockOnLossRadius;
    scr_weapon_field_t RequireLockOnToFire;
    scr_weapon_field_t SetUsedStat;
    scr_weapon_field_t MaxInstancesAllowed;
    scr_weapon_field_t IsEMP;
    scr_weapon_field_t IsFlash;
    scr_weapon_field_t IsStun;
    scr_weapon_field_t BulletImpactExplode;
    scr_weapon_field_t DoEMPDestroyFx;
    scr_weapon_field_t DoStun;
    scr_weapon_field_t DoDamageFeedback;
    scr_weapon_field_t DoHackedStats;
    scr_weapon_field_t HackerTriggerOriginTag;
    scr_weapon_field_t SpawnInfluencer;
    scr_weapon_field_t AnyPlayerCanRetrieve;
    scr_weapon_field_t IsTacticalInsertion;
    scr_weapon_field_t IsValuable;
    scr_weapon_field_t DestroyableByTrophySystem;
    scr_weapon_field_t DrawOffHandModelInHand;
    scr_weapon_field_t DisallowAtMatchStart;
    scr_weapon_field_t NonStowedWeapon;
    scr_weapon_field_t IsScavengable;
    scr_weapon_field_t DoesFireDamage;
    scr_weapon_field_t IgnoresFlakJacket;
    scr_weapon_field_t NotKillstreak;
    scr_weapon_field_t IsGameplayWeapon;
    scr_weapon_field_t IsSupplyDropWeapon;
    scr_weapon_field_t SkipBattleChatterKill;
    scr_weapon_field_t SkipBattleChatterReload;
    scr_weapon_field_t DoNotDamageOwner;
    scr_weapon_field_t DestroysEquipment;
    scr_weapon_field_t ForceDamageShellshockAndRumble;
    scr_weapon_field_t IsAIKillstreakDamage;
    scr_weapon_field_t IgnoreTeamKills;
    scr_weapon_field_t TeamKillPenaltyScale;
    scr_weapon_field_t VehicleProjectileDamageScalar;
    scr_weapon_field_t VehicleProjectileSplashDamageScalar;
    scr_weapon_field_t IsBallisticKnife;
    scr_weapon_field_t IsPerkBottle;
    scr_weapon_field_t SkipLowAmmoVox;
    scr_weapon_field_t IsFlourishWeapon;
    scr_weapon_field_t IsHybridWeapon;
    scr_weapon_field_t DisableDeploy;
    scr_weapon_field_t IsSniperWeapon;
    scr_weapon_field_t IsHackToolWeapon;
    scr_weapon_field_t MeleeIgnoresLightArmor;
    scr_weapon_field_t IgnoresLightArmor;
    scr_weapon_field_t IgnoresPowerArmor;
    scr_weapon_field_t SoundRattleRangeMin;
    scr_weapon_field_t SoundRattleRangeMax;
    scr_weapon_field_t GrappleWeapon;
    scr_weapon_field_t BurstCount;
    scr_weapon_field_t WeaponHeadObjectiveHeight;
    scr_weapon_field_t EnemyCrosshairRange;
    scr_weapon_field_t UnlimitedAmmo;
    scr_weapon_field_t IsNotDroppable;
    scr_weapon_field_t DamageAlwaysKillsPlayer;
    scr_weapon_field_t DamageToOwnerScalar;
    scr_weapon_field_t ViewModels;
    scr_weapon_field_t FrontendModel;
    scr_weapon_field_t WorldModel;
    scr_weapon_field_t WorldDamagedModel1;
    scr_weapon_field_t WorldDamagedModel2;
    scr_weapon_field_t WorldDamagedModel3;
    scr_weapon_field_t StowedModel;
    scr_weapon_field_t ShownEnemyEquip;
    scr_weapon_field_t ShownEnemyExplo;
    scr_weapon_field_t ShownRetrievable;
    scr_weapon_field_t LockOnMinRange;
    scr_weapon_field_t LockOnScreenRadius;
    scr_weapon_field_t LockOnAngleHorizontal;
    scr_weapon_field_t LockOnAngleVertical;
    scr_weapon_field_t LockOnLossAngleHorizontal;
    scr_weapon_field_t LockOnLossAngleVertical;
    // Has `getter`
    scr_weapon_field_t IsValid;
    // Has `getter`
    scr_weapon_field_t RootWeapon;
    // Has `getter`
    scr_weapon_field_t Attachments;
    // Has `getter`
    scr_weapon_field_t SupportedAttachments;
    // Has `getter`
    scr_weapon_field_t StartAmmo;
    // Has `getter`
    scr_weapon_field_t MaxAmmo;
    // Has `getter`
    scr_weapon_field_t GuidedMissileType;
    // Has `getter`
    scr_weapon_field_t LockOnType;
    // Has `getter`
    scr_weapon_field_t IsRocketLauncher;
    scr_weapon_field_t LockOnSeekerSeArchSound;
    scr_weapon_field_t LockOnSeekerSeArchSoundLoops;
    scr_weapon_field_t LockOnSeekerLockedSound;
    scr_weapon_field_t LockOnSeekerLockedSoundLoops;
    scr_weapon_field_t LockOnTargetLockedSound;
    scr_weapon_field_t LockOnTargetLockedSoundLoops;
    scr_weapon_field_t LockOnTargetFiredOnSound;
    scr_weapon_field_t LockOnTargetFiredOnSoundLoops;
    scr_weapon_field_t ForceDamageHitLocation;
    // Has `getter`
    scr_weapon_field_t Type;
    // Has `getter`
    scr_weapon_field_t IsBulletWeapon;
    // Has `getter`
    scr_weapon_field_t IsGrenadeWeapon;
    // Has `getter`
    scr_weapon_field_t IsProjectileWeapon;
    // Has `getter`
    scr_weapon_field_t IsGasWeapon;
    // Has `getter`
    scr_weapon_field_t IsRiotShield;
    // Has `getter`
    scr_weapon_field_t WeapClass;
    // Has `getter`
    scr_weapon_field_t IsKillstreak;
    scr_weapon_field_t IsCarriedKillstreak;
    // Has `getter`
    scr_weapon_field_t OffHandClass;
    // Has `getter`
    scr_weapon_field_t OffHandSlot;
    // Has `getter`
    scr_weapon_field_t IsLethalGrenade;
    // Has `getter`
    scr_weapon_field_t IsTacticalGrenade;
    // Has `getter`
    scr_weapon_field_t IsEquipment;
    // Has `getter`
    scr_weapon_field_t IsSpecificUse;
    // Has `getter`
    scr_weapon_field_t InventoryType;
    // Has `getter`
    scr_weapon_field_t IsPrimary;
    // Has `getter`
    scr_weapon_field_t IsItem;
    // Has `getter`
    scr_weapon_field_t IsAltMode;
    // Has `getter`
    scr_weapon_field_t ProjExplosionType;
    // Has `getter`
    scr_weapon_field_t IsGadget;
    // Has `getter`
    scr_weapon_field_t IsHeroWeapon;
    scr_weapon_field_t Gadget_HeroVersion_2_0;
    scr_weapon_field_t Gadget_BreadcrumbDuration;
    scr_weapon_field_t Gadget_FlickerOnDamage;
    scr_weapon_field_t Gadget_FlickerOnPowerLoss;
    scr_weapon_field_t Gadget_FlickerOnPowerLow;
    scr_weapon_field_t Gadget_Max_HitPoints;
    scr_weapon_field_t Gadget_Power_Consume_On_Ammo_Use;
    scr_weapon_field_t Gadget_PowerMoveLoss;
    scr_weapon_field_t Gadget_PowerMoveSpeed;
    scr_weapon_field_t Gadget_PowerGainScoreFactor;
    scr_weapon_field_t Gadget_PowerGainScoreIgnoreSelf;
    scr_weapon_field_t Gadget_PowerGainScoreIgnoreWhenActive;
    scr_weapon_field_t Gadget_PowerOffLossOnDamage;
    scr_weapon_field_t Gadget_PowerOnlossOnDamage;
    scr_weapon_field_t Gadget_PowerReplenishFactor;
    scr_weapon_field_t Gadget_Power_Reset_On_Spawn;
    scr_weapon_field_t Gadget_Power_Reset_On_Class_Change;
    scr_weapon_field_t Gadget_Power_Reset_On_Team_Change;
    scr_weapon_field_t Gadget_Power_Reset_On_Round_Switch;
    scr_weapon_field_t Gadget_Power_Round_End_Active_Penalty;
    scr_weapon_field_t Gadget_Power_Usage_Rate;
    scr_weapon_field_t Gadget_PowerTakedownGain;
    scr_weapon_field_t Gadget_TakeDownRevealTime;
    scr_weapon_field_t Gadget_Type;
    scr_weapon_field_t Gadget_ShieldReflectPowerGain;
    scr_weapon_field_t Gadget_ShieldReflectPowerLoss;
    scr_weapon_field_t Gadget_Shockfield_Radius;
    scr_weapon_field_t Gadget_Shockfield_Damage;
    scr_weapon_field_t Gadget_Turnoff_OnEMPJammed;
    // Has `getter`
    scr_weapon_field_t Name;
    // Has `getter`
    scr_weapon_field_t DisplayName;
    // Has `getter`
    scr_weapon_field_t FireType;
    // Has `getter`
    scr_weapon_field_t IsFullAuto;
    // Has `getter`
    scr_weapon_field_t IsSemiAuto;
    // Has `getter`
    scr_weapon_field_t IsBurstFire;
    // Has `getter`
    scr_weapon_field_t IsStackedFire;
    // Has `getter`
    scr_weapon_field_t IsAllLockedFire;
    // Has `getter`
    scr_weapon_field_t IsChargeShot;
    // Has `getter`
    scr_weapon_field_t IsLauncher;
    // Has `getter`
    scr_weapon_field_t ClipSize;
    // Has `getter`
    scr_weapon_field_t ShotCount;
    // Has `getter`
    scr_weapon_field_t IsMeleeWeapon;
    // Has `getter`
    scr_weapon_field_t DeathCamTime;
    // Has `getter`
    scr_weapon_field_t FireTime;
    // Has `getter`
    scr_weapon_field_t ReloadTime;
    // Has `getter`
    scr_weapon_field_t MeleeTime;
    // Has `getter`
    scr_weapon_field_t MeleePowerTime;
    // Has `getter`
    scr_weapon_field_t MeleePowerTimeLeft;
    // Has `getter`
    scr_weapon_field_t MeleeChargeTime;
    scr_weapon_field_t MeleeDamage;
    // Has `getter`
    scr_weapon_field_t AltWeapon;
    // Has `getter`
    scr_weapon_field_t StatIndex;
    // Has `getter`
    scr_weapon_field_t DualWieldWeapon;
    // Has `getter`
    scr_weapon_field_t IsDualWield;
    // Has `getter`
    scr_weapon_field_t FuseTime;
    // Has `getter`
    scr_weapon_field_t IsTimedDetonation;
    // Has `getter`
    scr_weapon_field_t AllowsDetonationDuringReload;
    // Has `getter`
    scr_weapon_field_t ProximityDetonation;
    // Has `getter`
    scr_weapon_field_t ProximityAlarmInnerRadius;
    // Has `getter`
    scr_weapon_field_t ProximityAlarmOuterRadius;
    // Has `getter`
    scr_weapon_field_t ProximityAlarmActivationDelay;
    // Has `getter`
    scr_weapon_field_t ChainEventRadius;
    // Has `getter`
    scr_weapon_field_t ChainEventTime;
    // Has `getter`
    scr_weapon_field_t ChainEventMax;
    // Has `getter`
    scr_weapon_field_t CookOffHoldTime;
    // Has `getter`
    scr_weapon_field_t MultiDetonation;
    // Has `getter`
    scr_weapon_field_t MultiDetonationFragmentSpeed;
    // Has `getter`
    scr_weapon_field_t ExplosionRadius;
    // Has `getter`
    scr_weapon_field_t ExplosionInnerRadius;
    // Has `getter`
    scr_weapon_field_t LockOnMaxRange;
    // Has `getter`
    scr_weapon_field_t LockOnMaxRangeNoLineOfSight;
    // Has `getter`
    scr_weapon_field_t LockOnSpeed;
    // Has `getter`
    scr_weapon_field_t AmmoCountEquipment;
    // Has `getter`
    scr_weapon_field_t Gadget_PowerSprintLoss;
    // Has `getter`
    scr_weapon_field_t Gadget_Pulse_Duration;
    // Has `getter`
    scr_weapon_field_t Gadget_Pulse_Margin;
    // Has `getter`
    scr_weapon_field_t Gadget_Pulse_Max_Range;
    // Has `getter`
    scr_weapon_field_t Gadget_PowerMax;
    scr_weapon_field_t WeaponStartHitPoints;
    scr_weapon_field_t WeaponDamage1HitPoints;
    scr_weapon_field_t WeaponDamage2HitPoints;
    scr_weapon_field_t WeaponDamage3HitPoints;
    scr_weapon_field_t NoHitMarker;
    scr_weapon_field_t SpecialPain;
    scr_weapon_field_t Decoy;
    scr_weapon_field_t AltOffHand;
    scr_weapon_field_t DNIWeapon;
    // Has `getter`
    scr_weapon_field_t PickupSound;
    // Has `getter`
    scr_weapon_field_t PickupSoundPlayer;
    scr_weapon_field_t GadgetReadySound;
    scr_weapon_field_t GadgetReadySoundPlayer;
    scr_weapon_field_t BurnDuration;
    scr_weapon_field_t BurnDamageInterval;
    scr_weapon_field_t BurnDamage;
    scr_weapon_field_t ExplosionInnerDamage;
    scr_weapon_field_t ExplosionOuterDamage;
    // Entirely zeroed out in engine, explicitly
    scr_weapon_field_t __reserved_unused;
  };

  static inline constexpr size_t COUNT = 0xF1;
  scr_weapon_field_t fields[COUNT];

  // Does not include `__reserve_unused`, which is a name defined here only -
  // the `__reserved_unused` field has no `canonId` (0), and thus no name.
  static inline constexpr size_t DEFINED_COUNT = 0xF0;
  static inline constexpr std::array<const char *, DEFINED_COUNT> names = {
      "AIFuseTime",
      "ProjExplosionSound",
      "ProjExplosionSoundPlayer",
      "ProjSmokeStartSound",
      "ProjSmokeLoopSound",
      "ProjSmokeEndSound",
      "MeleeChargeRange",
      "MeleeLungeRange",
      "DoGibbing",
      "DoGibbingOnMelee",
      "DoAnnihilate",
      "DoBlowback",
      "MaxGibDistance",
      "LeftArc",
      "RightArc",
      "TopArc",
      "BottomArc",
      "ClipModel",
      "FightDist",
      "MaxDist",
      "SpinUpTime",
      "SpinDownTime",
      "FuelLife",
      "IsBoltAction",
      "IsDisallowAtMatchStart",
      "FireSound",
      "FireSoundPlayer",
      "BlocksProne",
      "IsClipOnly",
      "LockOnRadius",
      "LockOnLossRadius",
      "RequireLockOnToFire",
      "SetUsedStat",
      "MaxInstancesAllowed",
      "IsEMP",
      "IsFlash",
      "IsStun",
      "BulletImpactExplode",
      "DoEMPDestroyFx",
      "DoStun",
      "DoDamageFeedback",
      "DoHackedStats",
      "HackerTriggerOriginTag",
      "SpawnInfluencer",
      "AnyPlayerCanRetrieve",
      "IsTacticalInsertion",
      "IsValuable",
      "DestroyableByTrophySystem",
      "DrawOffHandModelInHand",
      "DisallowAtMatchStart",
      "NonStowedWeapon",
      "IsScavengable",
      "DoesFireDamage",
      "IgnoresFlakJacket",
      "NotKillstreak",
      "IsGameplayWeapon",
      "IsSupplyDropWeapon",
      "SkipBattleChatterKill",
      "SkipBattleChatterReload",
      "DoNotDamageOwner",
      "DestroysEquipment",
      "ForceDamageShellshockAndRumble",
      "IsAIKillstreakDamage",
      "IgnoreTeamKills",
      "TeamKillPenaltyScale",
      "VehicleProjectileDamageScalar",
      "VehicleProjectileSplashDamageScalar",
      "IsBallisticKnife",
      "IsPerkBottle",
      "SkipLowAmmoVox",
      "IsFlourishWeapon",
      "IsHybridWeapon",
      "DisableDeploy",
      "IsSniperWeapon",
      "IsHackToolWeapon",
      "MeleeIgnoresLightArmor",
      "IgnoresLightArmor",
      "IgnoresPowerArmor",
      "SoundRattleRangeMin",
      "SoundRattleRangeMax",
      "GrappleWeapon",
      "BurstCount",
      "WeaponHeadObjectiveHeight",
      "EnemyCrosshairRange",
      "UnlimitedAmmo",
      "IsNotDroppable",
      "DamageAlwaysKillsPlayer",
      "DamageToOwnerScalar",
      "ViewModels",
      "FrontendModel",
      "WorldModel",
      "WorldDamagedModel1",
      "WorldDamagedModel2",
      "WorldDamagedModel3",
      "StowedModel",
      "ShownEnemyEquip",
      "ShownEnemyExplo",
      "ShownRetrievable",
      "LockOnMinRange",
      "LockOnScreenRadius",
      "LockOnAngleHorizontal",
      "LockOnAngleVertical",
      "LockOnLossAngleHorizontal",
      "LockOnLossAngleVertical",
      "IsValid",
      "RootWeapon",
      "Attachments",
      "SupportedAttachments",
      "StartAmmo",
      "MaxAmmo",
      "GuidedMissileType",
      "LockOnType",
      "IsRocketLauncher",
      "LockOnSeekerSeArchSound",
      "LockOnSeekerSeArchSoundLoops",
      "LockOnSeekerLockedSound",
      "LockOnSeekerLockedSoundLoops",
      "LockOnTargetLockedSound",
      "LockOnTargetLockedSoundLoops",
      "LockOnTargetFiredOnSound",
      "LockOnTargetFiredOnSoundLoops",
      "ForceDamageHitLocation",
      "Type",
      "IsBulletWeapon",
      "IsGrenadeWeapon",
      "IsProjectileWeapon",
      "IsGasWeapon",
      "IsRiotShield",
      "WeapClass",
      "IsKillstreak",
      "IsCarriedKillstreak",
      "OffHandClass",
      "OffHandSlot",
      "IsLethalGrenade",
      "IsTacticalGrenade",
      "IsEquipment",
      "IsSpecificUse",
      "InventoryType",
      "IsPrimary",
      "IsItem",
      "IsAltMode",
      "ProjExplosionType",
      "IsGadget",
      "IsHeroWeapon",
      "Gadget_HeroVersion_2_0",
      "Gadget_BreadcrumbDuration",
      "Gadget_FlickerOnDamage",
      "Gadget_FlickerOnPowerLoss",
      "Gadget_FlickerOnPowerLow",
      "Gadget_Max_HitPoints",
      "Gadget_Power_Consume_On_Ammo_Use",
      "Gadget_PowerMoveLoss",
      "Gadget_PowerMoveSpeed",
      "Gadget_PowerGainScoreFactor",
      "Gadget_PowerGainScoreIgnoreSelf",
      "Gadget_PowerGainScoreIgnoreWhenActive",
      "Gadget_PowerOffLossOnDamage",
      "Gadget_PowerOnlossOnDamage",
      "Gadget_PowerReplenishFactor",
      "Gadget_Power_Reset_On_Spawn",
      "Gadget_Power_Reset_On_Class_Change",
      "Gadget_Power_Reset_On_Team_Change",
      "Gadget_Power_Reset_On_Round_Switch",
      "Gadget_Power_Round_End_Active_Penalty",
      "Gadget_Power_Usage_Rate",
      "Gadget_PowerTakedownGain",
      "Gadget_TakeDownRevealTime",
      "Gadget_Type",
      "Gadget_ShieldReflectPowerGain",
      "Gadget_ShieldReflectPowerLoss",
      "Gadget_Shockfield_Radius",
      "Gadget_Shockfield_Damage",
      "Gadget_Turnoff_OnEMPJammed",
      "Name",
      "DisplayName",
      "FireType",
      "IsFullAuto",
      "IsSemiAuto",
      "IsBurstFire",
      "IsStackedFire",
      "IsAllLockedFire",
      "IsChargeShot",
      "IsLauncher",
      "ClipSize",
      "ShotCount",
      "IsMeleeWeapon",
      "DeathCamTime",
      "FireTime",
      "ReloadTime",
      "MeleeTime",
      "MeleePowerTime",
      "MeleePowerTimeLeft",
      "MeleeChargeTime",
      "MeleeDamage",
      "AltWeapon",
      "StatIndex",
      "DualWieldWeapon",
      "IsDualWield",
      "FuseTime",
      "IsTimedDetonation",
      "AllowsDetonationDuringReload",
      "ProximityDetonation",
      "ProximityAlarmInnerRadius",
      "ProximityAlarmOuterRadius",
      "ProximityAlarmActivationDelay",
      "ChainEventRadius",
      "ChainEventTime",
      "ChainEventMax",
      "CookOffHoldTime",
      "MultiDetonation",
      "MultiDetonationFragmentSpeed",
      "ExplosionRadius",
      "ExplosionInnerRadius",
      "LockOnMaxRange",
      "LockOnMaxRangeNoLineOfSight",
      "LockOnSpeed",
      "AmmoCountEquipment",
      "Gadget_PowerSprintLoss",
      "Gadget_Pulse_Duration",
      "Gadget_Pulse_Margin",
      "Gadget_Pulse_Max_Range",
      "Gadget_PowerMax",
      "WeaponStartHitPoints",
      "WeaponDamage1HitPoints",
      "WeaponDamage2HitPoints",
      "WeaponDamage3HitPoints",
      "NoHitMarker",
      "SpecialPain",
      "Decoy",
      "AltOffHand",
      "DNIWeapon",
      "PickupSound",
      "PickupSoundPlayer",
      "GadgetReadySound",
      "GadgetReadySoundPlayer",
      "BurnDuration",
      "BurnDamageInterval",
      "BurnDamage",
      "ExplosionInnerDamage",
      "ExplosionOuterDamage",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(fields);
});
ASSERT_SIZE(FieldTable, sizeof(FieldTable::fields));
} // namespace weapon
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game