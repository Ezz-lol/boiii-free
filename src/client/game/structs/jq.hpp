#pragma once

#include <cstdint>
namespace jq {
#pragma pack(push, 1)

struct jqBatch;
typedef int32_t (*jqModuleCallback)(jqBatch *);

struct jqBatchGroup {
  union {
    struct {
      int32_t QueuedBatchCount;
      int32_t ExecutingBatchCount;
    };
    uint64_t BatchCount;
  };
};

struct jqModule {
  const char *Name;
  jqModuleCallback Code;
  jqBatchGroup Group;
  uint64_t Ticks;
  uint16_t Calls;
  uint16_t Requeues;
  uint8_t _padding24[4];
  jqBatchGroup *Dependencies[4];
};

struct jqBatch {
  void *p3x_info;
  void *Input;
  void *Output;
  jqModule *Module;
  jqBatchGroup *GroupID;
  void *ConditionalAddress;
  uint32_t ConditionalValue;
  uint32_t ParamData[55];
};

#pragma pack(pop)
} // namespace jq