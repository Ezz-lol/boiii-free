#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace steam {

namespace sv {
WEAK symbol<SteamInterfaces> g_gameserverSteamInterfaces{
    0x1513AF1E0, 0x15142E150, 0x14837F7A0};
WEAK symbol<bool(SteamInterfaces *interfaces)> InitSteamGameServerInterfaces{
    0x0, 0x0, 0x140475830};

} // namespace sv

namespace cl {
WEAK symbol<SteamInterfaces> g_steamClientInterfaces{0x150B3DC10, 0x150BBCB90,
                                                     0x14820D670};
WEAK symbol<bool(SteamInterfaces *interfaces)> InitSteamClientInterfaces{
    0x1403D1880, 0x141D284D0, 0x1403D1880};
} // namespace cl

inline SteamInterfaces PrimarySteamInterfaces() {
  if (is_server()) {
    return SteamInterfaces::from(steam::sv::g_gameserverSteamInterfaces.get());
  }

  return SteamInterfaces::from(steam::cl::g_steamClientInterfaces.get());
}

/*
  Real type of arg is a ptr to the corresponding SteamInterfaces struct
  for the given engine, but SteamInterfacesBase represents the shared base of
  each, so used as the argument type for convenience here.
*/
inline bool InitPrimarySteamInterfaces(SteamInterfaces interfaces) {
  if (is_server()) {
    return sv::InitSteamGameServerInterfaces(interfaces.sv);
  }
  return cl::InitSteamClientInterfaces(interfaces.cl);
}

} // namespace steam
} // namespace game
