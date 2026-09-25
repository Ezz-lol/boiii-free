#pragma once

#include "named_mutex.hpp"
#include <filesystem>
#include <mutex>
#include <optional>

namespace utils::properties {
const std::filesystem::path &get_appdata_path();
const std::filesystem::path &get_key_path();

std::unique_lock<named_mutex> lock();

std::optional<std::string> load(const std::string_view &name);

void store(const std::string_view &name, const std::string_view &value);
} // namespace utils::properties
