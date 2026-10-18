#pragma once

#include <game/structs/core.hpp>

namespace game {
namespace dw {
namespace thread {

class __declspec(align(8)) bdRunnable {
public:
  // TODO
  struct VTbl {};
  VTbl *vtbl;
  bdBool m_stop;
};

typedef void *bdThreadHandle;

class bdThread;

class bdThreadArgs {
public:
  void *m_args;
  bdRunnable *m_runnable;
  bdThread *m_thread;
};

class bdThread {
public:
  bdRunnable *m_runnable;
  bdThreadArgs m_threadArgs;
  bdThreadHandle m_handle;
  bdBool m_isRunning;
  bdUInt m_stackSize;
  bdUInt m_priority;
  bdInt m_coreAffinity;
};

} // namespace thread
} // namespace dw
} // namespace game
