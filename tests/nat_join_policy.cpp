#include "../src/client/component/nat_join_policy.hpp"
#include <cassert>
#include <limits>
#include <string>
using namespace nat::join_policy;
int main() {
  std::uint64_t generation = 0;
  bool permitted = true;
  request_scope old{advance_generation(generation), [&] { return permitted; }};
  const auto scheduled = old;
  assert(scheduled.current(generation));
  permitted = false;
  assert(!scheduled.current(generation)); // consults current predicate, not captured result
  permitted = true;
  request_scope fresh{advance_generation(generation), {}};
  assert(fresh.current(generation) && !scheduled.current(generation));
  assert(!request_scope{}.current(generation));
  generation = (std::numeric_limits<std::uint64_t>::max)();
  assert(advance_generation(generation) == 1);
  const std::string token = "match1_" + std::string(32, 'a');
  assert(strict_match_token(token));
  assert(strict_match_token("match1_" + std::string(32, 'F')));
  for (const auto invalid : {std::string("match"), "match2_" + std::string(32, 'a'),
      "match1_" + std::string(31, 'a'), "match1_" + std::string(33, 'a'),
      "match1_" + std::string(32, 'g'), token + "\n"})
    assert(!strict_match_token(invalid));
  assert(reply_matches(true, true, token, token));
  assert(!reply_matches(false, true, token, token));
  assert(!reply_matches(true, false, token, token));
  assert(!reply_matches(true, true, token, "match1_" + std::string(32, 'b')));
  assert(!reply_matches(true, true, token, "match"));
  assert(reply_matches(true, true, "match", "match")); // legacy wire remains supported
}
