#pragma once

#include <cstdint>

#include <game/symbol.hpp>

#include <structs/func.hpp>

#include "macros.hpp"

namespace game {
PACKED(class alignas(8) tlAtomicMutex {
public:
  int64_t ThreadId;
  int32_t LockCount;
  uint8_t _padding0C[4];
  volatile tlAtomicMutex *ThisPtr;

  struct syms {
    static constexpr symbol<thiscall_t<void(volatile tlAtomicMutex *)>> Lock{
        0x140009CB0, 0x140009B20};
  };

  void Lock() volatile;

  static void Lock(volatile tlAtomicMutex * self) { return self->Lock(); }
});
ASSERT_SIZE(tlAtomicMutex, 0x18);
} // namespace game
