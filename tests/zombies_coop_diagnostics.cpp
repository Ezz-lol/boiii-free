#include "../src/client/component/zombies/diagnostic_ring.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace zombies::coop::diagnostics_detail;

int main() {
  assert(parse_integer("8") == 8 && parse_integer("-1") == -1);
  assert(parse_integer("2147483647") == 2147483647 &&
         parse_integer("-2147483648") == (-2147483647 - 1));
  for (const auto *invalid :
       {"", "x", "8junk", "2147483648", "-2147483649", "1.2", " 8"})
    assert(parse_integer(invalid) == 0);
  event_buffer ring;
  std::array<event, event_limit> events;
  assert(ring.copy(events) == 0);
  for (int i = 0; i < 80; ++i)
    ring.record("phase", i, -i, i + 1000);
  assert(ring.copy(events) == 64 && events.front().sequence == 17 &&
         events.back().sequence == 80);
  for (const auto &value : events)
    assert(value.b == -value.a &&
           value.tick_ms == static_cast<std::uint64_t>(value.a + 1000));
  ring.record(std::string(1000, 'x'), 0, 0, 0);
  assert(ring.copy(events) == 64 && events.back().phase.back() == 0);
  assert(std::string(events.back().phase.data()).size() == phase_limit - 1);
  ring.record("bad\n\"phase", 0, 0, 0);
  assert(ring.copy(events) == 64 &&
         std::string(events.back().phase.data()) == "bad__phase");

  event_buffer concurrent;
  std::atomic<bool> finished{false};
  std::thread reader([&] {
    std::array<event, event_limit> observed;
    while (!finished.load()) {
      const auto count = concurrent.copy(observed);
      assert(count <= event_limit);
      for (std::size_t i = 0; i < count; ++i) {
        const auto &value = observed[i];
        assert(value.b == -value.a &&
               value.tick_ms == static_cast<std::uint64_t>(value.a + 1000));
        assert(std::string(value.phase.data()) == "writer");
        if (i)
          assert(observed[i - 1].sequence < value.sequence);
      }
    }
  });
  std::vector<std::thread> writers;
  for (int writer = 0; writer < 8; ++writer)
    writers.emplace_back([&, writer] {
      for (int i = 0; i < 4000; ++i) {
        const auto value = static_cast<std::int64_t>(writer * 4000 + i);
        concurrent.record("writer", value, -value, value + 1000);
      }
    });
  for (auto &writer : writers)
    writer.join();
  finished.store(true);
  reader.join();
  assert(concurrent.copy(events) > 0);

  snapshot_text<128> cache;
  std::array<char, 128> text;
  std::size_t length;
  assert(!cache.copy(text, length));
  assert(cache.publish("ready") && cache.copy(text, length) &&
         std::string(text.data(), length) == "ready");
  assert(cache.publish(std::string(1000, 'x')) && cache.copy(text, length) &&
         length == 127 && text[127] == 0);
  finished.store(false);
  std::thread cached_reader([&] {
    std::array<char, 128> copy;
    std::size_t size;
    while (!finished.load())
      if (cache.copy(copy, size)) {
        assert(size == 127 && copy[127] == 0);
        for (std::size_t i = 1; i < size; ++i)
          assert(copy[i] == copy[0]);
      }
  });
  writers.clear();
  for (int writer = 0; writer < 4; ++writer)
    writers.emplace_back([&, writer] {
      const std::string value(127, static_cast<char>('a' + writer));
      for (int i = 0; i < 1500; ++i)
        cache.publish(value);
    });
  for (auto &writer : writers)
    writer.join();
  finished.store(true);
  cached_reader.join();
  std::cout << "PASS bounded lock-free event retention, concurrent complete "
               "records, sanitization and nonblocking cached snapshots\n";
}
