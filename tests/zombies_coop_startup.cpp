#include "../src/client/component/zombies/startup_gate.hpp"
#include <cassert>
#include <iostream>
#include <limits>
using zombies::coop::resolve_session_limit;
using zombies::coop::startup_gate;
int main()
{
    static_assert(resolve_session_limit(8, 1, true) == 1);
    static_assert(resolve_session_limit(8, 4, false) == 4);
    static_assert(resolve_session_limit(4, 8, true) == 8); // joined/inherited eight-slot lobby
    static_assert(resolve_session_limit(8, 5, true) == 5); // saved next-lobby preference is not this lobby
    static_assert(resolve_session_limit(8, 18, true) == 0);
    static_assert(resolve_session_limit(4, 8, false) == 4);
    static_assert(resolve_session_limit(4, 18, false) == 4); // foreign mod retains the old non-opted-in path
    static_assert(resolve_session_limit(8, 8, false) == 0); // contradictory expanded preference without opt-in
    // Every valid saved preference/native allocation pair under both flag states.
    // Expected results are grouped by native allocation, independently of host preference.
    for (const bool expanded_enabled : {false, true})
    {
        for (int configured = 1; configured <= 8; ++configured)
        {
            for (int capacity = 1; capacity <= 4; ++capacity)
                assert(resolve_session_limit(configured, capacity, expanded_enabled) == capacity);
            for (int capacity = 5; capacity <= 8; ++capacity)
                assert(resolve_session_limit(configured, capacity, expanded_enabled) ==
                    (expanded_enabled ? capacity : configured <= 4 ? configured : 0));
            for (int capacity = 9; capacity <= 18; ++capacity)
                assert(resolve_session_limit(configured, capacity, expanded_enabled) ==
                    (!expanded_enabled && configured <= 4 ? configured : 0));
        }
        for (const int invalid : {std::numeric_limits<int>::min(), -1, 0, 9, std::numeric_limits<int>::max()})
            for (const int capacity : {1, 4, 8, 18})
                assert(resolve_session_limit(invalid, capacity, expanded_enabled) == 0);
        for (int configured = 1; configured <= 8; ++configured)
            for (const int invalid : {std::numeric_limits<int>::min(), -1, 0, 19, std::numeric_limits<int>::max()})
                assert(resolve_session_limit(configured, invalid, expanded_enabled) == 0);
    }
    startup_gate gate;
    assert(gate.ready());
    const int next_custom_limit = 8;
    for (int capacity = 1; capacity <= 4; ++capacity)
    {
        gate.arm(1, resolve_session_limit(next_custom_limit, capacity, true));
        assert(gate.ready()); // Choosing Solo/Public must not await the expanded adapter.
    }
    gate.arm(1, 8);
    assert(!gate.ready());
    assert(gate.observe(600000, false, "initializing", 0, false, 0) == startup_gate::fault::none);
    assert(gate.observe(600100, true, "initializing", 0, false, 0) == startup_gate::fault::none);
    // Old-map success must not open admission or consume cold-asset time.
    assert(gate.observe(600200, true, "experimental_callbacks_installed", 9, true, 7) == startup_gate::fault::none);
    assert(!gate.ready());
    assert(gate.observe(600300, true, "experimental_callbacks_installed", 1, true, 1) == startup_gate::fault::none);
    assert(gate.ready());
    assert(gate.observe(601300, true, "experimental_callbacks_installed", 1, true, 2) == startup_gate::fault::none);
    assert(gate.observe(666301, true, "experimental_callbacks_installed", 1, true, 2) == startup_gate::fault::none);
    assert(gate.ready()); // paused GSC time is legitimate
    assert(gate.observe(666302, true, "callback_ownership_lost", 0, false, 0) == startup_gate::fault::adapter_declined);
    assert(!gate.ready());
    gate.arm(2, 8);
    assert(gate.observe(10, true, "custom_spawn_delay_preserved", 0, false, 0) == startup_gate::fault::adapter_declined);
    gate.arm(3, 5);
    gate.observe(0, true, "initializing", 0, false, 0);
    assert(gate.observe(40001, true, "initializing", 0, false, 0) == startup_gate::fault::timeout);
    gate.arm(4, 8);
    gate.observe(0, true, "experimental_callbacks_installed", 4, true, 1);
    assert(gate.observe(1, true, "experimental_callbacks_installed", 4, false, 2) == startup_gate::fault::wrong_map);
    gate.arm(5, 8);
    gate.observe(0, true, "experimental_callbacks_installed", 5, true, 1);
    assert(gate.observe(1, true, "experimental_callbacks_installed", 4, true, 2) == startup_gate::fault::wrong_epoch);
    gate.arm(6, 8);
    gate.observe(0, true, "experimental_callbacks_installed", 6, true, 3);
    assert(gate.observe(1, true, "experimental_callbacks_installed", 6, true, 0) == startup_gate::fault::heartbeat_lost);
    // Cold startup has no running process yet and must not spend its timeout.
    gate.arm(20, 8);
    for (const int time : {0, 600000, 1200000})
    {
        assert(gate.observe(time, false, "experimental_callbacks_installed", 19, true, 50) == startup_gate::fault::none);
        assert(!gate.ready());
    }
    assert(gate.observe(1200100, true, "initializing", 0, false, 0) == startup_gate::fault::none);
    // Stopping after the first running observation requires a new arm even if
    // scripts had not finished initialization. Reusing old dvars cannot reopen it.
    assert(gate.observe(1200200, false, "initializing", 0, false, 0) == startup_gate::fault::stopped);
    for (const int stale_epoch : {19, 20, 21})
    {
        assert(gate.observe(1200300, true, "experimental_callbacks_installed", stale_epoch, true, 100) == startup_gate::fault::stopped);
        assert(!gate.ready());
    }
    gate.arm(21, 8);
    assert(gate.observe(1200400, false, "experimental_callbacks_installed", 20, true, 100) == startup_gate::fault::none);
    assert(gate.observe(1200500, true, "experimental_callbacks_installed", 20, true, 100) == startup_gate::fault::none);
    assert(!gate.ready());
    assert(gate.observe(1200600, true, "experimental_callbacks_installed", 21, true, 0) == startup_gate::fault::none);
    assert(gate.ready());
    // The same latch protects a previously ready map and stays closed through
    // repeated false/true observations until an explicit fresh map arm.
    assert(gate.observe(1200700, false, "experimental_callbacks_installed", 21, true, 1) == startup_gate::fault::stopped);
    assert(gate.observe(1200800, false, "initializing", 0, false, 0) == startup_gate::fault::stopped);
    assert(gate.observe(1200900, true, "experimental_callbacks_installed", 21, true, 2) == startup_gate::fault::stopped);
    assert(!gate.ready());
    gate.arm(22, 5);
    assert(gate.observe(1201000, true, "experimental_callbacks_installed", 22, true, 0) == startup_gate::fault::none);
    assert(gate.ready());
    // Controller receipts alone must not admit a mismatched/uninitialized schema.
    gate.arm(23, 8);
    assert(gate.observe(0, true, "experimental_callbacks_installed", 23, true, 1, false) == startup_gate::fault::none);
    assert(!gate.ready());
    assert(gate.observe(1, true, "experimental_callbacks_installed", 23, true, 1, true) == startup_gate::fault::none);
    assert(gate.ready());
    assert(gate.observe(2, true, "experimental_callbacks_installed", 23, true, 1, false) == startup_gate::fault::schema_lost);
    assert(!gate.ready());
    gate.reset();
    assert(gate.ready());
    assert(gate.observe(1201100, false, "disabled", 0, false, 0) == startup_gate::fault::none);
    assert(gate.ready()); // normal stock shutdown never waits for expanded readiness
    std::cout << "PASS: opt-in native capacity matrix, startup epochs, cold loads, stopped-map rearm, decline, deadline, heartbeat loss and stock reset\n";
}
