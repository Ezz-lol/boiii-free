#pragma once

#include "core.hpp"
#include "weapon.hpp"

namespace game {
namespace cg {
#pragma pack(push, 1)
struct MigrationPers {
  int time;
  bool stanceHeld;
  uint8_t _padding05[3];
  StanceState stance;
  StanceState stancePosition;
  int stanceTime;
  uint8_t _padding14[4];
  weapon::Weapon cgameUserCmdWeapon;
  weapon::Weapon cgameUserCmdOffHandWeapon;
  weapon::Weapon weaponSelect;
  int weaponSelectTime;
  int weaponForcedSelectTime;
  weapon::Weapon weaponLatestPrimary[2];
  weapon::Weapon primaryWeaponForAlt[512];
  int holdBreathTime;
  int holdBreathInTime;
  int holdBreathDelay;
  float holdBreathFrac;
};
#pragma pack(pop)

enum class clientMigState_t : int32_t {
  CMSTATE_INACTIVE = 0x0,
  CMSTATE_OLDHOSTLEAVING = 0x1,
  CMSTATE_LIMBO = 0x2,
  CMSTATE_NEWHOSTCONNECT = 0x3,
  CMSTATE_COUNT = 0x4,
};

#pragma pack(push, 1)
struct clientUIActive_t {
  int flags;
  int keyCatchers;
  connstate_t connectionState;
  int nextScrollTime;
  clientMigState_t migrationState;
  uint8_t _padding14[4];
  MigrationPers migrationPers;
  int numVoicePacketsSent;
  int numVoicePacketsSentStart;
};
#pragma pack(pop)

static_assert(sizeof(clientUIActive_t) == 0x1078,
              "clientUIActive_t size must be 0x1078 bytes");
} // namespace cg
} // namespace game