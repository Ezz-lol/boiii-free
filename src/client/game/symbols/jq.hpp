#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

namespace jq {
WEAK symbol<void(jqBatch *batch, char *event)> jqCallbackPre{0x1422B4BA0,
                                                             0x1405711A0};
WEAK symbol<void(jqBatch *batch, void *data, bool finished)> jqCallbackPost{
    0x1422B4B60, 0x140571140};
WEAK symbol<void(void *data)> jqFreeBatchData{
    0x0, 0x14000A350}; // arg usually passed as batch->p3x_info

WEAK symbol<jqBatchPool> jqPool{0x143674810, 0x14137CF10};
} // namespace jq
} // namespace game