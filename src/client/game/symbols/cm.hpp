#pragma once

#include <game/symbols/sym_include.hpp>


namespace game {
namespace cm {

WEAK symbol<int32_t(const traceWork_t *tw, int32_t clipmapNodeNum,
                    const vec3_t *p1_, const vec3_t *p2, trace_t *trace)>
    CM_SightTraceThroughTree{0x1420E14E0};

WEAK symbol<clipMap_t *> cm{0x156817F88};
} // namespace cm
} // namespace game

