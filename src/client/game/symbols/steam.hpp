#pragma once

#include <game/symbols/sym_include.hpp>


namespace game {
namespace steam {

namespace sv {
WEAK symbol<SteamInterfaces> g_gameserverSteamInterfaces{0x0, 0x14837F7A0};
WEAK symbol<bool(SteamInterfaces *interfaces)> InitSteamGameServerInterfaces{
    0x0, 0x140475830};

} // namespace sv

namespace cl {
WEAK symbol<SteamInterfaces> g_steamClientInterfaces{0x150BBCB90, 0x14820D670};
WEAK symbol<bool(SteamInterfaces *interfaces)> InitSteamClientInterfaces{
    0x141D284D0, 0x1403D1880};
} // namespace cl

inline EngineDependent<steam::cl::SteamInterfaces *,
                       steam::sv::SteamInterfaces *>
PrimarySteamInterfaces() {
  if (is_server()) {
    return steam::sv::g_gameserverSteamInterfaces.get();
  }

  return steam::cl::g_steamClientInterfaces.get();
}

/*
  Real type of arg is a ptr to the corresponding SteamInterfaces struct
  for the given engine, but SteamInterfacesBase represents the shared base of
  each, so used as the argument type for convenience here.
*/
inline bool InitPrimarySteamInterfaces(SteamInterfacesBase *interfaces) {
  if (is_server()) {
    return sv::InitSteamGameServerInterfaces(
        reinterpret_cast<sv::SteamInterfaces *>(interfaces));
  }
  return cl::InitSteamClientInterfaces(
      reinterpret_cast<cl::SteamInterfaces *>(interfaces));
}

} // namespace steam
} // namespace game