#include "../src/common/utils/browser_request.hpp"
#include <cassert>
#include <iostream>

int main() {
  utils::browser_request_state request;
  const auto abandoned = request.begin_discovery();
  assert(request.refreshing());
  request.cancel();
  assert(!request.current(abandoned) && !request.refreshing());
  assert(!request.discovered(abandoned, 10));
  assert(!request.complete_one(abandoned));

  const auto scan = request.begin_discovery();
  assert(!request.complete_one(scan));
  assert(request.discovered(scan, 2));
  assert(
      !request.discovered(scan, 2)); // duplicate discovery cannot reset count
  assert(!request.complete_one(scan) && request.refreshing());
  assert(request.complete_one(scan) && !request.refreshing());
  assert(!request.complete_one(scan)); // completion is delivered only once

  const auto refresh = request.begin_queries(1);
  assert(!request.current(scan));
  assert(!request.complete_one(scan) && request.refreshing());
  assert(request.complete_one(refresh) && !request.refreshing());

  const auto pending = request.begin_discovery();
  const auto replacement = request.begin_discovery();
  assert(!request.discovered(pending, 3));
  assert(request.discovered(replacement, 0) && !request.refreshing());
  assert(!request.complete_one(replacement));

  const auto callback = request.begin_queries(1);
  assert(request.complete_one(callback));
  request.cancel(); // cancellation from ServerResponded revokes RefreshComplete
  assert(!request.current(callback));
  std::cout << "PASS: LAN discovery cancellation, refresh generations and one "
               "completion\n";
}
