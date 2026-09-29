#pragma once

#include <game/structs/scr/builtin/table/macros.hpp>

#include <game/structs/scr/field/clientscr.hpp>

#include <game/structs/scr/builtin/core.hpp>

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace field {
namespace clientscr {
using namespace game::scr::field;
using namespace game::scr::field::clientscr;

union FieldTable {
  struct {
    client_fields_t PlayerName;
    client_fields_t SessionTeam;
    client_fields_t Name;
    client_fields_t MaxHealth;
    client_fields_t WeaponHealth;
    client_fields_t HasSpyplane;
    client_fields_t HasSatellite;
    client_fields_t DisallowVehicleusage;
    client_fields_t Downs;
    client_fields_t Revives;
    client_fields_t Kills;
    client_fields_t Deaths;
    client_fields_t Assists;
    client_fields_t Defends;
    client_fields_t Plants;
    client_fields_t Defuses;
    client_fields_t Returns;
    client_fields_t Captures;
    client_fields_t ObjTime;
    client_fields_t Destructions;
    client_fields_t Disables;
    client_fields_t Escorts;
    client_fields_t Carries;
    client_fields_t Throws;
    client_fields_t Survived;
    client_fields_t Stabs;
    client_fields_t Tomahawks;
    client_fields_t Humiliated;
    client_fields_t X2Score;
    client_fields_t HeadShots;
    client_fields_t AGRKills;
    client_fields_t Hacks;
    client_fields_t PointsToWin;
    client_fields_t KillsConfirmed;
    client_fields_t KillsDenied;
    client_fields_t ShotsMissed;
    client_fields_t ShotsHit;
    client_fields_t Victory;
    client_fields_t SbTimePlayed;
    client_fields_t Incaps;
    client_fields_t Gems;
    client_fields_t Skulls;
    client_fields_t Chickens;
    client_fields_t KillCamEntity;
    client_fields_t KillCamTargetEntity;
    client_fields_t KillCamWeapon;
    client_fields_t KillCamMod;
    client_fields_t SpectateKillCam;
    client_fields_t Score;
    client_fields_t SessionState;
    client_fields_t StatusIcon;
    client_fields_t SpectatorClient;
    client_fields_t CurrentSpectatingClient;
    client_fields_t ArchiveTime;
    client_fields_t PSOffsetTime;
    client_fields_t Pers;
    client_fields_t UsingVehicle;
    client_fields_t VehiclePosition;
    client_fields_t HeadIcon;
    client_fields_t Momentum;
    client_fields_t DiveToProne;
    client_fields_t Sprinting;
    client_fields_t AnimViewUnlock;
    client_fields_t AnimInputUnlock;
    client_fields_t AnimNoClientTransform;
    client_fields_t TopDowncamera;
    client_fields_t GroundEntity;
    client_fields_t ViewLockedEntity;
    client_fields_t CursorHintEnt;
    client_fields_t UseHoldEnt;
    client_fields_t GroundSurfaceType;
    client_fields_t LookAtEnt;
    client_fields_t ChargeShotlevel;
    client_fields_t LockOnEntity;
    client_fields_t PivotEntity;
    client_fields_t LastDamageTime;
    client_fields_t CleanDeposits;
    client_fields_t CleanDenies;
    client_fields_t Infects;

    // Entirely zeroed out in engine, explicitly
    client_fields_t __reserved_unused;
  };

  static inline constexpr size_t COUNT = 0x50;
  client_fields_t fields[COUNT];

  static inline constexpr size_t DEFINED_COUNT = COUNT - 1;

  static const std::array<std::string_view, DEFINED_COUNT> names;

  DECLARE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(fields);
};
ASSERT_SIZE(FieldTable, sizeof(FieldTable::fields));
} // namespace clientscr
} // namespace field
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game
