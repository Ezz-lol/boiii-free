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
    const scr_weapon_field_t AIFuseTime;
    const scr_weapon_field_t ProjExplosionSound;
    const scr_weapon_field_t ProjExplosionSoundPlayer;
    const scr_weapon_field_t ProjSmokeStartSound;
    const scr_weapon_field_t ProjSmokeLoopSound;
    const scr_weapon_field_t ProjSmokeEndSound;
    const scr_weapon_field_t MeleeChargeRange;
    const scr_weapon_field_t MeleeLungeRange;
    const scr_weapon_field_t DoGibbing;
    const scr_weapon_field_t DoGibbingOnMelee;
    const scr_weapon_field_t DoAnnihilate;
    const scr_weapon_field_t DoBlowback;
    const scr_weapon_field_t MaxGibDistance;
    const scr_weapon_field_t LeftArc;
    const scr_weapon_field_t RightArc;
    const scr_weapon_field_t TopArc;
    const scr_weapon_field_t BottomArc;
    const scr_weapon_field_t ClipModel;
    const scr_weapon_field_t FightDist;
    const scr_weapon_field_t MaxDist;
    const scr_weapon_field_t SpinUpTime;
    const scr_weapon_field_t SpinDownTime;
    const scr_weapon_field_t FuelLife;
    const scr_weapon_field_t IsBoltAction;
    const scr_weapon_field_t IsDisallowAtMatchStart;
    const scr_weapon_field_t FireSound;
    const scr_weapon_field_t FireSoundPlayer;
    const scr_weapon_field_t BlocksProne;
    const scr_weapon_field_t IsClipOnly;
    const scr_weapon_field_t LockOnRadius;
    const scr_weapon_field_t LockOnLossRadius;
    const scr_weapon_field_t RequireLockOnToFire;
    const scr_weapon_field_t SetUsedStat;
    const scr_weapon_field_t MaxInstancesAllowed;
    const scr_weapon_field_t IsEMP;
    const scr_weapon_field_t IsFlash;
    const scr_weapon_field_t IsStun;
    const scr_weapon_field_t BulletImpactExplode;
    const scr_weapon_field_t DoEMPDestroyFx;
    const scr_weapon_field_t DoStun;
    const scr_weapon_field_t DoDamageFeedback;
    const scr_weapon_field_t DoHackedStats;
    const scr_weapon_field_t HackerTriggerOriginTag;
    const scr_weapon_field_t SpawnInfluencer;
    const scr_weapon_field_t AnyPlayerCanRetrieve;
    const scr_weapon_field_t IsTacticalInsertion;
    const scr_weapon_field_t IsValuable;
    const scr_weapon_field_t DestroyableByTrophySystem;
    const scr_weapon_field_t DrawOffHandModelInHand;
    const scr_weapon_field_t DisallowAtMatchStart;
    const scr_weapon_field_t NonStowedWeapon;
    const scr_weapon_field_t IsScavengable;
    const scr_weapon_field_t DoesFireDamage;
    const scr_weapon_field_t IgnoresFlakJacket;
    const scr_weapon_field_t NotKillstreak;
    const scr_weapon_field_t IsGameplayWeapon;
    const scr_weapon_field_t IsSupplyDropWeapon;
    const scr_weapon_field_t SkipBattleChatterKill;
    const scr_weapon_field_t SkipBattleChatterReload;
    const scr_weapon_field_t DoNotDamageOwner;
    const scr_weapon_field_t DestroysEquipment;
    const scr_weapon_field_t ForceDamageShellshockAndRumble;
    const scr_weapon_field_t IsAIKillstreakDamage;
    const scr_weapon_field_t IgnoreTeamKills;
    const scr_weapon_field_t TeamKillPenaltyScale;
    const scr_weapon_field_t VehicleProjectileDamageScalar;
    const scr_weapon_field_t VehicleProjectileSplashDamageScalar;
    const scr_weapon_field_t IsBallisticKnife;
    const scr_weapon_field_t IsPerkBottle;
    const scr_weapon_field_t SkipLowAmmoVox;
    const scr_weapon_field_t IsFlourishWeapon;
    const scr_weapon_field_t IsHybridWeapon;
    const scr_weapon_field_t DisableDeploy;
    const scr_weapon_field_t IsSniperWeapon;
    const scr_weapon_field_t IsHackToolWeapon;
    const scr_weapon_field_t MeleeIgnoresLightArmor;
    const scr_weapon_field_t IgnoresLightArmor;
    const scr_weapon_field_t IgnoresPowerArmor;
    const scr_weapon_field_t SoundRattleRangeMin;
    const scr_weapon_field_t SoundRattleRangeMax;
    const scr_weapon_field_t GrappleWeapon;
    const scr_weapon_field_t BurstCount;
    const scr_weapon_field_t WeaponHeadObjectiveHeight;
    const scr_weapon_field_t EnemyCrosshairRange;
    const scr_weapon_field_t UnlimitedAmmo;
    const scr_weapon_field_t IsNotDroppable;
    const scr_weapon_field_t DamageAlwaysKillsPlayer;
    const scr_weapon_field_t DamageToOwnerScalar;
    const scr_weapon_field_t ViewModels;
    const scr_weapon_field_t FrontendModel;
    const scr_weapon_field_t WorldModel;
    const scr_weapon_field_t WorldDamagedModel1;
    const scr_weapon_field_t WorldDamagedModel2;
    const scr_weapon_field_t WorldDamagedModel3;
    const scr_weapon_field_t StowedModel;
    const scr_weapon_field_t ShownEnemyEquip;
    const scr_weapon_field_t ShownEnemyExplo;
    const scr_weapon_field_t ShownRetrievable;
    const scr_weapon_field_t LockOnMinRange;
    const scr_weapon_field_t LockOnScreenRadius;
    const scr_weapon_field_t LockOnAngleHorizontal;
    const scr_weapon_field_t LockOnAngleVertical;
    const scr_weapon_field_t LockOnLossAngleHorizontal;
    const scr_weapon_field_t LockOnLossAngleVertical;
    // Has `getter`
    const scr_weapon_field_t IsValid;
    // Has `getter`
    const scr_weapon_field_t RootWeapon;
    // Has `getter`
    const scr_weapon_field_t Attachments;
    // Has `getter`
    const scr_weapon_field_t SupportedAttachments;
    // Has `getter`
    const scr_weapon_field_t StartAmmo;
    // Has `getter`
    const scr_weapon_field_t MaxAmmo;
    // Has `getter`
    const scr_weapon_field_t GuidedMissileType;
    // Has `getter`
    const scr_weapon_field_t LockOnType;
    // Has `getter`
    const scr_weapon_field_t IsRocketLauncher;
    const scr_weapon_field_t LockOnSeekerSeArchSound;
    const scr_weapon_field_t LockOnSeekerSeArchSoundLoops;
    const scr_weapon_field_t LockOnSeekerLockedSound;
    const scr_weapon_field_t LockOnSeekerLockedSoundLoops;
    const scr_weapon_field_t LockOnTargetLockedSound;
    const scr_weapon_field_t LockOnTargetLockedSoundLoops;
    const scr_weapon_field_t LockOnTargetFiredOnSound;
    const scr_weapon_field_t LockOnTargetFiredOnSoundLoops;
    const scr_weapon_field_t ForceDamageHitLocation;
    // Has `getter`
    const scr_weapon_field_t Type;
    // Has `getter`
    const scr_weapon_field_t IsBulletWeapon;
    // Has `getter`
    const scr_weapon_field_t IsGrenadeWeapon;
    // Has `getter`
    const scr_weapon_field_t IsProjectileWeapon;
    // Has `getter`
    const scr_weapon_field_t IsGasWeapon;
    // Has `getter`
    const scr_weapon_field_t IsRiotShield;
    // Has `getter`
    const scr_weapon_field_t WeapClass;
    // Has `getter`
    const scr_weapon_field_t IsKillstreak;
    const scr_weapon_field_t IsCarriedKillstreak;
    // Has `getter`
    const scr_weapon_field_t OffHandClass;
    // Has `getter`
    const scr_weapon_field_t OffHandSlot;
    // Has `getter`
    const scr_weapon_field_t IsLethalGrenade;
    // Has `getter`
    const scr_weapon_field_t IsTacticalGrenade;
    // Has `getter`
    const scr_weapon_field_t IsEquipment;
    // Has `getter`
    const scr_weapon_field_t IsSpecificUse;
    // Has `getter`
    const scr_weapon_field_t InventoryType;
    // Has `getter`
    const scr_weapon_field_t IsPrimary;
    // Has `getter`
    const scr_weapon_field_t IsItem;
    // Has `getter`
    const scr_weapon_field_t IsAltMode;
    // Has `getter`
    const scr_weapon_field_t ProjExplosionType;
    // Has `getter`
    const scr_weapon_field_t IsGadget;
    // Has `getter`
    const scr_weapon_field_t IsHeroWeapon;
    const scr_weapon_field_t Gadget_HeroVersion_2_0;
    const scr_weapon_field_t Gadget_BreadcrumbDuration;
    const scr_weapon_field_t Gadget_FlickerOnDamage;
    const scr_weapon_field_t Gadget_FlickerOnPowerLoss;
    const scr_weapon_field_t Gadget_FlickerOnPowerLow;
    const scr_weapon_field_t Gadget_Max_HitPoints;
    const scr_weapon_field_t Gadget_Power_Consume_On_Ammo_Use;
    const scr_weapon_field_t Gadget_PowerMoveLoss;
    const scr_weapon_field_t Gadget_PowerMoveSpeed;
    const scr_weapon_field_t Gadget_PowerGainScoreFactor;
    const scr_weapon_field_t Gadget_PowerGainScoreIgnoreSelf;
    const scr_weapon_field_t Gadget_PowerGainScoreIgnoreWhenActive;
    const scr_weapon_field_t Gadget_PowerOffLossOnDamage;
    const scr_weapon_field_t Gadget_PowerOnlossOnDamage;
    const scr_weapon_field_t Gadget_PowerReplenishFactor;
    const scr_weapon_field_t Gadget_Power_Reset_On_Spawn;
    const scr_weapon_field_t Gadget_Power_Reset_On_Class_Change;
    const scr_weapon_field_t Gadget_Power_Reset_On_Team_Change;
    const scr_weapon_field_t Gadget_Power_Reset_On_Round_Switch;
    const scr_weapon_field_t Gadget_Power_Round_End_Active_Penalty;
    const scr_weapon_field_t Gadget_Power_Usage_Rate;
    const scr_weapon_field_t Gadget_PowerTakedownGain;
    const scr_weapon_field_t Gadget_TakeDownRevealTime;
    const scr_weapon_field_t Gadget_Type;
    const scr_weapon_field_t Gadget_ShieldReflectPowerGain;
    const scr_weapon_field_t Gadget_ShieldReflectPowerLoss;
    const scr_weapon_field_t Gadget_Shockfield_Radius;
    const scr_weapon_field_t Gadget_Shockfield_Damage;
    const scr_weapon_field_t Gadget_Turnoff_OnEMPJammed;
    // Has `getter`
    const scr_weapon_field_t Name;
    // Has `getter`
    const scr_weapon_field_t DisplayName;
    // Has `getter`
    const scr_weapon_field_t FireType;
    // Has `getter`
    const scr_weapon_field_t IsFullAuto;
    // Has `getter`
    const scr_weapon_field_t IsSemiAuto;
    // Has `getter`
    const scr_weapon_field_t IsBurstFire;
    // Has `getter`
    const scr_weapon_field_t IsStackedFire;
    // Has `getter`
    const scr_weapon_field_t IsAllLockedFire;
    // Has `getter`
    const scr_weapon_field_t IsChargeShot;
    // Has `getter`
    const scr_weapon_field_t IsLauncher;
    // Has `getter`
    const scr_weapon_field_t ClipSize;
    // Has `getter`
    const scr_weapon_field_t ShotCount;
    // Has `getter`
    const scr_weapon_field_t IsMeleeWeapon;
    // Has `getter`
    const scr_weapon_field_t DeathCamTime;
    // Has `getter`
    const scr_weapon_field_t FireTime;
    // Has `getter`
    const scr_weapon_field_t ReloadTime;
    // Has `getter`
    const scr_weapon_field_t MeleeTime;
    // Has `getter`
    const scr_weapon_field_t MeleePowerTime;
    // Has `getter`
    const scr_weapon_field_t MeleePowerTimeLeft;
    // Has `getter`
    const scr_weapon_field_t MeleeChargeTime;
    const scr_weapon_field_t MeleeDamage;
    // Has `getter`
    const scr_weapon_field_t AltWeapon;
    // Has `getter`
    const scr_weapon_field_t StatIndex;
    // Has `getter`
    const scr_weapon_field_t DualWieldWeapon;
    // Has `getter`
    const scr_weapon_field_t IsDualWield;
    // Has `getter`
    const scr_weapon_field_t FuseTime;
    // Has `getter`
    const scr_weapon_field_t IsTimedDetonation;
    // Has `getter`
    const scr_weapon_field_t AllowsDetonationDuringReload;
    // Has `getter`
    const scr_weapon_field_t ProximityDetonation;
    // Has `getter`
    const scr_weapon_field_t ProximityAlarmInnerRadius;
    // Has `getter`
    const scr_weapon_field_t ProximityAlarmOuterRadius;
    // Has `getter`
    const scr_weapon_field_t ProximityAlarmActivationDelay;
    // Has `getter`
    const scr_weapon_field_t ChainEventRadius;
    // Has `getter`
    const scr_weapon_field_t ChainEventTime;
    // Has `getter`
    const scr_weapon_field_t ChainEventMax;
    // Has `getter`
    const scr_weapon_field_t CookOffHoldTime;
    // Has `getter`
    const scr_weapon_field_t MultiDetonation;
    // Has `getter`
    const scr_weapon_field_t MultiDetonationFragmentSpeed;
    // Has `getter`
    const scr_weapon_field_t ExplosionRadius;
    // Has `getter`
    const scr_weapon_field_t ExplosionInnerRadius;
    // Has `getter`
    const scr_weapon_field_t LockOnMaxRange;
    // Has `getter`
    const scr_weapon_field_t LockOnMaxRangeNoLineOfSight;
    // Has `getter`
    const scr_weapon_field_t LockOnSpeed;
    // Has `getter`
    const scr_weapon_field_t AmmoCountEquipment;
    // Has `getter`
    const scr_weapon_field_t Gadget_PowerSprintLoss;
    // Has `getter`
    const scr_weapon_field_t Gadget_Pulse_Duration;
    // Has `getter`
    const scr_weapon_field_t Gadget_Pulse_Margin;
    // Has `getter`
    const scr_weapon_field_t Gadget_Pulse_Max_Range;
    // Has `getter`
    const scr_weapon_field_t Gadget_PowerMax;
    const scr_weapon_field_t WeaponStartHitPoints;
    const scr_weapon_field_t WeaponDamage1HitPoints;
    const scr_weapon_field_t WeaponDamage2HitPoints;
    const scr_weapon_field_t WeaponDamage3HitPoints;
    const scr_weapon_field_t NoHitMarker;
    const scr_weapon_field_t SpecialPain;
    const scr_weapon_field_t Decoy;
    const scr_weapon_field_t AltOffHand;
    const scr_weapon_field_t DNIWeapon;
    // Has `getter`
    const scr_weapon_field_t PickupSound;
    // Has `getter`
    const scr_weapon_field_t PickupSoundPlayer;
    const scr_weapon_field_t GadgetReadySound;
    const scr_weapon_field_t GadgetReadySoundPlayer;
    const scr_weapon_field_t BurnDuration;
    const scr_weapon_field_t BurnDamageInterval;
    const scr_weapon_field_t BurnDamage;
    const scr_weapon_field_t ExplosionInnerDamage;
    const scr_weapon_field_t ExplosionOuterDamage;
    // Entirely zeroed out in engine, explicitly
    const scr_weapon_field_t __reserved_unused;
  };

  static inline constexpr size_t COUNT = 0xF1;
  const scr_weapon_field_t fields[COUNT];

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