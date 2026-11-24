#include "../src/client/component/zombies/startup_gate.hpp"
#include <cassert>
#include <iostream>
#include <limits>
using zombies::coop::resolve_session_limit;
using zombies::coop::startup_gate;
int main() {
  static_assert(resolve_session_limit(8, 1) == 1);
  static_assert(resolve_session_limit(8, 4) == 4);
  static_assert(resolve_session_limit(8, 8) == 8);
  static_assert(resolve_session_limit(8, 18) == 0);
  static_assert(resolve_session_limit(4, 8) == 4);
  static_assert(resolve_session_limit(4, 18) == 4);
  // Native capacity is authoritative for stock Solo/Public/custom lobbies.
  // Opted-in expansion must match exactly; other mods retain their stock path.
  for (int configured = 1; configured <= 8; ++configured)
    for (int capacity = 1; capacity <= 18; ++capacity) {
      const int expected = capacity <= 4            ? capacity
                           : configured <= 4        ? configured
                           : configured == capacity ? configured
                                                    : 0;
      assert(resolve_session_limit(configured, capacity) == expected);
    }
  for (const int invalid : {std::numeric_limits<int>::min(), -1, 0, 9,
                            std::numeric_limits<int>::max()})
    for (const int capacity : {1, 4, 8, 18})
      assert(resolve_session_limit(invalid, capacity) == 0);
  for (int configured = 1; configured <= 8; ++configured)
    for (const int invalid : {std::numeric_limits<int>::min(), -1, 0})
      assert(resolve_session_limit(configured, invalid) == 0);
  startup_gate gate;
  assert(gate.ready());
  const int next_custom_limit = 8;
  for (int capacity = 1; capacity <= 4; ++capacity) {
    gate.arm(1, resolve_session_limit(next_custom_limit, capacity));
    assert(gate.ready()); // Choosing Solo/Public must not await the expanded
                          // adapter.
  }
  gate.arm(1, 8);
  assert(!gate.ready());
  assert(gate.observe(600000, false, "initializing", 0, false, 0) ==
         startup_gate::fault::none);
  assert(gate.observe(600100, true, "initializing", 0, false, 0) ==
         startup_gate::fault::none);
  // Old-map success must not open admission or consume cold-asset time.
  assert(gate.observe(600200, true, "experimental_callbacks_installed", 9, true,
                      7) == startup_gate::fault::none);
  assert(!gate.ready());
  assert(gate.observe(600300, true, "experimental_callbacks_installed", 1, true,
                      1) == startup_gate::fault::none);
  assert(gate.ready());
  assert(gate.observe(601300, true, "experimental_callbacks_installed", 1, true,
                      2) == startup_gate::fault::none);
  assert(gate.observe(666301, true, "experimental_callbacks_installed", 1, true,
                      2) == startup_gate::fault::none);
  assert(gate.ready()); // paused GSC time is legitimate
  assert(gate.observe(666302, true, "callback_ownership_lost", 0, false, 0) ==
         startup_gate::fault::adapter_declined);
  assert(!gate.ready());
  gate.arm(2, 8);
  assert(gate.observe(10, true, "custom_spawn_delay_preserved", 0, false, 0) ==
         startup_gate::fault::adapter_declined);
  gate.arm(3, 5);
  gate.observe(0, true, "initializing", 0, false, 0);
  assert(gate.observe(40001, true, "initializing", 0, false, 0) ==
         startup_gate::fault::timeout);
  gate.arm(4, 8);
  gate.observe(0, true, "experimental_callbacks_installed", 4, true, 1);
  assert(gate.observe(1, true, "experimental_callbacks_installed", 4, false,
                      2) == startup_gate::fault::wrong_map);
  gate.arm(5, 8);
  gate.observe(0, true, "experimental_callbacks_installed", 5, true, 1);
  assert(gate.observe(1, true, "experimental_callbacks_installed", 4, true,
                      2) == startup_gate::fault::wrong_epoch);
  gate.arm(6, 8);
  gate.observe(0, true, "experimental_callbacks_installed", 6, true, 3);
  assert(gate.observe(1, true, "experimental_callbacks_installed", 6, true,
                      0) == startup_gate::fault::heartbeat_lost);
  gate.reset();
  assert(gate.ready());
  std::cout << "PASS: session capacity resolution, startup epochs, cold loads, "
               "decline, deadline, heartbeat "
               "loss and stock reset\n";
}
