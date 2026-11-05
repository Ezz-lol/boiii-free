#include <std_include.hpp>

#include "reservation.hpp"

namespace game {
namespace lobby {
namespace reservation {
void ReservationRemove(reservation::Reservation &reservation, XUID xuid) {

  for (reservation::ReservationStatus &status : reservation.status) {
    if (status.xuid == xuid) {
      status.xuid = 0;
      status.lobbyID = 0;
      status.key = 0;
      status.probedXuid = 0;
      debug::LobbyDebug_ReservationCount(--reservation.count);
    }
  }
}

ReservationStatus *GetReservationStatus(Reservation &reservation, XUID xuid) {
  for (ReservationStatus &status : reservation.status) {
    if (status.xuid == xuid) {
      return &status;
    }
  }
  return nullptr;
}

bool ReservationIsValid(Reservation &reservation, XUID xuid, uint64_t key,
                        uint64_t &outJoinNonce) {
  for (ReservationStatus &status : reservation.status) {
    if (status.xuid == xuid && status.key == key) {
      outJoinNonce = status.clientJoinNonce;
      return true;
    }
  }
  return false;
}

} // namespace reservation
} // namespace lobby
} // namespace game
