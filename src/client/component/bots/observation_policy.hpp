#pragma once
// Read-only native identity. This type cannot grant bot allocation or removal.
#include "spawn_policy.hpp"
namespace bots::observation {
struct snapshot {
  std::uint64_t epoch{};
  int native_epoch{}, mode{}, capacity{}, frozen_capacity{};
  bool host{}, initialized{}, schema_failed{}, source_current{};
  std::string map, frozen_map;
};
inline bool valid_map(std::string_view map) noexcept {
  if (map.empty() || map.size() > 64 || map == "core_frontend")
    return false;
  for (const char c : map)
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'))
      return false;
  return true;
}
inline bool valid(const snapshot &value) noexcept {
  if (!value.host || !value.initialized || !value.epoch ||
      !valid_map(value.map) || value.mode < 0 || value.mode > 2 ||
      value.capacity < 1 || value.capacity > policy::maximum_slots)
    return false;
  if (value.mode != 0)
    return true; // Preserve bounded baseline MP/CP observation.
  if (value.schema_failed || value.capacity > 8 || value.frozen_capacity < 0 ||
      value.frozen_capacity > 8)
    return false;
  // Even a <=4 native capacity must not mask a frozen expanded allocation.
  if (value.capacity > 4 || value.frozen_capacity > 4)
    return value.capacity >= 5 && value.capacity <= 8 &&
           value.frozen_capacity == value.capacity && value.native_epoch > 0 &&
           value.source_current && value.frozen_map == value.map;
  return value.frozen_capacity == 0 || value.frozen_capacity == value.capacity;
}
inline bool same_session(const snapshot &requested,
                         const snapshot &current) noexcept {
  return valid(requested) && valid(current) &&
         requested.epoch == current.epoch &&
         requested.native_epoch == current.native_epoch &&
         requested.mode == current.mode &&
         requested.capacity == current.capacity &&
         requested.frozen_capacity == current.frozen_capacity &&
         requested.map == current.map &&
         requested.frozen_map == current.frozen_map;
}
} // namespace bots::observation
