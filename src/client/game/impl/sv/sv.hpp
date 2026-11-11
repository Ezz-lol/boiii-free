#pragma once

#include <cstdint>
namespace game {
namespace sv {
void SV_SetConfigString_Impl(const int32_t index, const char *val);

inline bool SV_IsSameMachine(const net::netadr_t &a, const net::netadr_t &b) {
  // Loopback ports are local client numbers, so comparing them would count each
  // split screen player as another machine
  if (a.type == net::netadrtype_t::LOOPBACK &&
      b.type == net::netadrtype_t::LOOPBACK) {
    return true;
  }
  return net::NetAdr_CompareIP(&a, &b);
}

int32_t SV_GetUniqueClientMachineCount_Impl();
} // namespace sv
} // namespace game
