#include <std_include.hpp>

#include "path.hpp"

#include <game/game.hpp>

#ifndef NDEBUG
#include <utils/string.hpp>
#endif

namespace path {
using namespace game::sys;
using namespace game::fs;

const std::filesystem::path &cwd() {
  static const std::filesystem::path path = std::filesystem::path(Sys_Cwd());
  return path;
}

const std::filesystem::path &root() {
  static const std::filesystem::path path = cwd().root_path();
  return path;
}

const std::filesystem::path &players_directory() {
  static const std::filesystem::path path =
      std::filesystem::weakly_canonical(cwd() / "players");
  return path;
}

const std::filesystem::path &boiii_players_directory() {
  static const std::filesystem::path path =
      std::filesystem::weakly_canonical(cwd() / "boiii_players");
  return path;
}

const std::filesystem::path &steam_workshop_content_directory() {
  static const std::filesystem::path path = std::filesystem::weakly_canonical(
      cwd() / "../../workshop/content" / game::APP_ID_STR);
  return path;
}

bool have_steam_workshop_content_directory() {
  const bool result =
      std::filesystem::exists(steam_workshop_content_directory());
  return result;
}

const std::filesystem::path &usermaps_directory() {
  static const std::filesystem::path path =
      std::filesystem::weakly_canonical(cwd() / "usermaps");
  return path;
}

const std::filesystem::path &mods_directory() {
  static const std::filesystem::path path =
      std::filesystem::weakly_canonical(cwd() / "mods");
  return path;
}

const std::filesystem::path &lpc_directory() {
  static const std::filesystem::path path =
      std::filesystem::weakly_canonical(cwd() / "LPC");
  return path;
}

bool is_subpath(const std::filesystem::path &child,
                const std::filesystem::path &base) {
  const auto [base_it, child_it] =
      std::mismatch(base.begin(), base.end(), child.begin(), child.end());

  return base_it == base.end() && child_it != child.end();
}

std::filesystem::path replace_root(const std::filesystem::path &path,
                                   const std::filesystem::path &current,
                                   const std::filesystem::path &replacement) {
  return is_subpath(path, current)
             ? replacement / std::filesystem::relative(path, current)
             : path;
}

std::filesystem::path
try_replace_workshop_root(const std::filesystem::path &path) {
  if (!have_steam_workshop_content_directory() &&
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
  static const std::pair<std::filesystem::path, std::filesystem::path>
      root_replacements[] = {{players_directory(), boiii_players_directory()},
                             {root() / "LPC", lpc_directory()}};

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