#pragma once

#include <type_traits>

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
using stdcallPtr_t = typename stdcall_helper<T, Args...>::type;
template <typename T, typename... Args>
using fastcallPtr_t = typename fastcall_helper<T, Args...>::type;
template <typename T, typename... Args>
using cdeclPtr_t = typename cdecl_helper<T, Args...>::type;
template <typename T, typename... Args>
using thiscallPtr_t = typename thiscall_helper<T, Args...>::type;
template <typename T, typename... Args>
using funcPtr_t = typename func_helper<T, Args...>::type;

template <typename T, typename... Args>
using stdcall_t = std::remove_pointer_t<stdcallPtr_t<T, Args...>>;
template <typename T, typename... Args>
using fastcall_t = std::remove_pointer_t<fastcallPtr_t<T, Args...>>;
template <typename T, typename... Args>
using cdecl_t = std::remove_pointer_t<cdeclPtr_t<T, Args...>>;
template <typename T, typename... Args>
using thiscall_t = std::remove_pointer_t<thiscallPtr_t<T, Args...>>;
template <typename T, typename... Args>
using func_t = std::remove_pointer_t<funcPtr_t<T, Args...>>;

#ifdef __clang__
#pragma clang diagnostic pop
#endif

template <class...> struct types {
  using type = types;
};
template <class Sig> struct params;
template <class R, class... Params>
struct params<R(Params...)> : types<Params...> {};
template <class Sig> using params_t = typename params<Sig>::type;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif

template <class Sig> struct params<Sig *> : params<Sig> {};
template <class R, class... Params>
struct params<R(Params..., ...)> : types<Params...> {};

#if defined(_M_IX86) || defined(__i386__)
template <class R, class... Params>
struct params<R __cdecl(Params...)> : types<Params...> {};
template <class R, class... Params>
struct params<R __cdecl(Params..., ...)> : types<Params...> {};
template <class R, class... Params>
struct params<R __stdcall(Params...)> : types<Params...> {};
template <class R, class... Params>
struct params<R __stdcall(Params..., ...)> : types<Params...> {};
template <class R, class... Params>
struct params<R __fastcall(Params...)> : types<Params...> {};
template <class R, class... Params>
struct params<R __fastcall(Params..., ...)> : types<Params...> {};
template <class R, class... Params>
struct params<R __thiscall(Params...)> : types<Params...> {};
template <class R, class... Params>
struct params<R __thiscall(Params..., ...)> : types<Params...> {};
#endif

template <class Sig> struct ret;
template <class Sig> struct ret<Sig *> : ret<Sig> {};
template <class R, class... Params> struct ret<R(Params...)> {
  using type = R;
};
template <class R, class... Params> struct ret<R(Params..., ...)> {
  using type = R;
};

#if defined(_M_IX86) || defined(__i386__)
template <class R, class... Params> struct ret<R __cdecl(Params...)> {
  using type = R;
};
template <class R, class... Params> struct ret<R __cdecl(Params..., ...)> {
  using type = R;
};
template <class R, class... Params> struct ret<R __stdcall(Params...)> {
  using type = R;
};
template <class R, class... Params> struct ret<R __stdcall(Params..., ...)> {
  using type = R;
};
template <class R, class... Params> struct ret<R __fastcall(Params...)> {
  using type = R;
};
template <class R, class... Params> struct ret<R __fastcall(Params..., ...)> {
  using type = R;
};
template <class R, class... Params> struct ret<R __thiscall(Params...)> {
  using type = R;
};
template <class R, class... Params> struct ret<R __thiscall(Params..., ...)> {
  using type = R;
};
#endif

template <class Sig> using ret_t = typename ret<Sig>::type;

template <class Sig> struct args;
template <class Sig> struct args : args<params_t<Sig>> {};
template <class... Params>
struct args<types<Params...>> : types<std::decay_t<Params>...> {};
template <class Sig> using args_t = typename args<Sig>::type;

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif

template <typename T> struct is_cdecl : std::false_type {};
template <typename T> struct is_cdecl<T *> : is_cdecl<T> {};
template <typename Ret, typename... Args>
struct is_cdecl<Ret __cdecl(Args...)> : std::true_type {};
template <typename Ret, typename... Args>
struct is_cdecl<Ret __cdecl(Args..., ...)> : std::true_type {};
template <typename T> constexpr bool is_cdecl_v = is_cdecl<T>::value;
template <typename T> using is_cdecl_t = typename is_cdecl<T>::type;

