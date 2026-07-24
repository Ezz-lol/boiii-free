#pragma once

#include <std_include.hpp>

namespace launcher {
bool run();
bool is_game_process_running();
std::filesystem::path get_launcher_ui_file();
} // namespace launcher