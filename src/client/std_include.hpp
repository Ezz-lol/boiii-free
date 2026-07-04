#pragma once

#define BINARY_PAYLOAD_SIZE 0x14000000

// Decide whether to load the game as lib or to inject it
#define INJECT_HOST_AS_LIB

#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4127)
#pragma warning(disable : 4244)
#pragma warning(disable : 4458)
#pragma warning(disable : 4702)
#pragma warning(disable : 4996)
#pragma warning(disable : 5054)
#pragma warning(disable : 5056)
#pragma warning(disable : 6011)
#pragma warning(disable : 6297)
#pragma warning(disable : 6385)
#pragma warning(disable : 6386)
#pragma warning(disable : 6387)
#pragma warning(disable : 26110)
#pragma warning(disable : 26451)
#pragma warning(disable : 26444)
#pragma warning(disable : 26451)
#pragma warning(disable : 26489)
#pragma warning(disable : 26495)
#pragma warning(disable : 26498)
#pragma warning(disable : 26812)
#pragma warning(disable : 28020)

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mshtml.h>
#include <mshtmhst.h>
#include <exdisp.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <corecrt_io.h>
#include <fcntl.h>
#include <shellapi.h>
#include <csetjmp>
#include <shlobj.h>
#include <winternl.h>
#include <versionhelpers.h>
#include <psapi.h>
#include <urlmon.h>
#include <atlbase.h>
#include <atlsafe.h>
#include <iphlpapi.h>
#include <wincrypt.h>
#include <dwmapi.h>
#include <shellscalingapi.h>
#include <d3d11.h>
#include <dxgi1_6.h>

// min and max is required by gdi, therefore NOMINMAX won't work
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#undef GetObject

#include <cassert>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cstring>

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <regex>
#include <sstream>
#include <thread>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <asmjit/core/jitruntime.h>
#include <asmjit/x86/x86assembler.h>
#include <MinHook.h>
#include <udis86.h>

#define RAPIDJSON_NOEXCEPT
#define RAPIDJSON_ASSERT(cond)                                                 \
  if (cond)                                                                    \
    ;                                                                          \
  else                                                                         \
    throw std::runtime_error("rapidjson assert fail");

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#pragma warning(pop)
#pragma warning(disable : 4100)

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#ifndef __inline_def
#if defined(__clang__) || defined(__GNUC__)
#define __inline_def __attribute__((always_inline))
#elif defined(_MSC_VER)
#define __inline_def __forceinline
#define TEMPLATE_INVALID_HANDLE_VALUE -1
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#ifndef __optimize
#if defined(__clang__) || defined(__GNUC__)
#define __optimize __attribute__((hot))
#elif defined(_MSC_VER)
// No equivalent that I can find. Add if found or known otherwise.
#define __optimize
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#ifndef unreachable
#ifdef NDEBUG
#if defined(__GNUC__) || defined(__clang__)
#define unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define unreachable() __assume(false)
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#else
#define unreachable() assert(false && "This code should be unreachable.")
#endif
#endif

using namespace std::literals;