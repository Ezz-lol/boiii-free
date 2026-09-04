#include <std_include.hpp>

#include "path.hpp"

#include <game/game.hpp>

#ifndef NDEBUG
#include <utils/string.hpp>
#endif

namespace path {
using namespace game::sys;
using namespace game::fs;

std::filesystem::path cwd() { return std::filesystem::path(Sys_Cwd()); }

std::filesystem::path players_directory() {
  static const std::filesystem::path path =
      std::filesystem::weakly_canonical(cwd() / "players");
  return path;
}

std::filesystem::path boiii_players_directory() {
  static const std::filesystem::path path =
      std::filesystem::weakly_canonical(cwd() / "boiii_players");
  return path;
}

std::filesystem::path steam_workshop_content_directory() {
  static const std::filesystem::path path = std::filesystem::weakly_canonical(
      cwd() / "../../workshop/content" / game::APP_ID_STR);
  return path;
}

std::filesystem::path usermaps_directory() {
  static const std::filesystem::path path =
      std::filesystem::weakly_canonical(cwd() / "usermaps");
  return path;
}

std::filesystem::path mods_directory() {
  static const std::filesystem::path path =
      std::filesystem::weakly_canonical(cwd() / "mods");
  return path;
}

bool is_subpath(const std::filesystem::path &child,
                const std::filesystem::path &parent) {
  const std::filesystem::path child_norm = child.lexically_normal();
  const std::filesystem::path parent_norm = parent.lexically_normal();

  const std::filesystem::path rel =
      std::filesystem::relative(child_norm, parent_norm);

  if (rel.empty() || rel.native()[0] == '.') {
    return false;
  }

  return true;
}

std::filesystem::path replace_root(const std::filesystem::path &path,
                                   const std::filesystem::path &current,
                                   const std::filesystem::path &replacement) {
  if (is_subpath(path, current)) {
    const std::filesystem::path rel = std::filesystem::relative(path, current);
    return replacement / rel;
  }

  return path;
}

static std::vector<std::pair<std::filesystem::path, std::filesystem::path>>
    root_replacements;
static std::once_flag root_replacements_flag;
void set_root_replacements() {
  root_replacements.push_back({players_directory(), boiii_players_directory()});
}

std::filesystem::path
try_replace_workshop_root(const std::filesystem::path &path) {
  if (!std::filesystem::exists(steam_workshop_content_directory()) &&
      is_subpath(path, steam_workshop_content_directory())) {
    const std::filesystem::path with_usermaps_root = replace_root(
        path, steam_workshop_content_directory(), usermaps_directory());
    if (std::filesystem::exists(with_usermaps_root)) {
      return with_usermaps_root;
    }

    return replace_root(path, steam_workshop_content_directory(),
                        mods_directory());
  }

  return path;
}

std::filesystem::path normalize_path(const std::filesystem::path &path) {
  std::call_once(root_replacements_flag, set_root_replacements);

  std::filesystem::path canonicalized = std::filesystem::weakly_canonical(path);

  for (const auto &[root, replacement] : root_replacements) {
    canonicalized = replace_root(canonicalized, root, replacement);
  }

  canonicalized = try_replace_workshop_root(canonicalized);

#ifndef NDEBUG
  const std::string path_str = utils::string::convert(path.native());
  const std::string canonicalized_str =
      utils::string::convert(canonicalized.native());
  game::trace("[Path] Normalized \"%s\" -> \"%s\"", path_str.c_str(),
              canonicalized_str.c_str());
#endif
  return canonicalized;
}

} // namespace path