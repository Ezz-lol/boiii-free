#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {

namespace jq {
WEAK symbol<void(jqBatch *batch, char *event)> jqCallbackPre{0x0, 0x1405711A0};
WEAK symbol<void(jqBatch *batch, void *data, bool finished)> jqCallbackPost{
    0x0, 0x140571140};
WEAK symbol<void(void *data)> jqFreeBatchData{
    0x0, 0x14000A350}; // arg usually passed as batch->p3x_info
} // namespace jq
} // namespace game

#endif