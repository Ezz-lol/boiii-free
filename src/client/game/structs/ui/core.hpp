#pragma once

#include <cstdint>

namespace game::ui {
typedef uint16_t UIModelIndex;

struct CurrentWeaponModel {
  ui::UIModelIndex currentWeaponModel;
  ui::UIModelIndex equippedWeaponReferenceModel;
  ui::UIModelIndex currentWeaponReticleWidget;
  ui::UIModelIndex currentWeaponLockOnWidget;
  ui::UIModelIndex lookingDownSights;
  ui::UIModelIndex sightIsMagnified;
  ui::UIModelIndex lockedOnEnemy;
  ui::UIModelIndex lockedOnEnemyTeamTargetting;
  ui::UIModelIndex lockedOnEnemyTeamHacking;
  ui::UIModelIndex lockedOnEnemyTimeRemaining;
  ui::UIModelIndex lockedOnEnemyMissilesRemaining;
  ui::UIModelIndex currentShotCharge;
  ui::UIModelIndex weaponDelayFiring;
  ui::UIModelIndex weaponOverEnemy;
  ui::UIModelIndex usingOffhand;
  ui::UIModelIndex weaponPrestigeUI3DTextModel;
  ui::UIModelIndex updateWeaponSelectModel;
  ui::UIModelIndex weaponNameModel;
  ui::UIModelIndex viewmodelWeaponNameModel;
  ui::UIModelIndex attachmentNameModel[8];
  ui::UIModelIndex fireRateStringModel;
  ui::UIModelIndex weaponModel;
  ui::UIModelIndex clipMaxAmmoModel;
  ui::UIModelIndex ammoInClipModel;
  ui::UIModelIndex ammoInDWClipModel;
  ui::UIModelIndex ammoStockModel;
  ui::UIModelIndex ammoClipPercentageModel;
  ui::UIModelIndex ammoLowModel;
  ui::UIModelIndex ammoDWLowModel;
  ui::UIModelIndex totalAmmoLowModel;
  ui::UIModelIndex damageFeedbackModel;
  ui::UIModelIndex damageFeedbackFlagModel;
  ui::UIModelIndex spikeLauncherCounterReadyCountModel;
  ui::UIModelIndex spikeLauncherCounterBlastingModel;
  ui::UIModelIndex lmgInfiniteStateModel;
};
struct VehicleModel {
  ui::UIModelIndex vehicleTypeModel;
  ui::UIModelIndex isVehicleOwnerModel;
  ui::UIModelIndex outOfRangeModel;
  ui::UIModelIndex zoomedModel;
  ui::UIModelIndex ammoModel;
  ui::UIModelIndex ammoReadyModel;
  ui::UIModelIndex speedModel;
  ui::UIModelIndex remainingTimeModel;
  ui::UIModelIndex damageModel;
  ui::UIModelIndex engineRpmModel;
  ui::UIModelIndex weaponRevModel;
  ui::UIModelIndex pitchModel;
  ui::UIModelIndex yawModel;
  ui::UIModelIndex rollModel;
  ui::UIModelIndex altitudeModel;
  ui::UIModelIndex partDestroyedModel[2];
  ui::UIModelIndex missileLockedOnModel[5];
  ui::UIModelIndex fullscreenFilterModel;
  ui::UIModelIndex weaponHeatModel;
  ui::UIModelIndex weaponOverheatingModel;
};
struct KillstreakRewardModel {
  ui::UIModelIndex nameModel;
  ui::UIModelIndex imageModel;
  ui::UIModelIndex slotModel;
  ui::UIModelIndex momentumCostModel;
  ui::UIModelIndex ammoModel;
};
struct PlayerGadgetModel {
  ui::UIModelIndex idModel;
  ui::UIModelIndex ammoModel;
  ui::UIModelIndex powerRatioModel;
  ui::UIModelIndex stateModel;
};
struct PlayerRoleModel {
  ui::UIModelIndex clientTeam;
  ui::UIModelIndex clientCharacter;
  ui::UIModelIndex clientHealth;
  ui::UIModelIndex clientArmor;
  ui::UIModelIndex clientArmorMax;
  ui::UIModelIndex clientArmorTier;
  ui::UIModelIndex clientArmorPct;
  ui::UIModelIndex clientLpLevel;
  ui::UIModelIndex clientPlayerName;
  ui::UIModelIndex clientClanTag;
};

struct PerkModel {
  ui::UIModelIndex nameModel;
  ui::UIModelIndex imageModel;
};
} // namespace game::ui