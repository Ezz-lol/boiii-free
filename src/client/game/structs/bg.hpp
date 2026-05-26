#pragma once
#include <cstdint>

namespace game {
namespace bg {
enum asmPrintLevel_t : int32_t {
  ASM_PRINT_LEVEL_INFO = 0x0,
  ASM_PRINT_LEVEL_WARNING = 0x1,
  ASM_PRINT_LEVEL_ERROR = 0x2,
};

enum AsmDebugLevel : uint32_t {
  ASM_DEBUG_LEVEL_GENERAL = 0x0,
  ASM_DEBUG_LEVEL_ANIMATOR = 0x1,
  ASM_DEBUG_LEVEL_AI_PROGRAMMER = 0x2
};

enum bgCacheInstance : uint32_t {
  BG_CACHE_INSTANCE_SERVER = 0x0,
  BG_CACHE_INSTANCE_CLIENT = 0x1,
  BG_CACHE_INSTANCE_COUNT = 0x2,
};
} // namespace bg
} // namespace game