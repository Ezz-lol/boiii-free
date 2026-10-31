#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace dw {
namespace task {
namespace fs {
WEAK symbol<TaskRecord *(ControllerIndex_t localControllerIndex,
                         dwFileTask *fileTask, uint8_t *buffer,
                         int32_t bufferSize)>
    dwWriteAndValidateFile{0x141444490, 0x0, 0x0};
WEAK symbol<TaskRecord *(ControllerIndex_t localControllerIndex,
                         dwFileTask *fileTask, uint8_t *buffer,
                         int32_t bufferSize)>
    dwWriteAndValidateFileInternal{0x1414444A0, 0x0, 0x0};
} // namespace fs
} // namespace task
} // namespace dw
} // namespace game
