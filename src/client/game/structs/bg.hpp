#pragma once
#include <cstdint>

namespace game {
namespace bg {
enum class asmPrintLevel_t : int32_t {
  LEVEL_INFO = 0x0,
  LEVEL_WARNING = 0x1,
  LEVEL_ERROR = 0x2,
};

enum class AsmDebugLevel : uint32_t {
  GENERAL = 0x0,
  ANIMATOR = 0x1,
  AI_PROGRAMMER = 0x2
};

enum class bgCacheInstance : uint32_t {
  SERVER = 0x0,
  CLIENT = 0x1,
  COUNT = 0x2,
};
} // namespace bg
} // namespace game