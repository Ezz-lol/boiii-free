#pragma once

#include <cstdint>

#include <game/structs/core.hpp>
#include <game/structs/lobby/core.hpp>

namespace game {
namespace lobby {
namespace reservation {

// Verified
PACKED(struct ReservationStatus {
  XUID xuid;
  LobbyID lobbyID;
  uint64_t key;
  uint64_t clientJoinNonce;
  XUID probedXuid;
  int32_t timeout;
  uint8_t _padding0x2C[4];
});
ASSERT_SIZE(ReservationStatus, 0x30);

// Verified
PACKED(struct Reservation {
  LobbyClientPool<ReservationStatus> status;
  int32_t count;
  uint8_t _padding364[4];
  uint64_t key;
});
ASSERT_SIZE(Reservation, 0x370);
} // namespace reservation
} // namespace lobby
} // namespace game
