#pragma once

#include <cstdint>
#include "func.hpp"

namespace game {

// Has to be considered POD for interop with engine code, otherwise CPP tries to
// pass it as a pointer to a hidden struct type instead of as a 32-bit value,
// which causes dereferencing 0x0 or 0x1 errors
using qboolean = uint32_t;

inline constexpr qboolean qfalse = 0;
inline constexpr qboolean qtrue = 1;

// Type used by engine
typedef cdecl_t<int32_t, const void *, const void *>
    _CoreCrtNonSecureSearchSortCompareFunction;

// Can be used by boiii for more strict, specific argument typing
template <typename T>
using CoreCrtNonSecureSearchSortCompareFunction =
    cdecl_t<int32_t, const T *, const T *>;

// Type used by engine
typedef cdecl_t<int32_t, void *, const void *, const void *>
    _CoreCrtSecureSearchSortCompareFunction;

// Can be used by boiii for more strict, specific argument typing
template <typename T>
using CoreCrtSecureSearchSortCompareFunction =
    cdecl_t<int32_t, T *, const T *, const T *>;
} // namespace game