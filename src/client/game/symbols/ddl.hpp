#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace ddl {

WEAK symbol<bool(const DDLState *, DDLState *, const char *)> DDL_MoveToName{
    0x1424A92F0, 0x142522460, 0x0};
WEAK symbol<bool(const DDLState *, DDLState *, int32_t)> DDL_MoveToIndex{
    0x1424A92E0, 0x142522450, 0x0};
WEAK symbol<uint32_t(const DDLState *, const DDLContext *)> DDL_GetUInt{
    0x1424A8DC0, 0x142521F30, 0x0};
WEAK symbol<bool(const DDLState *, DDLContext *, uint32_t)> DDL_SetUInt{
    0x1424A9800, 0x142522970, 0x0};

WEAK symbol<bool(void *buff, int32_t len, const DDLDef *ddlDef,
                 DDLContext *const ddlContext, DDLWriteCB writeCB,
                 void *userData)>
    DDL_Buffer_ResetContext{0x1424A9DF0, 0x142522F60, 0x1406196F0};

} // namespace ddl
} // namespace game
