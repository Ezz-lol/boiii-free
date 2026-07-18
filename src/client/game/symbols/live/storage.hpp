#pragma once

#include <game/symbols/sym_include.hpp>


#include <cstdint>

namespace game {
namespace live {
namespace storage {

WEAK symbol<int32_t(const ControllerIndex_t controllerIndex,
                    uint8_t *transferBuffer, int32_t transferBufferSize)>
    Storage_SerializeTransferData{0x142277710};
} // namespace storage

} // namespace live
} // namespace game

