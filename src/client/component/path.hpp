#pragma once

#include <filesystem>
namespace path {
const std::filesystem::path &cwd();
// E.g. "C:\\"
const std::filesystem::path &root();

const std::filesystem::path &players_directory();
const std::filesystem::path &boiii_players_directory();

const std::filesystem::path &steam_workshop_content_directory();
bool have_steam_workshop_content_directory();
const std::filesystem::path &usermaps_directory();
const std::filesystem::path &mods_directory();

const std::filesystem::path &lpc_directory();

bool is_subpath(const std::filesystem::path &child,
                const std::filesystem::path &parent);
std::filesystem::path replace_root(const std::filesystem::path &path,
                                   const std::filesystem::path &current,
                                   const std::filesystem::path &replacement);

std::filesystem::path normalize_path(const std::filesystem::path &path);

} // namespace path