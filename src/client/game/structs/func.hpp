#pragma once

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif

// Helpers
namespace {
template <typename T, typename... Args> struct stdcall_helper;
// Standard function signatures, similar to std::function
template <typename Ret, typename... Args> struct stdcall_helper<Ret(Args...)> {
  using type = Ret(__stdcall *)(Args...);
};
// C-Style Variadic signature (...)
template <typename Ret, typename... Args>
struct stdcall_helper<Ret(Args..., ...)> {
  using type = Ret(__stdcall *)(Args..., ...);
};
// Legacy flat syntax fallback
template <typename Ret, typename... Args> struct stdcall_helper {
  using type = Ret(__stdcall *)(Args...);
};

template <typename T, typename... Args> struct fastcall_helper;
template <typename Ret, typename... Args> struct fastcall_helper<Ret(Args...)> {
  using type = Ret(__fastcall *)(Args...);
};
template <typename Ret, typename... Args>
struct fastcall_helper<Ret(Args..., ...)> {
  using type = Ret(__fastcall *)(Args..., ...);
};
template <typename Ret, typename... Args> struct fastcall_helper {
  using type = Ret(__fastcall *)(Args...);
};

template <typename T, typename... Args> struct cdecl_helper;
template <typename Ret, typename... Args> struct cdecl_helper<Ret(Args...)> {
  using type = Ret(__cdecl *)(Args...);
};
template <typename Ret, typename... Args>
struct cdecl_helper<Ret(Args..., ...)> {
  using type = Ret(__cdecl *)(Args..., ...);
};
template <typename Ret, typename... Args> struct cdecl_helper {
  using type = Ret(__cdecl *)(Args...);
};

template <typename T, typename... Args> struct thiscall_helper;
template <typename Ret, typename This, typename... Args>
struct thiscall_helper<Ret(This *, Args...)> {
  using type = Ret(__thiscall *)(This *, Args...);
};
template <typename Ret, typename This, typename... Args>
struct thiscall_helper<Ret, This, Args...> {
  using type = Ret(__thiscall *)(This *, Args...);
};

template <typename T, typename... Args> struct func_helper;
template <typename Ret, typename... Args> struct func_helper<Ret(Args...)> {
  using type = Ret (*)(Args...);
};
template <typename Ret, typename... Args>
struct func_helper<Ret(Args..., ...)> {
  using type = Ret (*)(Args..., ...);
};
template <typename Ret, typename... Args> struct func_helper {
  using type = Ret (*)(Args...);
};
} // namespace

// Consolidated API
template <typename T, typename... Args>
using stdcall_t = typename stdcall_helper<T, Args...>::type;
template <typename T, typename... Args>
using fastcall_t = typename fastcall_helper<T, Args...>::type;
template <typename T, typename... Args>
using cdecl_t = typename cdecl_helper<T, Args...>::type;
template <typename T, typename... Args>
using thiscall_t = typename thiscall_helper<T, Args...>::type;
template <typename T, typename... Args>
using func_t = typename func_helper<T, Args...>::type;

template <typename T, typename... Args>
using stdcallPtr = stdcall_t<T, Args...> *;
template <typename T, typename... Args>
using fastcallPtr = fastcall_t<T, Args...> *;
template <typename T, typename... Args> using cdeclPtr = cdecl_t<T, Args...> *;
template <typename T, typename... Args>
using thiscallPtr = thiscall_t<T, Args...> *;
template <typename T, typename... Args> using funcPtr = func_t<T, Args...> *;

#ifdef __clang__
#pragma clang diagnostic pop
#endif