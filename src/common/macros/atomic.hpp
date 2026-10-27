#pragma once

#include <type_traits>

#include "macros/builtin.hpp"
#include "meta.hpp"

#if !defined(GNUC_OR_CLANG) && defined(MSC)
#include <intrin.h>
#endif

template <typename T>
concept UnderlyingAtomicIntegral =
    std::is_integral_v<T> && (sizeof(T) <= sizeof(size_t));

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_load(volatile T *ptr) {
#if defined(GNUC_OR_CLANG)
  return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
#else
  return *ptr;
#endif
}

template <UnderlyingAtomicIntegral T>
alwaysinline void atomic_store(volatile T *ptr, T desired) {
#if defined(GNUC_OR_CLANG)
  __atomic_store_n(ptr, desired, __ATOMIC_SEQ_CST);
#elif defined(MSC)
  if constexpr (sizeof(T) == 1) {
    _InterlockedExchange8(reinterpret_cast<char volatile *>(ptr),
                          static_cast<char>(desired));
  } else if constexpr (sizeof(T) == 2) {
    _InterlockedExchange16(reinterpret_cast<short volatile *>(ptr),
                           static_cast<short>(desired));
  } else if constexpr (sizeof(T) == 4) {
    _InterlockedExchange(reinterpret_cast<long volatile *>(ptr),
                         static_cast<long>(desired));
  } else if constexpr (sizeof(T) == 8) {
    _InterlockedExchange64(reinterpret_cast<__int64 volatile *>(ptr),
                           static_cast<__int64>(desired));
  }
#else
  *ptr = desired;
#endif
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_inc(volatile T *ptr) {
  constexpr T ONE = 1;
#if defined(GNUC_OR_CLANG)
  return __atomic_add_fetch(ptr, ONE, __ATOMIC_SEQ_CST);
#elif defined(MSC)
  if constexpr (sizeof(T) == 1) {
    return static_cast<T>(
        _InterlockedExchangeAdd8(reinterpret_cast<char volatile *>(ptr), 1) +
        1);
  } else if constexpr (sizeof(T) == 2) {
    return static_cast<T>(
        _InterlockedIncrement16(reinterpret_cast<short volatile *>(ptr)));
  } else if constexpr (sizeof(T) == 4) {
    return static_cast<T>(
        _InterlockedIncrement(reinterpret_cast<long volatile *>(ptr)));
  } else if constexpr (sizeof(T) == 8) {
    return static_cast<T>(
        _InterlockedIncrement64(reinterpret_cast<__int64 volatile *>(ptr)));
  }
#else
  return ++(*ptr);
#endif
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_dec(volatile T *ptr) {
  constexpr T ONE = 1;
#if defined(GNUC_OR_CLANG)
  return __atomic_sub_fetch(ptr, ONE, __ATOMIC_SEQ_CST);
#elif defined(MSC)
  if constexpr (sizeof(T) == 1) {
    return static_cast<T>(
        _InterlockedExchangeAdd8(reinterpret_cast<char volatile *>(ptr), -1) -
        1);
  } else if constexpr (sizeof(T) == 2) {
    return static_cast<T>(
        _InterlockedDecrement16(reinterpret_cast<short volatile *>(ptr)));
  } else if constexpr (sizeof(T) == 4) {
    return static_cast<T>(
        _InterlockedDecrement(reinterpret_cast<long volatile *>(ptr)));
  } else if constexpr (sizeof(T) == 8) {
    return static_cast<T>(
        _InterlockedDecrement64(reinterpret_cast<__int64 volatile *>(ptr)));
  }
#else
  return --(*ptr);
#endif
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_add(volatile T *ptr, T value) {
#if defined(GNUC_OR_CLANG)
  return __atomic_add_fetch(ptr, value, __ATOMIC_SEQ_CST);
#elif defined(MSC)
  if constexpr (sizeof(T) == 1) {
    return static_cast<T>(
        _InterlockedExchangeAdd8(reinterpret_cast<char volatile *>(ptr),
                                 static_cast<char>(value)) +
        static_cast<char>(value));
  } else if constexpr (sizeof(T) == 2) {
    return static_cast<T>(
        _InterlockedExchangeAdd16(reinterpret_cast<short volatile *>(ptr),
                                  static_cast<short>(value)) +
        static_cast<short>(value));
  } else if constexpr (sizeof(T) == 4) {
    return static_cast<T>(
        _InterlockedExchangeAdd(reinterpret_cast<long volatile *>(ptr),
                                static_cast<long>(value)) +
        static_cast<long>(value));
  } else if constexpr (sizeof(T) == 8) {
    return static_cast<T>(
        _InterlockedExchangeAdd64(reinterpret_cast<__int64 volatile *>(ptr),
                                  static_cast<__int64>(value)) +
        static_cast<__int64>(value));
  }
#else
  return (*ptr += value);
#endif
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_sub(volatile T *ptr, T value) {
#if defined(GNUC_OR_CLANG)
  return __atomic_sub_fetch(ptr, value, __ATOMIC_SEQ_CST);
#elif defined(MSC)
  if constexpr (sizeof(T) == 1) {
    return static_cast<T>(
        _InterlockedExchangeAdd8(reinterpret_cast<char volatile *>(ptr),
                                 -static_cast<char>(value)) -
        static_cast<char>(value));
  } else if constexpr (sizeof(T) == 2) {
    return static_cast<T>(
        _InterlockedExchangeAdd16(reinterpret_cast<short volatile *>(ptr),
                                  -static_cast<short>(value)) -
        static_cast<short>(value));
  } else if constexpr (sizeof(T) == 4) {
    return static_cast<T>(
        _InterlockedExchangeAdd(reinterpret_cast<long volatile *>(ptr),
                                -static_cast<long>(value)) -
        static_cast<long>(value));
  } else if constexpr (sizeof(T) == 8) {
    return static_cast<T>(
        _InterlockedExchangeAdd64(reinterpret_cast<__int64 volatile *>(ptr),
                                  -static_cast<__int64>(value)) -
        static_cast<__int64>(value));
  }
#else
  return (*ptr -= value);
#endif
}

template <UnderlyingAtomicIntegral T>
alwaysinline bool atomic_compare_exchange(volatile T *ptr, T *expected,
                                          T desired) {
#if defined(GNUC_OR_CLANG)
  return __atomic_compare_exchange_n(ptr, expected, desired, false,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#elif defined(MSC)
  T cmp = *expected;
  T prev = 0;
  if constexpr (sizeof(T) == 1) {
    prev = static_cast<T>(_InterlockedCompareExchange8(
        reinterpret_cast<char volatile *>(ptr), static_cast<char>(desired),
        static_cast<char>(cmp)));
  } else if constexpr (sizeof(T) == 2) {
    prev = static_cast<T>(_InterlockedCompareExchange16(
        reinterpret_cast<short volatile *>(ptr), static_cast<short>(desired),
        static_cast<short>(cmp)));
  } else if constexpr (sizeof(T) == 4) {
    prev = static_cast<T>(_InterlockedCompareExchange(
        reinterpret_cast<long volatile *>(ptr), static_cast<long>(desired),
        static_cast<long>(cmp)));
  } else if constexpr (sizeof(T) == 8) {
    prev = static_cast<T>(_InterlockedCompareExchange64(
        reinterpret_cast<__int64 volatile *>(ptr),
        static_cast<__int64>(desired), static_cast<__int64>(cmp)));
  }
  if (prev == cmp) {
    return true;
  } else {
    *expected = prev;
    return false;
  }
#else
  if (*ptr == *expected) {
    *ptr = desired;
    return true;
  } else {
    *expected = *ptr;
    return false;
  }
#endif
}
template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_and(volatile T *ptr, T value) {
#if defined(GNUC_OR_CLANG)
  return __atomic_and_fetch(ptr, value, __ATOMIC_SEQ_CST);
#elif defined(MSC)
  if constexpr (sizeof(T) == 1) {
    char old = _InterlockedAnd8(reinterpret_cast<char volatile *>(ptr),
                                static_cast<char>(value));
    return static_cast<T>(old & static_cast<char>(value));
  } else if constexpr (sizeof(T) == 2) {
    short old = _InterlockedAnd16(reinterpret_cast<short volatile *>(ptr),
                                  static_cast<short>(value));
    return static_cast<T>(old & static_cast<short>(value));
  } else if constexpr (sizeof(T) == 4) {
    long old = _InterlockedAnd(reinterpret_cast<long volatile *>(ptr),
                               static_cast<long>(value));
    return static_cast<T>(old & static_cast<long>(value));
  } else if constexpr (sizeof(T) == 8) {
    __int64 old = _InterlockedAnd64(reinterpret_cast<__int64 volatile *>(ptr),
                                    static_cast<__int64>(value));
    return static_cast<T>(old & static_cast<__int64>(value));
  }
#else
  return (*ptr &= value);
#endif
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_or(volatile T *ptr, T value) {
#if defined(GNUC_OR_CLANG)
  return __atomic_or_fetch(ptr, value, __ATOMIC_SEQ_CST);
#elif defined(MSC)
  if constexpr (sizeof(T) == 1) {
    char old = _InterlockedOr8(reinterpret_cast<char volatile *>(ptr),
                               static_cast<char>(value));
    return static_cast<T>(old | static_cast<char>(value));
  } else if constexpr (sizeof(T) == 2) {
    short old = _InterlockedOr16(reinterpret_cast<short volatile *>(ptr),
                                 static_cast<short>(value));
    return static_cast<T>(old | static_cast<short>(value));
  } else if constexpr (sizeof(T) == 4) {
    long old = _InterlockedOr(reinterpret_cast<long volatile *>(ptr),
                              static_cast<long>(value));
    return static_cast<T>(old | static_cast<long>(value));
  } else if constexpr (sizeof(T) == 8) {
    __int64 old = _InterlockedOr64(reinterpret_cast<__int64 volatile *>(ptr),
                                   static_cast<__int64>(value));
    return static_cast<T>(old | static_cast<__int64>(value));
  }
#else
  return (*ptr |= value);
#endif
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_xor(volatile T *ptr, T value) {
#if defined(GNUC_OR_CLANG)
  return __atomic_xor_fetch(ptr, value, __ATOMIC_SEQ_CST);
#elif defined(MSC)
  if constexpr (sizeof(T) == 1) {
    char old = _InterlockedXor8(reinterpret_cast<char volatile *>(ptr),
                                static_cast<char>(value));
    return static_cast<T>(old ^ static_cast<char>(value));
  } else if constexpr (sizeof(T) == 2) {
    short old = _InterlockedXor16(reinterpret_cast<short volatile *>(ptr),
                                  static_cast<short>(value));
    return static_cast<T>(old ^ static_cast<short>(value));
  } else if constexpr (sizeof(T) == 4) {
    long old = _InterlockedXor(reinterpret_cast<long volatile *>(ptr),
                               static_cast<long>(value));
    return static_cast<T>(old ^ static_cast<long>(value));
  } else if constexpr (sizeof(T) == 8) {
    __int64 old = _InterlockedXor64(reinterpret_cast<__int64 volatile *>(ptr),
                                    static_cast<__int64>(value));
    return static_cast<T>(old ^ static_cast<__int64>(value));
  }
#else
  return (*ptr ^= value);
#endif
}

alwaysinline bool atomic_toggle(volatile bool *ptr) {
#if defined(GNUC_OR_CLANG)
  return __atomic_xor_fetch(ptr, true, __ATOMIC_SEQ_CST);
#elif defined(MSC)
  // _InterlockedXor8 returns the old value; XOR with 1 and invert to get the
  // new boolean value.
  return static_cast<bool>(
      _InterlockedXor8(reinterpret_cast<char volatile *>(ptr), 1) ^ 1);
#else
  return (*ptr = !*ptr);
#endif
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_load(volatile T &ptr) {
  return atomic_load(&ptr);
}

template <UnderlyingAtomicIntegral T>
alwaysinline void atomic_store(volatile T &ptr, T desired) {
  return atomic_store(&ptr, desired);
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_inc(volatile T &ptr) {
  return atomic_inc(&ptr);
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_dec(volatile T &ptr) {
  return atomic_dec(&ptr);
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_add(volatile T &ptr, T value) {
  return atomic_add(&ptr, value);
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_sub(volatile T &ptr, T value) {
  return atomic_sub(&ptr, value);
}

template <UnderlyingAtomicIntegral T>
alwaysinline bool atomic_compare_exchange(volatile T &ptr, T &expected,
                                          T desired) {
  return atomic_compare_exchange(&ptr, &expected, desired);
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_xor(volatile T &ptr, T value) {
  return atomic_xor(&ptr, value);
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_or(volatile T &ptr, T value) {
  return atomic_or(&ptr, value);
}

template <UnderlyingAtomicIntegral T>
alwaysinline T atomic_and(volatile T &ptr, T value) {
  return atomic_and(&ptr, value);
}

alwaysinline bool atomic_toggle(volatile bool &ptr) {
  return atomic_toggle(&ptr);
}
