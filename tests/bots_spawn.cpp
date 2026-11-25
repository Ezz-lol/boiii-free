#include "../src/client/component/bots/spawn_policy.hpp"

#include <cassert>
#include <iostream>
#include <limits>

using namespace bots::policy;

static session_snapshot session(int capacity = 8, int occupied = 0,
                                bool zombies = true) {
  return {.epoch = 12,
          .mode = zombies ? 1 : 2,
          .capacity = capacity,
          .occupied = occupied,
          .running = true,
          .zombies = zombies,
          .schema_ready = zombies && capacity > 4,
          .schema_capacity = zombies && capacity > 4 ? capacity : 0,
          .map = "zm_zod"};
}

int main() {
  assert(parse_count(std::nullopt) == 1);
  assert(parse_count("1") == 1 && parse_count("8") == 8 &&
         parse_count("18") == 18);
  assert(parse_count("all") == 18 && parse_count("0008") == 8);
  for (const auto text :
       {"", "0", "-1", "+1", " 1", "1 ", "8junk", "1.0", "19", "allx", "ALL",
        "9999999999999999999999999999999999999999"})
    assert(!parse_count(text));
  for (int value = 1; value <= 18; ++value)
    assert(parse_count(std::to_string(value)) == value);

  // Eight dedicated bots, departures to four, then refill only the actual
  // slots.
  auto live = session();
  auto captured = live;
  auto read = [&]() { return std::optional{live}; };
  auto add = [&]() {
    ++live.occupied;
    return true;
  };
  assert(spawn_batch(captured, *parse_count("all"), read, add) == 8 &&
         live.occupied == 8);
  live.occupied = 4;
  assert(spawn_batch(captured, *parse_count("all"), read, add) == 4 &&
         live.occupied == 8);
  assert(spawn_batch(captured, 1, read, add) == 0);

  // A listen host owns one slot; queued human connections also reserve space.
  live = session(8, 1);
  captured = live;
  assert(spawn_batch(captured, 18, read, add) == 7 && live.occupied == 8);
  live = session(8, 1);
  captured = live;
  live.occupied = 4;
  assert(spawn_batch(captured, 18, read, add) == 4 && live.occupied == 8);
  live = session(8, 1);
  captured = live;
  assert(spawn_batch(captured, 18, read, [&]() {
           live.occupied += 7;
           return true;
         }) == 1);
  assert(live.occupied == 8);

  // No stale batch may move into a new map, same-map restart, mode or capacity.
  const auto expect_refused = [&](const session_snapshot &changed) {
    const auto old = session();
    int calls = 0;
    assert(spawn_batch(
               old, 8, [&]() { return std::optional{changed}; },
               [&]() {
                 ++calls;
                 return true;
               }) == 0);
    assert(calls == 0);
  };
  auto changed = session();
  changed.epoch++;
  expect_refused(changed);
  changed = session();
  changed.map = "zm_tomb";
  expect_refused(changed);
  changed = session();
  changed.mode++;
  expect_refused(changed);
  changed = session();
  changed.capacity = 4;
  changed.schema_capacity = 0;
  expect_refused(changed);
  changed = session();
  changed.schema_ready = false;
  expect_refused(changed);
  changed = session();
  changed.schema_capacity = 5;
  expect_refused(changed);
  changed = session();
  changed.running = false;
  expect_refused(changed);
  live = session();
  captured = live;
  assert(spawn_batch(captured, 8, read, [&]() {
           ++live.occupied;
           ++live.epoch;
           return true;
         }) == 1);
  live = session();
  captured = live;
  assert(spawn_batch(captured, 8, read, []() { return false; }) == 0);
  assert(spawn_batch(
             captured, 8, []() { return std::optional<session_snapshot>{}; },
             []() {
               assert(false);
               return false;
             }) == 0);

  // Stock Zombies remains four; baseline MP/CP can use each of the 18 wire
  // slots.
  live = session(4);
  captured = live;
  assert(spawn_batch(captured, 18, read, add) == 4);
  live = session(18, 0, false);
  captured = live;
  assert(spawn_batch(captured, 1, read, add) == 1);
  assert(spawn_batch(captured, 18, read, add) == 17 && live.occupied == 18);
  live = session(18, 0, false);
  live.mode = 3;
  captured = live;
  assert(spawn_batch(captured, 18, read, add) == 18);
  assert(!valid(session(0)) && !valid(session(19)) && !valid(session(18)));
  assert(!valid(session(8, -1)) && !valid(session(8, 9)));
  assert(spawn_batch(session(), 0, read, add) == 0);
  assert(spawn_batch(session(), std::numeric_limits<int>::max(), read, add) ==
         0);
  assert(parse_unsigned("0", 17) == 0 && parse_unsigned("17", 17) == 17);
  assert(!parse_unsigned("18", 17) && !parse_unsigned("-1", 17) &&
         !parse_unsigned("1junk", 17));
  assert(parse_unsigned("18446744073709551615", UINT64_MAX) == UINT64_MAX);
  assert(!parse_unsigned("18446744073709551616", UINT64_MAX));
  assert(userinfo_component(std::string(5000, 'a'), player_name_bytes).size() ==
         31);
  assert(userinfo_component(std::string(5000, 't'), clan_abbrev_bytes).size() ==
         7);
  assert(userinfo_component("^1Good name", player_name_bytes) == "^1Good name");
  assert(userinfo_component("bad\\\";\nname", player_name_bytes) ==
         "bad____name");
  assert(userinfo_component(std::string("ab\0cd", 5), player_name_bytes) ==
         "ab_cd");
  assert(userinfo_component("", player_name_bytes).empty() &&
         userinfo_component("a", 0).empty());
  const auto current_session = session();
  assert(may_drop_test_client(current_session, current_session, 12, 7, true,
                              true));
  assert(!may_drop_test_client(current_session, current_session, 11, 7, true,
                               true));
  assert(!may_drop_test_client(current_session, current_session, 12, 8, true,
                               true));
  assert(!may_drop_test_client(current_session, current_session, 12, -1, true,
                               true));
  assert(!may_drop_test_client(current_session, current_session, 12, 7, true,
                               false)); // human reused slot
  assert(!may_drop_test_client(current_session, current_session, 12, 7, false,
                               true)); // freed/zombie bot
  auto replaced = current_session;
  replaced.epoch++;
  assert(!may_drop_test_client(current_session, replaced, 12, 7, true, true));
  replaced = current_session;
  replaced.capacity = 4;
  assert(!may_drop_test_client(current_session, replaced, 12, 3, true, true));
  std::cout << "PASS strict bot counts, actual occupied slots, 8->4->8, "
               "host+7, lifecycle invalidation, stock4 and MP/CP18\n";
}
