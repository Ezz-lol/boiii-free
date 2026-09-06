#ifndef NDEBUG
#include <std_include.hpp>

#include "log.hpp"

#include <game/flags.hpp>
#include "concurrentqueue/blockingconcurrentqueue.h"
#include <utils/thread.hpp>

namespace game {
namespace log {
static moodycamel::BlockingConcurrentQueue<std::string> message_queue(0xFF);

namespace thread {
static std::thread background_thread;

void worker_loop() {
  std::ofstream &debug_log = game::tracing_logfile();
  std::string msg;

  while (true) {
    message_queue.wait_dequeue(msg);

    // Empty message triggers shutdown
    if (msg.empty()) {
      break;
    }

    debug_log << msg;

    // Flush once per batch
    debug_log.flush();
  }
}

void startup() {
  background_thread = std::thread(worker_loop);
  utils::thread::set_name(background_thread, "Logger");
}
void shutdown() {
  // Send empty string to trigger shutdown
  message_queue.enqueue("");
  if (background_thread.joinable()) {
    background_thread.join();
  }
}
} // namespace thread

__inline_def void enqueue(const std::string &msg) {
  message_queue.enqueue(std::move(msg));
}

} // namespace log
} // namespace game
#endif