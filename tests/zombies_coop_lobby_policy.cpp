#include "../src/client/component/zombies/lobby_policy.hpp"
#include <cassert>
#include <string>

using namespace zombies::coop;
int main() {
  lobby_policy_input input{};
  input.enabled = input.zombies = input.local_game_host = input.session_ready =
      true;
  input.custom = input.open = input.stock_content = input.live = true;
  input.lan_discoverable = input.browse_screen = true;
  input.capacity = 8;
  input.clients = 1;
  input.map = "zm_zod";
  auto result = lobby_listing_policy(input);
  assert(result.active && result.can_advertise && !result.advertise &&
         !result.lan_respond);
  input.advertise_opt_in = true;
  assert(lobby_listing_policy(input).advertise);
  input.browse_screen = false; // Solo/custom staging must not inherit saved public listing
  result = lobby_listing_policy(input);
  assert(result.active && !result.can_advertise && !result.advertise);
  input.browse_screen = true;
  input.open = false;
  assert(!lobby_listing_policy(input).active);
  input.open = true;
  input.live = false;
  input.lan = true;
  result = lobby_listing_policy(input);
  assert(result.active && result.lan_respond && !result.can_advertise &&
         !result.advertise);
  input.lan_discoverable = false;
  assert(!lobby_listing_policy(input).lan_respond);
  input.lan_discoverable = true;
  input.in_match = true;
  assert(!lobby_listing_policy(input).active);
  input.adapter_ready = true;
  input.browse_screen = false; // confirmed match may list after its menu has closed
  assert(lobby_listing_policy(input).active && lobby_listing_policy(input).lan_respond);
  for (int capacity = -1; capacity <= 19; ++capacity) {
    input.capacity = capacity;
    assert(lobby_listing_policy(input).active ==
           (capacity >= 5 && capacity <= 8));
  }
  input.capacity = 8;
  for (int clients = -1; clients <= 19; ++clients) {
    input.clients = clients;
    assert(lobby_listing_policy(input).active ==
           (clients >= 1 && clients <= 8));
  }
  input.clients = 1;
  for (auto field :
       {&lobby_policy_input::enabled, &lobby_policy_input::zombies,
        &lobby_policy_input::local_game_host,
        &lobby_policy_input::session_ready, &lobby_policy_input::custom,
        &lobby_policy_input::open, &lobby_policy_input::stock_content}) {
    input.*field = false;
    assert(!lobby_listing_policy(input).active);
    input.*field = true;
  }
  input.live = true;
  assert(!lobby_listing_policy(input).active);
  input.live = false;
  input.lan = false;
  assert(!lobby_listing_policy(input).active);
  input.lan = true;
  input.map = "zm_factory";
  assert(!lobby_listing_policy(input).active);
  input.map = "zm_tomb";
  assert(lobby_listing_policy(input).active);

  assert(private_lan_ipv4(0x0a000001) && private_lan_ipv4(0xac100001));
  assert(private_lan_ipv4(0xac1f0001) && private_lan_ipv4(0xc0a80101));
  assert(private_lan_ipv4(0x7f000001) && private_lan_ipv4(0xa9fe0001));
  for (const auto ip :
       {0u, 0xffffffffu, 0x08080808u, 0xac0f0001u, 0xac200001u, 0xc0a70101u})
    assert(!private_lan_ipv4(ip));
  const std::string nonce(32, 'a');
  assert(valid_lan_nonce(nonce) && valid_lan_nonce(std::string(64, 'F')));
  for (const auto size : {0u, 16u, 31u, 33u, 63u, 65u, 128u, 129u})
    assert(!valid_lan_nonce(std::string(size, 'a')));
  for (const char bad : {' ', '\0', '\n', 'g', '/', static_cast<char>(255)}) {
    auto invalid = nonce;
    invalid[0] = bad;
    assert(!valid_lan_nonce(invalid));
  }
  assert(accept_lan_reply(nonce, nonce, 1000, 2999, 127));
  assert(!accept_lan_reply(nonce, nonce, 1000, 3000, 0));
  assert(!accept_lan_reply(nonce, nonce, 1000, 999, 0));
  assert(!accept_lan_reply(nonce, nonce, 1000, 1000, 128));
  assert(!accept_lan_reply(nonce, std::string(32, 'b'), 1000, 1000, 0));
  lan_reply_limiter limiter;
  assert(limiter.allow(0x0a000001, 1000));
  assert(!limiter.allow(0x0a000001, 1999));
  assert(limiter.allow(0x0a000001, 2000));
  for (std::uint32_t i = 2; i <= 16; ++i)
    assert(limiter.allow(0x0a000000 + i, 2000));
  assert(!limiter.allow(0x0a000011, 2000));
  assert(limiter.allow(0x0a000011, 3000));
  assert(!limiter.allow(0x08080808, 4000));
}
