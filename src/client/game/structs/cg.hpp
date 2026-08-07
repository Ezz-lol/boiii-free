#pragma once

#include "core.hpp"
#include "weapon.hpp"
#include "phys/core.hpp"
#include "ik.hpp"

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

ASSERT_SIZE(clientUIActive_t, 0x1078);

struct ClientUIActives {
  clientUIActive_t actives[2];
};

struct ClientPlayerAttachmentInfo {
  const char *translatedDisplayName;
};

struct ClientPlayerAttachmentInfoPool {
  static constexpr size_t LEN = 64;
  ClientPlayerAttachmentInfo pool[LEN];

  static inline constexpr void assert_range(size_t index) {
    assert(index < LEN && "index to ClientPlayerAttachmentInfoPool must be "
                          "within range 0 <= index < 64");
  }

  template <IntegralLike<size_t> Index>
  inline constexpr const ClientPlayerAttachmentInfo &
  get(Index index_arg) const {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike<size_t> Index>
  inline constexpr const ClientPlayerAttachmentInfo &
  operator[](Index index) const {
    return get(index);
  }

  template <IntegralLike<size_t> Index>
  inline constexpr ClientPlayerAttachmentInfo &get(Index index_arg) {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike<size_t> Index>
  inline constexpr ClientPlayerAttachmentInfo &operator[](Index index) {
    return get(index);
  }
};

#pragma pack(push, 1)
struct ClientPlayerWeaponInfo {
  db::xasset::XModel *handModel;
  db::xasset::XModel *rocketModel;
  qboolean registered;
  uint8_t _padding14[4];
  weapon::Weapon weapon;
  const char *translatedDisplayName;
  const char *translatedModename;
  const char *translatedAIOverlayDescription;
};
#pragma pack(pop)

struct CGFakeEntitiesInuseBitArray {
  LocalClientPool<bitarray<768>> inUse;
};

struct LocalClientCgWeaponsPools {
  LocalClientPool<ClientPlayerWeaponInfo *> pools;
};

struct LocalClientCgDestructiblesPools {
  LocalClientPool<phys::Destructible *> pools;
};

struct LocalClientIkBufs {
  LocalClientPool<ik::IkBufRef> bufs;
};

struct LocalClientIkBufPool {
  LocalClientPool<ik::IkBuf> bufs;
};

} // namespace cg
} // namespace game