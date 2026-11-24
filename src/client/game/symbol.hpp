#pragma once

#include "ptr.hpp"
#include "tac.hpp"

namespace game {
template <typename T> struct base_symbol {
public:
  alwaysinline constexpr base_symbol(const uintptr_t address)
      : address(address) {}

  alwaysinline constexpr base_symbol(const uintptr_t address,
                                     const uintptr_t server_address)
      : address(address), server_address(server_address) {}

  alwaysinline constexpr base_symbol(const intptr_t address)
      : address(address) {}

  alwaysinline constexpr base_symbol(const intptr_t address,
                                     const intptr_t server_address)
      : address(address), server_address(server_address) {}

  alwaysinline T *get() const {
    return reinterpret_cast<T *>(select(this->address, this->server_address));
  }

  alwaysinline operator T *() const { return this->get(); }

  alwaysinline T *operator->() const { return this->get(); }

  template <IntegralLike<uintptr_t> Offset>
  alwaysinline uintptr_t offset(Offset offset) const noexcept {
    return reinterpret_cast<uintptr_t>(this->get()) +
           static_cast<uintptr_t>(offset);
  }

private:
  uintptr_t address{};
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
    return tac::call_safe(*this->get(), args...);
  }
};
} // namespace game
