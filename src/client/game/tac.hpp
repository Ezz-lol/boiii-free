#pragma once

#include <cstdint>
#include <macros/builtin.hpp>

namespace game {
namespace tac {
extern void *callstack_proxy_addr;

extern thread_local const void *address_to_call;

void set_address_to_call(const void *address);
const void *get_address_to_call();
void store_address(uintptr_t address);
uintptr_t get_stored_address();

template <typename T, typename... Args>
alwaysinline T call_safe(T (&func)(Args...), Args... args) {
  set_address_to_call(&func);
  return static_cast<T (*)(Args...)>(callstack_proxy_addr)(args...);
}
} // namespace tac
} // namespace game
