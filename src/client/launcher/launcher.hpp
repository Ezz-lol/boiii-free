#pragma once

#include <std_include.hpp>

namespace launcher {
bool run();
bool is_game_process_running();
const std::filesystem::path &get_launcher_ui_file();
std::string get_steam_workshop_preview_url(const std::string &workshop_id);
bool workshop_remove_by_path(const std::string &path_str, std::string &error);
} // namespace launcher
