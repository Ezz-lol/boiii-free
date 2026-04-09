#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "game/game.hpp"

namespace live_presence_patch {
namespace {
constexpr int MAX_PARTY_MEMBERS = 18;

utils::hook::detour live_presence_party_hook;

// The game only has room for 18 players. Anything above that overflows
// a stack buffer and corrupts the return address, causing an instant crash.
void live_presence_party_stub(void *context_array, void *data_buffer) {
  if (context_array) {
    const auto ptr =
        *reinterpret_cast<int **>(static_cast<char *>(context_array) + 16);
    if (ptr) {
      const int party_count = (*ptr >> 2) & 0x1F;
      if (party_count > MAX_PARTY_MEMBERS) {
        // Zero the party count bits [6:2] to kill the overflow loop
        *ptr &= ~(0x1F << 2);
        printf("[Live][Presence] Blocked malicious party_count: %d (max %d)\n",
               party_count, MAX_PARTY_MEMBERS);
        return;
      }
    }
  }

  live_presence_party_hook.invoke<void>(context_array, data_buffer);
}
} // namespace

struct component final : client_component {
  void post_unpack() override {
    live_presence_party_hook.create(0x141E91820_g, live_presence_party_stub);
  }
};
} // namespace live_presence_patch

REGISTER_COMPONENT(live_presence_patch::component)
