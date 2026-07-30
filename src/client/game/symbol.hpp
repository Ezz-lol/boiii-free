#pragma once

#include <cstdint>
#include "ptr.hpp"
namespace arxan::detail {
void set_address_to_call(const void *address);
extern void *callstack_proxy_addr;
} // namespace arxan::detail

namespace game {
template <typename T> class base_symbol {
public:
  inline constexpr base_symbol(const uintptr_t address) : address_(address) {}

  inline constexpr base_symbol(const uintptr_t address,
                               const uintptr_t server_address)
      : address_(address), server_address_(server_address) {}

  inline constexpr base_symbol(const intptr_t address) : address_(address) {}

  inline constexpr base_symbol(const intptr_t address,
                               const intptr_t server_address)
      : address_(address), server_address_(server_address) {}

  T *get() const {
    return reinterpret_cast<T *>(select(this->address_, this->server_address_));
  }

  operator T *() const { return this->get(); }

  T *operator->() const { return this->get(); }

private:
  uintptr_t address_{};
  uintptr_t server_address_{};
};

template <typename T> struct symbol : base_symbol<T> {
  using base_symbol<T>::base_symbol;
};

template <typename T, typename... Args>
struct symbol<T(Args...)> : base_symbol<T(Args...)> {
  using func_type = T(Args...);

  using base_symbol<func_type>::base_symbol;

  T call_safe(Args... args) const {
    arxan::detail::set_address_to_call(this->get());
    return static_cast<func_type *>(arxan::detail::callstack_proxy_addr)(
        args...);
  }
};
} // namespace game