#pragma once

namespace exception {
void show_fatal_error(const std::string &message);
[[noreturn]] void restart_game();
} // namespace exception
