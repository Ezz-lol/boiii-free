#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace ddl {

WEAK symbol<bool(void *buff, int32_t len, const DDLDef *ddlDef,
                 DDLContext *const ddlContext, DDLWriteCB writeCB,
                 void *userData)>
    DDL_Buffer_ResetContext{0x142522F60, 0x1406196F0};

} // namespace ddl
} // namespace game