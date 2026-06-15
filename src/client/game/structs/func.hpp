#pragma once

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif
template <typename T, typename... Args>
using stdcall_t = T(__stdcall *)(Args...);
#ifdef __clang__
#pragma clang diagnostic pop
#endif

template <typename T, typename... Args>
using stdcallPtr = stdcall_t<T, Args...> *;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#endif
template <typename T, typename... Args>
using fastcall_t = T(__fastcall *)(Args...);
#ifdef __clang__
#pragma clang diagnostic pop
#endif

template <typename T, typename... Args>
using fastcallPtr = fastcall_t<T, Args...> *;

template <typename T, typename... Args> using cdecl_t = T(__cdecl *)(Args...);
template <typename T, typename... Args> using cdeclPtr = cdecl_t<T, Args...> *;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#endif
template <typename T, typename This = void, typename... Args>
using thiscall_t = T(__thiscall *)(This *, Args...);
#ifdef __clang__
#pragma clang diagnostic pop
#endif

template <typename T, typename This = void, typename... Args>
using thiscallPtr = thiscall_t<T, This, Args...> *;

template <typename T, typename... Args> using func_t = T (*)(Args...);
template <typename T, typename... Args> using funcPtr = func_t<T, Args...> *;