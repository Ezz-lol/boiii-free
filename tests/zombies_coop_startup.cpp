#include "../src/client/component/zombies/startup_gate.hpp"
#include <cassert>
#include <iostream>
using zombies::coop::startup_gate;
int main() {
  startup_gate gate;
  assert(gate.ready());
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
  std::cout << "PASS: startup epochs, cold loads, decline, deadline, heartbeat "
               "loss and stock reset\n";
}
