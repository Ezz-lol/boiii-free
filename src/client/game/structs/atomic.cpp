#include <std_include.hpp>

#include "atomic.hpp"

namespace game {
void tlAtomicMutex::Lock() volatile {
  // PATCH: ensure `ThisPtr` non-null
  if (!this->ThisPtr) {
    this->ThisPtr = this;
  }

  uint32_t thread_id = GetCurrentThreadId();
  if (this->ThreadId == thread_id) {
    this->LockCount += 1;
  } else {
    if (_InterlockedCompareExchange64(&this->ThisPtr->ThreadId, thread_id, 0) !=
        0) {
      while (_InterlockedCompareExchange64(&this->ThisPtr->ThreadId, thread_id,
                                           0) != 0) {
        Sleep(0);
      }
    }
    long _fence = 0;
    _InterlockedExchange(&_fence, 0);
    this->LockCount = 1;
  }
}
} // namespace game
