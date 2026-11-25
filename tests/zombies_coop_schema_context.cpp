#include "../src/client/component/zombies/schema_context_policy.hpp"
#include <cassert>

int main() {
  using namespace zombies::coop;
  static_assert(peer_schema_prepare(7, 7, true, false, true) == peer_prepare_action::duplicate);
  static_assert(peer_schema_prepare(7, 8, true, false, true) == peer_prepare_action::require_frontend);
  static_assert(peer_schema_prepare(7, 7, false, false, true) == peer_prepare_action::require_frontend);
  static_assert(peer_schema_prepare(7, 8, true, true, true) == peer_prepare_action::fresh);
  static_assert(peer_schema_prepare(0, 1, false, false, false) == peer_prepare_action::fresh);
  static_assert(peer_schema_prepare(0, 0, false, true, false) == peer_prepare_action::invalid);
  static_assert(peer_schema_prepare(7, 7, false, true, true) == peer_prepare_action::observe_map);
  // CSC sees a rotated map first; later server preparation must not clear it.
  static_assert(!client_map_generation_changes("zm_tomb", "zm_tomb", false));
  static_assert(client_map_generation_changes("zm_zod", "zm_tomb", false));
  static_assert(client_map_generation_changes("zm_zod", "zm_zod", true));
  static_assert(!schema_host_eligible(true, 1, 400)); // Solo staging
  static_assert(schema_host_eligible(true, 1, 402));
  static_assert(schema_host_eligible(true, 1, 410));
  static_assert(schema_host_eligible(true, 0, 400)); // actual PUBLIC allocation
  static_assert(schema_host_eligible(false, 1, 400)); // launched native game
  static_assert(!schema_host_eligible(true, 2, 402)); // theater/unknown
  static_assert(!schema_host_eligible(false, -1, 402));
  schema_context_policy policy;
  assert(!policy.retain(0, true, true, true, false));
  policy.peer(10);
  assert(policy.peer_owned());
  assert(policy.retain(11, true, false, false, false)); // MP frontend before ZM handoff
  assert(policy.retain(899999, true, false, false, false));
  assert(!policy.retain(900010, true, false, false, false)); // abandoned query expires
  policy.peer(10);
  assert(policy.retain(20, true, true, false, true)); // native GAME client accepts handoff
  assert(policy.source() == schema_context_policy::origin::peer_active);
  assert(policy.retain(1000000, true, true, false, true)); // valid staging lobby remains allocated
  assert(!policy.retain(1000001, true, true, false, false)); // left native session
  policy.peer(0);
  assert(policy.retain(900001, false, true, false, false)); // loading/live VM cannot unfreeze midmap
  assert(policy.source() == schema_context_policy::origin::peer_active);
  assert(policy.retain(1800000, false, true, false, false)); // map travel
  assert(!policy.retain(1800001, true, false, false, false));
  policy.host();
  assert(!policy.peer_owned());
  assert(policy.retain(0, true, true, true, false));
  assert(!policy.retain(0, true, true, false, false));
  assert(policy.retain(0, false, true, false, false));
  policy.reset();
  assert(!policy.retain(0, false, true, false, false));
  // If the engine refuses a ROM dvar reset, native admission/injection must
  // retain its previous allocation and publish failure, never silently stock.
  int native_slots = 8, script_slots = 8;
  bool failed = false;
  reset_schema_allocation([] { throw 1; }, [&] { native_slots = 0; }, [&] { failed = true; });
  assert(failed && native_slots == 8 && script_slots == 8);
  failed = false;
  reset_schema_allocation([&] { script_slots = 0; }, [&] {
    assert(script_slots == 0); native_slots = 0;
  }, [&] { failed = true; });
  assert(!failed && native_slots == 0 && script_slots == 0);
}
