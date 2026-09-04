#include <std_include.hpp>

#include "path.hpp"

#include <game/game.hpp>

#ifndef NDEBUG
#include <utils/string.hpp>
#endif

namespace path {

const std::filesystem::path &cwd() {
  static const std::filesystem::path path{game::sys::Sys_Cwd()};
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
  static const bool result =
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

bool reparent(std::filesystem::path &path, const std::filesystem::path &current,
              const std::filesystem::path &replacement) {
  const std::filesystem::path rel = relative(path, current);
  if (!rel.empty() && rel.native()[0] != '.') {
    path = replacement / std::move(rel);
    return true;
  }
  return false;
}

std::filesystem::path strip_parts(const std::filesystem::path &p,
                                  size_t count) {
  if (p.empty() || p.begin() == p.end()) {
    return {};
  }

  std::filesystem::path::iterator it = p.begin();
  std::advance(it, count);

  std::filesystem::path result;
  for (; it != p.end(); ++it) {
    result /= *it; // Append each subsequent part
  }

  return result;
}

bool try_replace_workshop_root(std::filesystem::path &path) {
  if (!have_steam_workshop_content_directory()) {

    // Calculate relative path once to avoid duplicate reparent overhead
    const std::filesystem::path rel =
        std::filesystem::relative(path, steam_workshop_content_directory());
    // Check if path is a child of the Steam workshop content directory
    if (!rel.empty() && rel.native()[0] != '.') {
      const std::filesystem::path rel_zone_parented =
          part_count(rel) == 1 ? std::move(rel) / "zone"
                               : part(rel, 0) / "zone" / strip_parts(rel, 1);
      const std::filesystem::path in_usermaps_tree =
          usermaps_directory() / rel_zone_parented;

      if (std::filesystem::exists(in_usermaps_tree)) {
        path = in_usermaps_tree;
        return true;
      }

      const std::filesystem::path in_mods_tree =
          mods_directory() / std::move(rel_zone_parented);
      if (std::filesystem::exists(in_mods_tree)) {
        path = in_mods_tree;
        return true;
      }
    }
  }
  return false;
}

std::filesystem::path normalize(const std::filesystem::path &path) {
  static const ParentMatch parent_replacements[] = {
      {players_directory(), boiii_players_directory()},
      {root() / "LPC", lpc_directory()}};

  std::filesystem::path canonicalized = std::filesystem::weakly_canonical(path);

  if (!try_replace_workshop_root(canonicalized)) {
    for (size_t i = 0; i < std::size(parent_replacements) &&
                       !reparent(canonicalized, parent_replacements[i]);
         ++i) {
    }
  }

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