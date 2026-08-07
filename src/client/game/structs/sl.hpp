#pragma once

#include <cstdint>
#include <game/structs/macros.hpp>

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
#include <windows.h>
#endif

namespace game {
namespace sl {
PACKED(struct alignas(4) RefString {
  union {
    struct {
      volatile uint32_t refCount : 16;
      volatile uint32_t user : 8;
      volatile uint32_t byteLen : 8;
    };
    volatile uint32_t data;
  };
  char str[1];
  uint8_t _padding05[3];

  inline uint32_t dec_ref() volatile {
#if defined(__clang__) || defined(__GNUC__)
    __atomic_sub_fetch(&data, 1, __ATOMIC_SEQ_CST);
#elif defined(_MSC_VER)
    _InterlockedDecrement(reinterpret_cast<long volatile*>(&data));
#else
       // Unsupported, but we have an obvious fallback for this case.
      --refCount;
#endif
    return refCount;
  }

  inline uint32_t inc_ref() volatile {
#if defined(__clang__) || defined(__GNUC__)
    __atomic_add_fetch(&data, 1, __ATOMIC_SEQ_CST);
#elif defined(_MSC_VER)
    _InterlockedIncrement(reinterpret_cast<long volatile*>(&data));
#else
       // Unsupported, but we have an obvious fallback for this case.
      ++refCount;
#endif
    return refCount;
  }
});
ASSERT_SIZE(RefString, 0x8);

} // namespace sl
} // namespace game