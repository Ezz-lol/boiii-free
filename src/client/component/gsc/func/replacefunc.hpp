#pragma once

#include <atomic>
#include <unordered_map>

namespace gsc {
namespace func {
namespace replacefunc {
extern std::unordered_map<uintptr_t, uintptr_t> function_replacements;

extern std::atomic_bool detours_enabled;

inline void add_detour(const uintptr_t target_addr,
                       const uintptr_t replacement_addr) {
  function_replacements[target_addr] = replacement_addr;
  detours_enabled.store(true, std::memory_order_release);
}

template <typename T, typename R>
inline void add_detour(const T *target_addr, const R *replacement_addr) {
  return add_detour(reinterpret_cast<uintptr_t>(target_addr),
                    reinterpret_cast<uintptr_t>(replacement_addr));
}

} // namespace replacefunc
} // namespace func
} // namespace gsc
