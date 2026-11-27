// Pure lifecycle/admission policy tests. No game or retail bytes.
#ifdef BOIII_ENDING_PRIVATE_TEST
#include "ending_policy.hpp"
#include "startup_gate.hpp"
#include "spawn_policy.hpp"
#else
#include "../src/client/component/zombies/ending_policy.hpp"
#include "../src/client/component/zombies/startup_gate.hpp"
#include "../src/client/component/bots/spawn_policy.hpp"
#endif
#include <cassert>
#include <iostream>
#include <thread>
using namespace zombies::coop;
static unsigned checks{};
static void check(bool ok) {
  ++checks;
  assert(ok);
}
int main() {
  using ending_policy::admission_lifecycle;
  using fault = startup_gate::fault;
  for (bool zm : {false, true})
    for (int capacity = 0; capacity <= 18; ++capacity)
      for (int epoch : {-1, 0, 1, 2}) {
        check(ending_policy::matches(zm, capacity, epoch, "zm_zod", "zm_zod",
                                     "ending", 1, epoch, "zm_zod") ==
              (zm && capacity >= 5 && capacity <= 8 && epoch > 0));
      }
  for (auto bad :
       {"", "core_frontend", "bad map", "bad/map", "bad\\map", "bad\"map"})
    check(!ending_policy::matches(true, 8, 1, bad, bad, "ending", 1, 1, bad));
  for (int rev : {-1, 0, 2, 4})
    check(!ending_policy::matches(true, 8, 1, "zm_zod", "zm_zod", "ending", rev,
                                  1, "zm_zod"));
  check(!ending_policy::matches(true, 8, 1, "zm_zod", "zm_tomb", "ending", 1, 1,
                                "zm_zod"));
  check(!ending_policy::matches(true, 8, 1, "zm_zod", "zm_zod", "ending", 1, 2,
                                "zm_zod"));
  check(!ending_policy::matches(true, 8, 1, "zm_zod", "zm_zod", "ending", 1, 1,
                                "zm_tomb"));
  for (auto status : {"initializing", "experimental_callbacks_installed",
                      "callback_ownership_lost"})
    check(!ending_policy::matches(true, 8, 1, "zm_zod", "zm_zod", status, 1, 1,
                                  "zm_zod"));
  admission_lifecycle life;
  for (int epoch = 1; epoch <= 1000; ++epoch) {
    life.arm(epoch);
    check(life.epoch() == epoch && !life.ending());
    check(!life.finish(epoch - 1) && !life.finish(epoch + 1) && !life.ending());
    check(life.finish(epoch) && life.ending());
    check(life.finish(epoch) && life.ending());
    life.arm(epoch + 1);
    check(!life.finish(epoch) && !life.ending() && life.epoch() == epoch + 1);
    life.reset();
    check(!life.finish(epoch) && life.epoch() == 0 && !life.ending());
  }
  for (int epoch = 1; epoch <= 64; ++epoch) {
    life.arm(epoch);
    std::thread stale([&] {
      for (int i = 0; i < 64; ++i)
        (void)life.finish(epoch);
    });
    life.arm(epoch + 1);
    stale.join();
    check(life.epoch() == epoch + 1 && !life.ending());
  }
  for (int limit = 1; limit <= 8; ++limit) {
    startup_gate gate;
    gate.arm(10, limit);
    check(!gate.close_for_ending(0, true, 1));
    check(!gate.close_for_ending(9, true, 1));
    check(!gate.close_for_ending(10, false, 1));
    check(!gate.close_for_ending(10, true, 0));
    check(!gate.close_for_ending(10, true, 2));
    if (limit <= 4) {
      check(!gate.close_for_ending(10, true, 1) && gate.ready());
      continue;
    }
    check(gate.close_for_ending(10, true, 1) && gate.ending() && !gate.ready());
    for (auto status :
         {"ending", "initializing", "experimental_callbacks_installed"})
      check(gate.observe(999999, false, status, 10, true, 0, false) ==
                fault::none &&
            !gate.ready());
    check(gate.close_for_ending(10, true, 1));
    gate.arm(11, limit);
    check(!gate.ending() && !gate.ready());
    check(!gate.close_for_ending(10, true, 1));
    check(gate.observe(1, true, "experimental_callbacks_installed", 11, true,
                       1) == fault::none &&
          gate.ready());
    check(gate.close_for_ending(11, true, 1) && !gate.ready());
    gate.reset();
    check(!gate.ending() && gate.ready());
  }
  startup_gate zero;
  zero.arm(0, 8);
  check(!zero.close_for_ending(0, true, 1));
  startup_gate declined;
  declined.arm(1, 8);
  check(declined.observe(0, true, "callback_ownership_lost", 1, true, 1) ==
        fault::adapter_declined);
  check(!declined.close_for_ending(1, true, 1));
  bots::policy::session_snapshot request;
  request.epoch = 42;
  request.mode = 2;
  request.capacity = 8;
  request.running = true;
  request.zombies = true;
  request.schema_ready = true;
  request.schema_capacity = 8;
  request.map = "zm_zod";
  life.arm(42);
  int added{};
  auto read = [&]() -> std::optional<bots::policy::session_snapshot> {
    auto now = request;
    now.occupied = added;
    now.schema_ready = !life.ending();
    return now;
  };
  auto add = [&] {
    ++added;
    if (added == 2)
      check(life.finish(42));
    return true;
  };
  check(bots::policy::spawn_batch(request, 8, read, add) == 2 && added == 2);
  life.arm(43);
  request.epoch = 43;
  check(!life.finish(42));
  added = 0;
  auto plain = [&] {
    ++added;
    return true;
  };
  check(bots::policy::spawn_batch(request, 8, read, plain) == 8);
  check(life.finish(43));
  added = 0;
  check(bots::policy::spawn_batch(request, 8, read, plain) == 0);
  request.zombies = false;
  request.capacity = 18;
  request.schema_ready = false;
  added = 0;
  check(bots::policy::spawn_batch(request, 8, read, plain) == 8);
  request.zombies = true;
  request.capacity = 4;
  added = 0;
  check(bots::policy::spawn_batch(request, 4, read, plain) == 4);
  std::cout << "checks=" << checks
            << " queued_allocations_stopped=true native_engine=false\n";
}
