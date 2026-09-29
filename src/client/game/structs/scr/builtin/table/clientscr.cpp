#include <std_include.hpp>

#include "clientscr.hpp"

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace field {
namespace clientscr {
constexpr std::array<std::string_view, FieldTable::DEFINED_COUNT>
    FieldTable::names = {"PlayerName",
                         "SessionTeam",
                         "Name",
                         "MaxHealth",
                         "WeaponHealth",
                         "HasSpyplane",
                         "HasSatellite",
                         "DisallowVehicleusage",
                         "Downs",
                         "Revives",
                         "Kills",
                         "Deaths",
                         "Assists",
                         "Defends",
                         "Plants",
                         "Defuses",
                         "Returns",
                         "Captures",
                         "ObjTime",
                         "Destructions",
                         "Disables",
                         "Escorts",
                         "Carries",
                         "Throws",
                         "Survived",
                         "Stabs",
                         "Tomahawks",
                         "Humiliated",
                         "X2Score",
                         "HeadShots",
                         "AGRKills",
                         "Hacks",
                         "PointsToWin",
                         "KillsConfirmed",
                         "KillsDenied",
                         "ShotsMissed",
                         "ShotsHit",
                         "Victory",
                         "SbTimePlayed",
                         "Incaps",
                         "Gems",
                         "Skulls",
                         "Chickens",
                         "KillCamEntity",
                         "KillCamTargetEntity",
                         "KillCamWeapon",
                         "KillCamMod",
                         "SpectateKillCam",
                         "Score",
                         "SessionState",
                         "StatusIcon",
                         "SpectatorClient",
                         "CurrentSpectatingClient",
                         "ArchiveTime",
                         "PSOffsetTime",
                         "Pers",
                         "UsingVehicle",
                         "VehiclePosition",
                         "HeadIcon",
                         "Momentum",
                         "DiveToProne",
                         "Sprinting",
                         "AnimViewUnlock",
                         "AnimInputUnlock",
                         "AnimNoClientTransform",
                         "TopDowncamera",
                         "GroundEntity",
                         "ViewLockedEntity",
                         "CursorHintEnt",
                         "UseHoldEnt",
                         "GroundSurfaceType",
                         "LookAtEnt",
                         "ChargeShotlevel",
                         "LockOnEntity",
                         "PivotEntity",
                         "LastDamageTime",
                         "CleanDeposits",
                         "CleanDenies",
                         "Infects"

};
DEFINE_NAME_MAP(FieldTable::names, FieldTable::hashes);
} // namespace clientscr
} // namespace field
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game
