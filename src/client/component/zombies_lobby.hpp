#pragma once

#include "network.hpp"
#include "server_list.hpp"
#include <string>

namespace zombies::coop {
struct lobby_advertisement {
  bool enabled{}, active{}, can_advertise{}, advertise{}, lan{}, in_match{};
  int capacity{}, clients{};
  std::string map, gametype;
};
// Main-thread engine state, published as a bounded copy for network callbacks.
lobby_advertisement get_lobby_advertisement();
// A two-second challenge scan. Each callback runs once on the main thread;
// a newer request cancels the prior request with success=false.
void request_lan_lobbies(server_list::callback callback);
} // namespace zombies::coop
