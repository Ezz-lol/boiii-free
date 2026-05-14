#pragma once

#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace live {
namespace storage {

WEAK symbol<int32_t(const ControllerIndex_t controllerIndex,
                    uint8_t *transferBuffer, int32_t transferBufferSize)>
    Storage_SerializeTransferData{0x142277710};
} // namespace storage

} // namespace live
} // namespace game

#endif