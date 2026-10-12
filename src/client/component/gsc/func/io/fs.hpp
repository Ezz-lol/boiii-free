#pragma once

#include <filesystem>

#include <component/path.hpp>

namespace gsc {
namespace func {
namespace io {
namespace fs {
const std::filesystem::path &scriptdata_path();

inline std::filesystem::path resolve_path(const std::filesystem::path &path) {
  return std::filesystem::weakly_canonical(scriptdata_path() / path);
}

inline bool is_safe_path(const std::filesystem::path &path) {
  return path::has_parent(resolve_path(path), scriptdata_path());
}

inline std::filesystem::path
relative_path(const std::filesystem::path &full_path) {
  return std::filesystem::relative(full_path, scriptdata_path());
}
} // namespace fs
} // namespace io
} // namespace func
} // namespace gsc
