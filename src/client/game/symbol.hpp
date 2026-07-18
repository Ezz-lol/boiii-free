#pragma once

namespace arxan::detail {
void set_address_to_call(const void *address);
extern void *callstack_proxy_addr;
} // namespace arxan::detail

namespace game {
inline size_t select(const size_t client_val, const size_t server_val);

inline size_t select(const void *client_val, const void *server_val);

template <typename T> class base_symbol {
public:
  constexpr base_symbol(const size_t address) : address_(address) {}

  constexpr base_symbol(const size_t address, const size_t server_address)
      : address_(address), server_address_(server_address) {}

  T *get() const {
    return reinterpret_cast<T *>(select(this->address_, this->server_address_));
  }

  operator T *() const { return this->get(); }

  T *operator->() const { return this->get(); }

private:
  size_t address_{};
  size_t server_address_{};
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