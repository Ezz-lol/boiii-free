#include <std_include.hpp>

#include "sl.hpp"

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
#include <windows.h>
#endif

namespace game {
namespace sl {

uint32_t RefString::dec_ref() volatile {
#if defined(__clang__) || defined(__GNUC__)
  __atomic_sub_fetch(&data, 1, __ATOMIC_SEQ_CST);
#elif defined(_MSC_VER)
  _InterlockedDecrement(reinterpret_cast<long volatile *>(&data));
#else
  // Unsupported, but we have an obvious fallback for this case.
  --refCount;
#endif
  return refCount;
}

uint32_t RefString::inc_ref() volatile {
#if defined(__clang__) || defined(__GNUC__)
  __atomic_add_fetch(&data, 1, __ATOMIC_SEQ_CST);
#elif defined(_MSC_VER)
  _InterlockedIncrement(reinterpret_cast<long volatile *>(&data));
#else
  // Unsupported, but we have an obvious fallback for this case.
  ++refCount;
#endif
  return refCount;
}

} // namespace sl
} // namespace game
