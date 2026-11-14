#pragma once

#include <chrono>
#include <memory>
#include <std_include.hpp>
#include <utils/concurrency.hpp>
#include <vector>

namespace scheduler {
enum class pipeline : uint8_t {
  first = 0x0,
  async = 0x0,
  renderer = 0x1,
  server = 0x2,
  main = 0x3,
  dvars_flags_patched = 0x4,
  dvars_loaded = 0x5,
  count = 0x6,
};
IMPL_ENUM_OPERATORS(pipeline);

static constexpr bool COND_CONTINUE = false;
static constexpr bool COND_END = true;

typedef std::function<bool()> conditionalSchedulerCallback_t;
typedef std::function<void()> schedulerCallback_t;

void execute(pipeline type);

void schedule(const conditionalSchedulerCallback_t &callback, pipeline type,
              std::chrono::milliseconds delay = 0ms);

void loop(const schedulerCallback_t &callback, pipeline type,
          std::chrono::milliseconds delay = 0ms);
void once(const schedulerCallback_t &callback, pipeline type,
          std::chrono::milliseconds delay = 0ms);

} // namespace scheduler
