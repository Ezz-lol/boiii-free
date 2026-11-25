#include "../src/client/component/zombies/join_network_policy.hpp"
#include <cassert>
#include <iostream>

// Distinct values ensure the policy uses engine enum names, not assumed ABI
// constants. Production instantiates it with the actual Lobby/Join enums.
enum class Mode { UNKNOWN = 19, LAN = 23, LIVE = 41 };
enum class Join { NORMAL, PLAYLIST, FRIEND, INVITE, PARTY, GROUPS };

int main() {
  using namespace zombies::coop;
  assert(join_network_mode(true, true, Mode::LAN) == Mode::LAN);
  assert(join_network_mode(true, true, Mode::LIVE) == Mode::LIVE);
  assert(join_network_mode(true, true, Mode::UNKNOWN) == Mode::UNKNOWN);
  assert(join_network_mode(false, true, Mode::LAN) == Mode::LAN);
  assert(join_network_mode(false, false, Mode::UNKNOWN) == Mode::UNKNOWN);
  assert(join_network_mode(true, false, Mode::LAN) == Mode::LIVE);

  assert(network_join_allowed(false, false, Mode::LAN));
  assert(network_join_allowed(false, false, Mode::LIVE));
  assert(!network_join_allowed(false, false, Mode::UNKNOWN));
  assert(network_join_allowed(true, false, Mode::UNKNOWN));
  assert(network_join_allowed(false, true, Mode::UNKNOWN));

  for (const auto kind : {Join::NORMAL, Join::PLAYLIST, Join::FRIEND,
                          Join::INVITE, Join::PARTY, Join::GROUPS}) {
    const bool native_lan_kind = kind == Join::PARTY || kind == Join::PLAYLIST;
    assert(lan_join_request_allowed(Mode::LAN, kind) == native_lan_kind);
    assert(!lan_join_request_allowed(Mode::LIVE, kind));
    assert(!lan_join_request_allowed(Mode::UNKNOWN, kind));
  }
  std::cout << "PASS: native Zombies network modes, LAN party/playlist joins and online isolation\n";
}
