#pragma once

#include <filesystem>

namespace game {
size_t get_base();
bool is_server();
bool is_client();
bool is_legacy_client();
std::filesystem::path game_directory();
} // namespace game