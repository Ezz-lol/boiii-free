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

#include <windows.h>         // IWYU pragma: export
#include <mshtml.h>          // IWYU pragma: export
#include <mshtmhst.h>        // IWYU pragma: export
#include <exdisp.h>          // IWYU pragma: export
#include <winsock2.h>        // IWYU pragma: export
#include <ws2tcpip.h>        // IWYU pragma: export
#include <corecrt_io.h>      // IWYU pragma: export
#include <fcntl.h>           // IWYU pragma: export
#include <shellapi.h>        // IWYU pragma: export
#include <csetjmp>           // IWYU pragma: export
#include <shlobj.h>          // IWYU pragma: export
#include <winternl.h>        // IWYU pragma: export
#include <versionhelpers.h>  // IWYU pragma: export
#include <psapi.h>           // IWYU pragma: export
#include <urlmon.h>          // IWYU pragma: export
#include <atlbase.h>         // IWYU pragma: export
#include <atlsafe.h>         // IWYU pragma: export
#include <iphlpapi.h>        // IWYU pragma: export
#include <wincrypt.h>        // IWYU pragma: export
#include <dwmapi.h>          // IWYU pragma: export
#include <shellscalingapi.h> // IWYU pragma: export
#include <d3d11.h>           // IWYU pragma: export
#include <dxgi1_6.h>         // IWYU pragma: export

// min and max is required by gdi, therefore NOMINMAX won't work
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#undef GetObject

#include <cassert> // IWYU pragma: export
#include <cctype>  // IWYU pragma: export
#include <climits> // IWYU pragma: export
#include <cstdint> // IWYU pragma: export
#include <cstring> // IWYU pragma: export

#include <array>         // IWYU pragma: export
#include <atomic>        // IWYU pragma: export
#include <chrono>        // IWYU pragma: export
#include <filesystem>    // IWYU pragma: export
#include <fstream>       // IWYU pragma: export
#include <functional>    // IWYU pragma: export
#include <iostream>      // IWYU pragma: export
#include <limits>        // IWYU pragma: export
#include <map>           // IWYU pragma: export
#include <mutex>         // IWYU pragma: export
#include <optional>      // IWYU pragma: export
#include <queue>         // IWYU pragma: export
#include <random>        // IWYU pragma: export
#include <regex>         // IWYU pragma: export
#include <sstream>       // IWYU pragma: export
#include <thread>        // IWYU pragma: export
#include <unordered_set> // IWYU pragma: export
#include <utility>       // IWYU pragma: export
#include <variant>       // IWYU pragma: export
#include <vector>        // IWYU pragma: export

#include <asmjit/core/jitruntime.h>  // IWYU pragma: export
#include <asmjit/x86/x86assembler.h> // IWYU pragma: export
#include <MinHook.h>                 // IWYU pragma: export
#include <udis86.h>                  // IWYU pragma: export
#include <macros.hpp>                // IWYU pragma: export

#define RAPIDJSON_NOEXCEPT
#define RAPIDJSON_ASSERT(cond)                                                 \
  if (cond)                                                                    \
    ;                                                                          \
  else                                                                         \
    throw std::runtime_error("rapidjson assert fail");

#include <rapidjson/document.h>     // IWYU pragma: export
#include <rapidjson/prettywriter.h> // IWYU pragma: export
#include <rapidjson/stringbuffer.h> // IWYU pragma: export

#pragma warning(pop)
#pragma warning(disable : 4100)

using namespace std::literals;

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")