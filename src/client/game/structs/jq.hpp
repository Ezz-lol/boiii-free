#pragma once

#include "game/structs/core.hpp"
#include "game/symbol.hpp"
#include <cstdint>

#include <structs/func.hpp>
#include <game/structs/macros.hpp>

namespace game {
namespace jq {

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

struct jqBatch;
typedef fastcallPtr_t<int32_t(jqBatch *batch)> jqModuleCallback;

PACKED(struct jqBatchGroup {
  union {
    struct {
      int32_t QueuedBatchCount;
      int32_t ExecutingBatchCount;
    };
    uint64_t BatchCount;
  };
});

PACKED(struct jqModule {
  const char *Name;
  jqModuleCallback Code;
  jqBatchGroup Group;
  uint64_t Ticks;
  uint16_t Calls;
  uint16_t Requeues;
  uint8_t _padding24[4];
  jqBatchGroup *Dependencies[4];
});

PACKED(struct jqBatch {
  void *p3x_info;
  void *Input;
  void *Output;
  jqModule *Module;
  jqBatchGroup *GroupID;
  void *ConditionalAddress;
  uint32_t ConditionalValue;
  uint32_t ParamData[55];
});

template <typename NodeType> class jqAtomicQueuePtr {
  union {
    struct {
      volatile uint32_t Index;
      volatile uint32_t Count;
    };
    volatile u64 Val64;
  };
};

template <typename Batch> class jqAtomicQueue {
public:
  struct NodeType {
    jqAtomicQueuePtr<NodeType> Next;
    jqBatch Data;
  };
  typedef jqAtomicQueuePtr<NodeType> NodePtrType;

  NodeType *NodeStorage;
  NodePtrType Head;
  NodePtrType Tail;
  NodePtrType FreeList;
};

typedef jqAtomicQueue<jqBatch> jqAtomicQueueType;

struct jqQueue {
  jqAtomicQueueType Queue;
  int32_t QueuedBatchCount;
  u32 ProcessorsMask;
};

class jqAtomicHeap;
class jqAtomicHeap {
public:
  typedef u64 CellType;

  struct LevelInfo {
    u32 BlockSize;
    int32_t NBlocks;
    int32_t NCells;
    jqAtomicHeap::CellType *CellAvailable;
    jqAtomicHeap::CellType *CellAllocated;
  };

  jqAtomicHeap *ThisPtr;
  tlAtomicMutex Mutex;
  u8 *HeapBase;
  u32 HeapSize;
  u32 BlockSize;
  volatile u32 TotalUsed;
  volatile u32 TotalBlocks;
  int32_t NLevels;
  jqAtomicHeap::LevelInfo Levels[13];
  u8 *LevelData;

  struct syms {
    static constexpr symbol<
        thiscall_t<void(volatile jqAtomicHeap *, void *Ptr)>>
        Free{0x140009920, 0x140009790};
  };

  inline void Free(void *Ptr) volatile { syms::Free(this, Ptr); }
};

struct jqBatchPool {
  jqQueue BaseQueue;
  union {
    struct {
      int32_t QueuedBatchCount;
      int32_t ExecutingBatchCount;
    };
    u64 BatchCount;
  };
  jqAtomicHeap BatchDataHeap;
};

} // namespace jq
} // namespace game