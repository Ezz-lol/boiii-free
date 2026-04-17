#pragma once

#include <cstdint>
#include <type_traits>

namespace game {

// Has to be considered POD for interop with engine code, otherwise CPP tries to
// pass it as a pointer to a hidden struct type instead of as a 32-bit value,
// which causes dereferencing 0x0 or 0x1 errors
using qboolean = uint32_t;

inline constexpr qboolean qfalse = 0;
inline constexpr qboolean qtrue = 1;

typedef int(__cdecl *_CoreCrtNonSecureSearchSortCompareFunction)(const void *,
                                                                 const void *);

typedef int(__cdecl *_CoreCrtSecureSearchSortCompareFunction)(void *,
                                                              const void *,
                                                              const void *);
} // namespace game