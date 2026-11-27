#include "../src/common/utils/connection_request.hpp"
#include <cassert>
#include <functional>
#include <future>
#include <iostream>
#include <thread>

int main() {
  utils::connection_request_state requests;
  assert(!requests.current(0));
  const auto lan = requests.begin();
  int selected_network = 0;
  const std::function<void()> delayed_lan = [&] {
    if (requests.current(lan))
      selected_network = 1;
  };
  const auto online = requests.begin();
  delayed_lan();
  assert(selected_network == 0 && !requests.current(lan));
  assert(requests.current(online));
  requests.cancel();
  assert(!requests.current(online));
  delayed_lan();
  assert(selected_network == 0);

  // A DNS response released only after another user request must stay revoked
  // even when its validity check occurs on another thread.
  const auto dns = requests.begin();
  std::promise<void> response_ready;
  auto release = response_ready.get_future();
  bool accepted = true;
  std::thread worker([&] {
    release.wait();
    accepted = requests.current(dns);
  });
  const auto replacement = requests.begin();
  response_ready.set_value();
  worker.join();
  assert(!accepted && requests.current(replacement));
  requests.cancel();
  const auto reconnect = requests.begin();
  assert(requests.current(reconnect) && !requests.current(replacement));
  std::cout << "PASS: connection replacement/cancellation, stale network "
               "actions, cross-thread DNS and reconnect\n";
}
