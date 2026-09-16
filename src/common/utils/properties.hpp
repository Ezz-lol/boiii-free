#pragma once

#include "named_mutex.hpp"
#include <filesystem>
#include <mutex>
#include <optional>

namespace utils::properties {
std::filesystem::path get_appdata_path();
std::filesystem::path get_key_path();

std::unique_lock<named_mutex> lock();

std::optional<std::string> load(const std::string &name);

void store(const std::string &name, const std::string &value);
} // namespace utils::properties
