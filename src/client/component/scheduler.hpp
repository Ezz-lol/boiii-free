#pragma once

#include <csetjmp>

namespace scheduler {
enum pipeline {
  // Asynchronuous pipeline, disconnected from the game
  async = 0,

  // The game's rendering pipeline
  renderer,

  // The game's server thread
  server,

  // The game's main thread
  main,

  // Dvars flags have been patched, ready to be set from config file
  dvars_flags_patched,

  // Dvars are done loading from the config file
  dvars_loaded,

  count,
};

static constexpr bool cond_continue = false;
static constexpr bool cond_end = true;

void execute(pipeline type);

void schedule(const std::function<bool()> &callback, pipeline type,
              std::chrono::milliseconds delay = 0ms);
void loop(const std::function<void()> &callback, pipeline type,
          std::chrono::milliseconds delay = 0ms);
void once(const std::function<void()> &callback, pipeline type,
          std::chrono::milliseconds delay = 0ms);
void on_game_initialized(const std::function<void()> &callback, pipeline type,
                         std::chrono::milliseconds delay = 0ms);
} // namespace scheduler

namespace server_restart {
// Custom exception code for script error abort (triggers SEH unwind with
// __finally cleanup)
static constexpr DWORD SCRIPT_ERROR_EXCEPTION = 0xE0530001;

inline std::atomic<int> restart_count{0};
inline std::atomic<bool> restart_pending{false};
inline std::atomic<int64_t> restart_execute_time{0};
inline std::atomic<int> consecutive_crash_count{0};
inline std::atomic<bool> restart_recovery_active{false};
inline std::atomic<int> recovery_skip_count{0};
inline std::atomic<bool> last_error_is_link{false};
inline thread_local jmp_buf game_frame_jmp{};
inline thread_local bool game_frame_jmp_set{false};

bool schedule(const char *reason,
              std::chrono::seconds delay = std::chrono::seconds(10));
void check_and_execute();
void abort_game_frame();
} // namespace server_restart
