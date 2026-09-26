#pragma once

#include <std_include.hpp>

namespace launcher {
bool run();
bool is_game_process_running();
const std::filesystem::path &get_launcher_ui_file();
} // namespace launcher
