#pragma once

#include <filesystem>
namespace path {
std::filesystem::path players_directory();
std::filesystem::path boiii_players_directory();

bool is_subpath(const std::filesystem::path &child,
                const std::filesystem::path &parent);
std::filesystem::path replace_root(const std::filesystem::path &path,
                                   const std::filesystem::path &current,
                                   const std::filesystem::path &replacement);

std::filesystem::path normalize_path(const std::filesystem::path &path);

} // namespace path