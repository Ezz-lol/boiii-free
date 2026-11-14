#pragma once

#include <atomic>
#include <string>

namespace exception {
void show_fatal_error(const std::string &message);
[[noreturn]] void restart_game();

extern std::atomic_size_t known_blocking_call;
} // namespace exception
