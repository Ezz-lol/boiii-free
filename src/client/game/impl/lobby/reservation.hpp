#pragma once

#include "game/game.hpp"

namespace game {
namespace lobby {
namespace reservation {
void ReservationRemove(reservation::Reservation &reservation, XUID xuid);
ReservationStatus *GetReservationStatus(Reservation &reservation, XUID xuid);
bool ReservationIsValid(Reservation &reservation, XUID xuid, uint64_t key,
                        uint64_t &outJoinNonce);
} // namespace reservation
} // namespace lobby
} // namespace game
