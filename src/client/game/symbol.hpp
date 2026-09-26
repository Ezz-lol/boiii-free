#pragma once

#include "ptr.hpp"
#include <cstdint>

namespace arxan::detail {
void set_address_to_call(const void *address);
extern void *callstack_proxy_addr;
} // namespace arxan::detail

namespace game {
template <typename T> class base_symbol {
public:
  inline constexpr base_symbol(const uintptr_t address) : address(address) {}

  inline constexpr base_symbol(const uintptr_t address,
                               const uintptr_t legacy_address)
      : address(address), legacy_address(legacy_address) {}

  inline constexpr base_symbol(const uintptr_t address,
                               const uintptr_t legacy_address,
                               const uintptr_t server_address)
      : address(address), legacy_address(legacy_address),
        server_address(server_address) {}

  inline constexpr base_symbol(const intptr_t address) : address(address) {}

  inline constexpr base_symbol(const intptr_t address,
                               const intptr_t legacy_address)
      : address(address), legacy_address(legacy_address) {}

  inline constexpr base_symbol(const intptr_t address,
                               const intptr_t legacy_address,
                               const intptr_t server_address)
      : address(address), legacy_address(legacy_address),
        server_address(server_address) {}

  inline T *get() const {
    return reinterpret_cast<T *>(
        select(this->address, this->legacy_address, this->server_address));
  }

  inline operator T *() const { return this->get(); }

  inline T *operator->() const { return this->get(); }

  template <IntegralLike<uintptr_t> Offset>
  inline uintptr_t offset(Offset offset) const noexcept {
    return reinterpret_cast<uintptr_t>(this->get()) +
           static_cast<uintptr_t>(offset);
  }

private:
  uintptr_t address{};
  uintptr_t legacy_address{};
  uintptr_t server_address{};
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
