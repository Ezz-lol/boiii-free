#ifndef GAME_HPP
#define GAME_HPP

#include <filesystem>
#include <string>
#include "../../common/utils/nt.hpp" // IWYU pragma: export
#include "symbol.hpp"                // IWYU pragma: export
#include "structs/structs.hpp"       // IWYU pragma: export

namespace arxan::detail {
void set_address_to_call(const void *address);
extern void *callstack_proxy_addr;
} // namespace arxan::detail

namespace game {

constexpr uint32_t APP_ID = 311210;
constexpr const char *APP_ID_STR = "311210";

size_t get_base();
bool is_server();
bool is_client();
bool is_legacy_client();

bool quiet_crash();
bool is_headless();

void show_error(const std::string &text, const std::string &title = "Error");

inline size_t relocate(const size_t val) {
  if (!val)
    return 0;

  const size_t base = get_base();
  return base + (val - 0x140000000);
}

inline size_t derelocate(const size_t val) {
  if (!val)
    return 0;

  const size_t base = get_base();
  return (val - base) + 0x140000000;
}

inline size_t derelocate(const void *val) {
  return derelocate(reinterpret_cast<size_t>(val));
}

inline size_t select(const size_t client_val, const size_t server_val) {
  return relocate(is_server() ? server_val : client_val);
}

inline size_t select(const void *client_val, const void *server_val) {
  return select(reinterpret_cast<size_t>(client_val),
                reinterpret_cast<size_t>(server_val));
}

/// @brief Checks if a pointer resides within the current thread's stack
/// boundaries.
inline bool valid_stack_ptr(uintptr_t ptr) {
  if (!ptr) {
    return false;
  }

  // In x64 Windows, GS:[0x08] holds the Stack Base (highest address)
  // and GS:[0x10] holds the Stack Limit (lowest address).
  // The stack grows downwards from the Base to the Limit.
  uintptr_t stack_base = __readgsqword(0x08);
  uintptr_t stack_limit = __readgsqword(0x10);

  return (ptr >= stack_limit) && (ptr < stack_base);
}

template <typename T> inline bool valid_stack_ptr(T *ptr) {
  return valid_stack_ptr(reinterpret_cast<uintptr_t>(ptr));
}

/// @brief Checks if a pointer's address points to the engine module address
/// space or a location on the current thread's stack.
inline bool valid_engine_ptr(uintptr_t ptr) {
  if (!ptr) {
    return false;
  }

  bool in_module = (ptr >= get_base()) && (ptr < (get_base() + 0x030000000));
  bool on_stack = valid_stack_ptr(ptr);

  return in_module || on_stack;
}

template <typename T> inline bool valid_engine_ptr(T *ptr) {
  return valid_engine_ptr(reinterpret_cast<uintptr_t>(ptr));
}

std::filesystem::path get_appdata_path();
std::filesystem::path get_game_path();
std::vector<std::string> get_registered_dvar_names();
size_t get_registered_dvar_name_count();
} // namespace game

inline size_t operator""_g(const size_t val) { return game::relocate(val); }

#include "symbols/symbols.hpp" // IWYU pragma: export
#include "impl/game/dvar.hpp"  // IWYU pragma: export
#endif