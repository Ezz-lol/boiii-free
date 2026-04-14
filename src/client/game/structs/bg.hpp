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
} // namespace bg
} // namespace game