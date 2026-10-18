#pragma once

#include <cstdint>

#include <structs/func.hpp>

namespace game {
namespace task {

typedef uint64_t TaskCategory;

enum class LocalTaskState : uint32_t {
  INPROGRESS = 0x0,
  COMPLETED = 0x1,
  FAILED = 0x2,
};

struct TaskRecord;

typedef fastcall_t<LocalTaskState(TaskRecord *task)> localTaskfunc;
typedef fastcall_t<bool(TaskRecord *task)> task_callback;

struct TaskDefinition {
  TaskCategory category;
  const char *name;
  int32_t payloadSize;
  task_callback *completed_callback;
  task_callback *failure_callback;
  localTaskfunc *localTask;
};

} // namespace task
} // namespace game