template <typename T> struct is_stdcall : std::false_type {};
template <typename T> struct is_stdcall<T *> : is_stdcall<T> {};
template <typename Ret, typename... Args>
struct is_stdcall<Ret __stdcall(Args...)> : std::true_type {};
template <typename Ret, typename... Args>
struct is_stdcall<Ret __stdcall(Args..., ...)> : std::true_type {};
template <typename T> constexpr bool is_stdcall_v = is_stdcall<T>::value;
template <typename T> using is_stdcall_t = typename is_stdcall<T>::type;

template <typename T> struct is_fastcall : std::false_type {};
template <typename T> struct is_fastcall<T *> : is_fastcall<T> {};
template <typename Ret, typename... Args>
struct is_fastcall<Ret __fastcall(Args...)> : std::true_type {};
template <typename Ret, typename... Args>
struct is_fastcall<Ret __fastcall(Args..., ...)> : std::true_type {};
template <typename T> constexpr bool is_fastcall_v = is_fastcall<T>::value;
template <typename T> using is_fastcall_t = typename is_fastcall<T>::type;

template <typename T> struct is_thiscall : std::false_type {};
template <typename T> struct is_thiscall<T *> : is_thiscall<T> {};
template <typename Ret, typename... Args>
struct is_thiscall<Ret __thiscall(Args...)> : std::true_type {};
template <typename Ret, typename... Args>
struct is_thiscall<Ret __thiscall(Args..., ...)> : std::true_type {};
template <typename T> constexpr bool is_thiscall_v = is_thiscall<T>::value;
template <typename T> using is_thiscall_t = typename is_thiscall<T>::type;

template <typename T>
struct has_explicit_calling_convention
    : std::bool_constant<is_cdecl_v<T> || is_stdcall_v<T> || is_fastcall_v<T> ||
                         is_thiscall_v<T>> {};
template <typename T>
constexpr bool has_explicit_calling_convention_v =
    has_explicit_calling_convention<T>::value;
template <typename T>
using has_explicit_calling_convention_t =
    typename has_explicit_calling_convention<T>::type;

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif

template <typename FuncType, typename... AppendedParams> struct merge_params;

template <typename FuncType, typename... AppendedParams>
struct merge_params<FuncType *, AppendedParams...> {
  using type = typename merge_params<FuncType, AppendedParams...>::type *;
};

template <typename Ret, typename... Args, typename... AppendedParams>
struct merge_params<Ret(Args...), AppendedParams...> {
  using type = Ret(Args..., AppendedParams...);
};

template <typename Ret, typename... Args, typename... AppendedParams>
struct merge_params<Ret(Args..., ...), AppendedParams...> {
  using type = Ret(Args..., AppendedParams..., ...);
};

#if defined(_M_IX86) || defined(__i386__)

template <typename Ret, typename... Args, typename... AppendedParams>
struct merge_params<Ret __cdecl(Args...), AppendedParams...> {
  using type = Ret __cdecl(Args..., AppendedParams...);
};

template <typename Ret, typename... Args, typename... AppendedParams>
struct merge_params<Ret __cdecl(Args..., ...), AppendedParams...> {
  using type = Ret __cdecl(Args..., AppendedParams..., ...);
};

template <typename Ret, typename... Args, typename... AppendedParams>
struct merge_params<Ret __stdcall(Args...), AppendedParams...> {
  using type = Ret __stdcall(Args..., AppendedParams...);
};

template <typename Ret, typename... Args, typename... AppendedParams>
struct merge_params<Ret __stdcall(Args..., ...), AppendedParams...> {
  using type = Ret __stdcall(Args..., AppendedParams..., ...);
};

template <typename Ret, typename... Args, typename... AppendedParams>
struct merge_params<Ret __fastcall(Args...), AppendedParams...> {
  using type = Ret __fastcall(Args..., AppendedParams...);
};

template <typename Ret, typename... Args, typename... AppendedParams>
struct merge_params<Ret __fastcall(Args..., ...), AppendedParams...> {
  using type = Ret __fastcall(Args..., AppendedParams..., ...);
};

template <typename Ret, typename... Args, typename... AppendedParams>
struct merge_params<Ret __thiscall(Args...), AppendedParams...> {
  using type = Ret __thiscall(Args..., AppendedParams...);
};

template <typename Ret, typename... Args, typename... AppendedParams>
struct merge_params<Ret __thiscall(Args..., ...), AppendedParams...> {
  using type = Ret __thiscall(Args..., AppendedParams..., ...);
};

#endif

template <typename FuncType, typename... AppendedParams>
using merge_params_t = typename merge_params<FuncType, AppendedParams...>::type;

#ifdef __clang__
#pragma clang diagnostic pop
#endif