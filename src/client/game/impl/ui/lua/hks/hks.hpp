#pragma once

#include <cstdint>
#include "../../../../../game/game.hpp"

namespace game {
namespace ui {
namespace lua {
namespace hks {

template <typename Op>
inline HksRegister *operator_outofline(HksRegister *retstr, lua_State *s,
                                       HksObject *left, HksObject *right,
                                       const hksInstruction *PC) {

  if constexpr (std::is_same_v<Op, OpcodeAdd>) {
    return op::OpcodeAdd(retstr, s, left, right, PC);
  } else if constexpr (std::is_same_v<Op, OpcodeBitAnd>) {
    return op::OpcodeBitAnd(retstr, s, left, right, PC);
  } else if constexpr (std::is_same_v<Op, OpcodeBitOr>) {
    return op::OpcodeBitOr(retstr, s, left, right, PC);
  } else if constexpr (std::is_same_v<Op, OpcodeDiv>) {
    return op::OpcodeDiv(retstr, s, left, right, PC);
  } else if constexpr (std::is_same_v<Op, OpcodeLShift>) {
    return op::OpcodeLShift(retstr, s, left, right, PC);
  } else if constexpr (std::is_same_v<Op, OpcodeMod>) {
    return op::OpcodeMod(retstr, s, left, right, PC);
  } else if constexpr (std::is_same_v<Op, OpcodeMul>) {
    return op::OpcodeMul(retstr, s, left, right, PC);
  } else if constexpr (std::is_same_v<Op, OpcodePow>) {
    return op::OpcodePow(retstr, s, left, right, PC);
  } else if constexpr (std::is_same_v<Op, OpcodeRShift>) {
    return op::OpcodeRShift(retstr, s, left, right, PC);
  } else if constexpr (std::is_same_v<Op, OpcodeSub>) {
    return op::OpcodeSub(retstr, s, left, right, PC);
  } else if constexpr (std::is_same_v<Op, OpcodeUnm>) {
    return op::OpcodeUnm(retstr, s, left, right, PC);
  }

  unreachable();
}

luaReturnCount_e executeSharedSecure(lua_State *s, const hksInstruction *PC,
                                     hksInt32 call_depth);
luaReturnCount_e executeShared(lua_State *s, const hksInstruction *PC,
                               hksInt32 call_depth);
luaReturnCount_e executeIsolated(lua_State *s, const hksInstruction *PC,
                                 hksInt32 call_depth);

template <const HksBytecodeSharingMode Mode>
inline luaReturnCount_e execute(lua_State *s, const hksInstruction *PC,
                                hksInt32 call_depth) {
  switch (Mode) {
  case HksBytecodeSharingMode::SECURE:
    return executeSharedSecure(s, PC, call_depth);
  case HksBytecodeSharingMode::ON:
    return executeShared(s, PC, call_depth);
  case HksBytecodeSharingMode::OFF:
    return executeIsolated(s, PC, call_depth);
  default:
    unreachable();
  }
}

} // namespace hks
} // namespace lua
} // namespace ui
} // namespace game