#pragma once

#include <filesystem>
namespace path {

struct ParentMatch {
  std::filesystem::path parent;
  std::filesystem::path replacement;
};

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

// Assumes both paths have already been canonicalized prior to call
inline bool has_parent(const std::filesystem::path &child,
                       const std::filesystem::path &base) {
  return base.native().size() < child.native().size() &&
         std::equal(base.begin(), base.end(), child.begin());
}

bool reparent(std::filesystem::path &path, const std::filesystem::path &current,
              const std::filesystem::path &replacement);

inline bool reparent(std::filesystem::path &path,
                     const ParentMatch &replacement) {
  return reparent(path, replacement.parent, replacement.replacement);
}

std::filesystem::path normalize(const std::filesystem::path &path);

} // namespace path