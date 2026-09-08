#pragma once

/*
  This header is separated from core.hpp to ensure we do not need to navigate
  around these massive compile-time opcode mappings while reviewing or defining
  other data structures.
*/

#include "game/structs/macros.hpp"
#include <frozen/unordered_map.h>

namespace game {
namespace scr {
namespace vm {
namespace op {

typedef uint16_t OP_TYPE;

/*
  Logical map of unique opcode types.

  These do not correspond to bytecode value or jump table order.

  The jump table mapping in the T7 engine is highly irregular, even across all
  other CoD engines. Opcodes are ordered seemingly randomly across and within
  the jump tables, and most are repeated many times - there are 0x4000 combined
  entries in the two jump tables, but < 0xFF unique opcodes.

  As such, this mapping serves to differentiate known opcodes by functionality,
  uniquely, for use in:
  - the GSC compiler opcode emitter
  - The opcode -> corresponding bytecode value(s) mapping, below
*/
enum class Opcode : uint8_t {
  Bit_And = 0x00,
  Bit_Or = 0x01,
  Bit_Xor = 0x02,
  /*
   Note: `BoolComplement` is the name
   of this opcode in the engine, but
   it is actually a 64-bit Bitwise NOT
   operation that can only be used with
   an `INT`-typed `ScrVar_t`.
  */
  BoolComplement = 0x03,
  BoolNot = 0x04,
  CallBuiltin = 0x05,
  CallBuiltinMethod = 0x06,
  CastBool = 0x07,
  CastFieldObject = 0x08,
  CheckClearParams = 0x09,
  ClearArray = 0x0A,
  ClearFieldVariable = 0x0B,
  ClearParams = 0x0C,
  Dec = 0x0D,
  DecTop = 0x0E,
  Divide = 0x0F,
  End = 0x10,
  EndOn = 0x11,
  EndSwitch = 0x12,
  Equal = 0x13,
  EvalArray = 0x14,
  EvalArrayRef = 0x15,
  EvalFieldVariable = 0x16,
  EvalFieldVariableRef = 0x17,
  EvalLocalVariableCached = 0x18,
  EvalLocalVariableCachedDebug = 0x19,
  EvalLocalVariableRefCached = 0x1A,
  EvalLocalVariableRefCachedDebug = 0x1B,
  FirstArrayKey = 0x1C,
  GetAnim = 0x1D,
  GetAnimation = 0x1E,
  GetAnimObject = 0x1F,
  GetAPIFunction = 0x20,
  GetByte = 0x21,
  GetClasses = 0x22,
  GetClassesObject = 0x23,
  GetEmptyArray = 0x24,
  GetFloat = 0x25,
  GetFunction = 0x26,
  GetGame = 0x27,
  GetGameRef = 0x28,
  GetHash = 0x29,
  GetInteger = 0x2A,
  GetIString = 0x2B,
  GetLevel = 0x2C,
  GetLevelObject = 0x2D,
  GetNegByte = 0x2E,
  GetNegUnsignedShort = 0x2F,
  GetSelf = 0x30,
  GetSelfObject = 0x31,
  GetString = 0x32,
  GetTime = 0x33,
  GetUintptr = 0x34,
  GetUndefined = 0x35,
  GetUnsignedShort = 0x36,
  GetVector = 0x37,
  GetZero = 0x38,
  GreaterThan = 0x39,
  GreaterThanOrEqualTo = 0x3A,
  Inc = 0x3B,
  IsDefined = 0x3C,
  Jump = 0x3D,
  JumpOnFalse = 0x3E,
  JumpOnFalseExpr = 0x3F,
  JumpOnTrue = 0x40,
  JumpOnTrueExpr = 0x41,
  LessThan = 0x42,
  LessThanOrEqualTo = 0x43,
  Minus = 0x44,
  Modulus = 0x45,
  Multiply = 0x46,
  New = 0x47,
  NextArrayKey = 0x48,
  NotEqual = 0x49,
  Notify = 0x4A,
  Plus = 0x4B,
  PreScriptCall = 0x4C,
  ProfileStart = 0x4D,
  ProfileStop = 0x4E,
  Return = 0x4F,
  SafeCreateLocalVariables = 0x50,
  SafeDecTop = 0x51,
  SafeSetVariableFieldCached = 0x52,
  SafeSetWaittillVariableFieldCached = 0x53,
  ScriptFunctionCall = 0x54,
  ScriptFunctionCallClass = 0x55,
  ScriptFunctionCallPointer = 0x56,
  ScriptMethodCall = 0x57,
  ScriptMethodCallPointer = 0x58,
  ScriptMethodThreadCall = 0x59,
  ScriptMethodThreadCallPointer = 0x5A,
  ScriptThreadCall = 0x5B,
  ScriptThreadCallClass = 0x5C,
  ScriptThreadCallPointer = 0x5D,
  SetVariableField = 0x5E,
  ShiftLeft = 0x5F,
  ShiftRight = 0x60,
  SizeOf = 0x61,
  Switch = 0x62,
  Vector = 0x63,
  VectorConstant = 0x64,
  VectorScale = 0x65,
  Wait = 0x66,
  WaitTill = 0x67,
  WaitTillFrameEnd = 0x68,
  WaitTillMatch = 0x69,

  // Extended opcodes (T7 PC)
  DevblockBegin = 0x6A,
  EvalLevelFieldVariable = 0x6B,
  EvalLevelFieldVariableRef = 0x6C,
  EvalSelfFieldVariable = 0x6D,
  EvalSelfFieldVariableRef = 0x6E,
  GetWorld = 0x6F,
  GetWorldObject = 0x70,
  /*
     Same as `Equal`, but also ensures that the gives operands have comparable
     types - either the same type, or int and float.

     Note: BO4 expanded this to also allow comparison of a `HASH`-typed
     `ScrVar_t` and a `STRING`-typed `ScrVar_t`, but this is not permitted in
     the latest BO3 engine's `ScrVar_EvalEquality`'s implementation
  */
  SuperEqual = 0x71,
  // Same as `NotEqual`, with the additional operand type requirements seen in
  // `SuperEqual`.
  SuperNotEqual = 0x72,
  WaitRealTime = 0x73,

  /*
    This is an all-emcompassing enumeration for opcodes which have
    stubbed handlers in the engine - their handler functions immediately return.

    Some of these are simply unused or invalid functions.

    Others are special functions which are conditionally compiled to perform
    functionality in specific build profiles - profiling or debug builds,
    generally.

    In any case, for most of these functions, we do not have access to the
    conditionally compiled, complete contents in the latest bytecode revision
    (0x1C) - we cannot reverse engineer the operation executed by the function.
    Thus, each is instead labelled with a generic `StubbedUnknownOrInvalid`
    enumeration.

    This should be inconsequential - none of these opcodes are useful in release
    builds.
  */
  UnknownOrInvalid = 0xFF,

  Count = 0x74,
};
IMPL_ENUM_OPERATORS(Opcode);

// For debug logging
#ifndef NDEBUG
inline constexpr const char *serialize(vm::op::Opcode opcode) {
  switch (opcode) {
  case vm::op::Opcode::Bit_And:
    return "OP_Bit_And";
  case vm::op::Opcode::Bit_Or:
    return "OP_Bit_Or";
  case vm::op::Opcode::Bit_Xor:
    return "OP_Bit_Xor";
  case vm::op::Opcode::BoolComplement:
    return "OP_BoolComplement";
  case vm::op::Opcode::BoolNot:
    return "OP_BoolNot";
  case vm::op::Opcode::CallBuiltin:
    return "OP_CallBuiltin";
  case vm::op::Opcode::CallBuiltinMethod:
    return "OP_CallBuiltinMethod";
  case vm::op::Opcode::CastBool:
    return "OP_CastBool";
  case vm::op::Opcode::CastFieldObject:
    return "OP_CastFieldObject";
  case vm::op::Opcode::CheckClearParams:
    return "OP_CheckClearParams";
  case vm::op::Opcode::ClearArray:
    return "OP_ClearArray";
  case vm::op::Opcode::ClearFieldVariable:
    return "OP_ClearFieldVariable";
  case vm::op::Opcode::ClearParams:
    return "OP_ClearParams";
  case vm::op::Opcode::Dec:
    return "OP_Dec";
  case vm::op::Opcode::DecTop:
    return "OP_DecTop";
  case vm::op::Opcode::Divide:
    return "OP_Divide";
  case vm::op::Opcode::End:
    return "OP_End";
  case vm::op::Opcode::EndOn:
    return "OP_EndOn";
  case vm::op::Opcode::EndSwitch:
    return "OP_EndSwitch";
  case vm::op::Opcode::Equal:
    return "OP_Equal";
  case vm::op::Opcode::EvalArray:
    return "OP_EvalArray";
  case vm::op::Opcode::EvalArrayRef:
    return "OP_EvalArrayRef";
  case vm::op::Opcode::EvalFieldVariable:
    return "OP_EvalFieldVariable";
  case vm::op::Opcode::EvalFieldVariableRef:
    return "OP_EvalFieldVariableRef";
  case vm::op::Opcode::EvalLocalVariableCached:
    return "OP_EvalLocalVariableCached";
  case vm::op::Opcode::EvalLocalVariableCachedDebug:
    return "OP_EvalLocalVariableCachedDebug";
  case vm::op::Opcode::EvalLocalVariableRefCached:
    return "OP_EvalLocalVariableRefCached";
  case vm::op::Opcode::EvalLocalVariableRefCachedDebug:
    return "OP_EvalLocalVariableRefCachedDebug";
  case vm::op::Opcode::FirstArrayKey:
    return "OP_FirstArrayKey";
  case vm::op::Opcode::GetAnim:
    return "OP_GetAnim";
  case vm::op::Opcode::GetAnimation:
    return "OP_GetAnimation";
  case vm::op::Opcode::GetAnimObject:
    return "OP_GetAnimObject";
  case vm::op::Opcode::GetAPIFunction:
    return "OP_GetAPIFunction";
  case vm::op::Opcode::GetByte:
    return "OP_GetByte";
  case vm::op::Opcode::GetClasses:
    return "OP_GetClasses";
  case vm::op::Opcode::GetClassesObject:
    return "OP_GetClassesObject";
  case vm::op::Opcode::GetEmptyArray:
    return "OP_GetEmptyArray";
  case vm::op::Opcode::GetFloat:
    return "OP_GetFloat";
  case vm::op::Opcode::GetFunction:
    return "OP_GetFunction";
  case vm::op::Opcode::GetGame:
    return "OP_GetGame";
  case vm::op::Opcode::GetGameRef:
    return "OP_GetGameRef";
  case vm::op::Opcode::GetHash:
    return "OP_GetHash";
  case vm::op::Opcode::GetInteger:
    return "OP_GetInteger";
  case vm::op::Opcode::GetIString:
    return "OP_GetIString";
  case vm::op::Opcode::GetLevel:
    return "OP_GetLevel";
  case vm::op::Opcode::GetLevelObject:
    return "OP_GetLevelObject";
  case vm::op::Opcode::GetNegByte:
    return "OP_GetNegByte";
  case vm::op::Opcode::GetNegUnsignedShort:
    return "OP_GetNegUnsignedShort";
  case vm::op::Opcode::GetSelf:
    return "OP_GetSelf";
  case vm::op::Opcode::GetSelfObject:
    return "OP_GetSelfObject";
  case vm::op::Opcode::GetString:
    return "OP_GetString";
  case vm::op::Opcode::GetTime:
    return "OP_GetTime";
  case vm::op::Opcode::GetUintptr:
    return "OP_GetUintptr";
  case vm::op::Opcode::GetUndefined:
    return "OP_GetUndefined";
  case vm::op::Opcode::GetUnsignedShort:
    return "OP_GetUnsignedShort";
  case vm::op::Opcode::GetVector:
    return "OP_GetVector";
  case vm::op::Opcode::GetZero:
    return "OP_GetZero";
  case vm::op::Opcode::GreaterThan:
    return "OP_GreaterThan";
  case vm::op::Opcode::GreaterThanOrEqualTo:
    return "OP_GreaterThanOrEqualTo";
  case vm::op::Opcode::Inc:
    return "OP_Inc";
  case vm::op::Opcode::IsDefined:
    return "OP_IsDefined";
  case vm::op::Opcode::Jump:
    return "OP_Jump";
  case vm::op::Opcode::JumpOnFalse:
    return "OP_JumpOnFalse";
  case vm::op::Opcode::JumpOnFalseExpr:
    return "OP_JumpOnFalseExpr";
  case vm::op::Opcode::JumpOnTrue:
    return "OP_JumpOnTrue";
  case vm::op::Opcode::JumpOnTrueExpr:
    return "OP_JumpOnTrueExpr";
  case vm::op::Opcode::LessThan:
    return "OP_LessThan";
  case vm::op::Opcode::LessThanOrEqualTo:
    return "OP_LessThanOrEqualTo";
  case vm::op::Opcode::Minus:
    return "OP_Minus";
  case vm::op::Opcode::Modulus:
    return "OP_Modulus";
  case vm::op::Opcode::Multiply:
    return "OP_Multiply";
  case vm::op::Opcode::New:
    return "OP_New";
  case vm::op::Opcode::NextArrayKey:
    return "OP_NextArrayKey";
  case vm::op::Opcode::NotEqual:
    return "OP_NotEqual";
  case vm::op::Opcode::Notify:
    return "OP_Notify";
  case vm::op::Opcode::Plus:
    return "OP_Plus";
  case vm::op::Opcode::PreScriptCall:
    return "OP_PreScriptCall";
  case vm::op::Opcode::ProfileStart:
    return "OP_ProfileStart";
  case vm::op::Opcode::ProfileStop:
    return "OP_ProfileStop";
  case vm::op::Opcode::Return:
    return "OP_Return";
  case vm::op::Opcode::SafeCreateLocalVariables:
    return "OP_SafeCreateLocalVariables";
  case vm::op::Opcode::SafeDecTop:
    return "OP_SafeDecTop";
  case vm::op::Opcode::SafeSetVariableFieldCached:
    return "OP_SafeSetVariableFieldCached";
  case vm::op::Opcode::SafeSetWaittillVariableFieldCached:
    return "OP_SafeSetWaittillVariableFieldCached";
  case vm::op::Opcode::ScriptFunctionCall:
    return "OP_ScriptFunctionCall";
  case vm::op::Opcode::ScriptFunctionCallClass:
    return "OP_ScriptFunctionCallClass";
  case vm::op::Opcode::ScriptFunctionCallPointer:
    return "OP_ScriptFunctionCallPointer";
  case vm::op::Opcode::ScriptMethodCall:
    return "OP_ScriptMethodCall";
  case vm::op::Opcode::ScriptMethodCallPointer:
    return "OP_ScriptMethodCallPointer";
  case vm::op::Opcode::ScriptMethodThreadCall:
    return "OP_ScriptMethodThreadCall";
  case vm::op::Opcode::ScriptMethodThreadCallPointer:
    return "OP_ScriptMethodThreadCallPointer";
  case vm::op::Opcode::ScriptThreadCall:
    return "OP_ScriptThreadCall";
  case vm::op::Opcode::ScriptThreadCallClass:
    return "OP_ScriptThreadCallClass";
  case vm::op::Opcode::ScriptThreadCallPointer:
    return "OP_ScriptThreadCallPointer";
  case vm::op::Opcode::SetVariableField:
    return "OP_SetVariableField";
  case vm::op::Opcode::ShiftLeft:
    return "OP_ShiftLeft";
  case vm::op::Opcode::ShiftRight:
    return "OP_ShiftRight";
  case vm::op::Opcode::SizeOf:
    return "OP_SizeOf";
  case vm::op::Opcode::Switch:
    return "OP_Switch";
  case vm::op::Opcode::Vector:
    return "OP_Vector";
  case vm::op::Opcode::VectorConstant:
    return "OP_VectorConstant";
  case vm::op::Opcode::VectorScale:
    return "OP_VectorScale";
  case vm::op::Opcode::WaitRealTime:
    return "OP_Wait";
  case vm::op::Opcode::WaitTill:
    return "OP_WaitTill";
  case vm::op::Opcode::WaitTillFrameEnd:
    return "OP_WaitTillFrameEnd";
  case vm::op::Opcode::WaitTillMatch:
    return "OP_WaitTillMatch";
  case vm::op::Opcode::DevblockBegin:
    return "OP_DevblockBegin";
  case vm::op::Opcode::EvalLevelFieldVariable:
    return "OP_EvalLevelFieldVariable";
  case vm::op::Opcode::EvalLevelFieldVariableRef:
    return "OP_EvalLevelFieldVariableRef";
  case vm::op::Opcode::EvalSelfFieldVariable:
    return "OP_EvalSelfFieldVariable";
  case vm::op::Opcode::EvalSelfFieldVariableRef:
    return "OP_EvalSelfFieldVariableRef";
  case vm::op::Opcode::GetWorld:
    return "OP_GetWorld";
  case vm::op::Opcode::GetWorldObject:
    return "OP_GetWorldObject";
  case vm::op::Opcode::SuperEqual:
    return "OP_SuperEqual";
  case vm::op::Opcode::SuperNotEqual:
    return "OP_SuperNotEqual";
  case vm::op::Opcode::Wait:
    return "OP_WaitRealTime";
  default:
    return "OP_UnknownOrInvalid";
  }
}
#endif

extern const frozen::unordered_map<Opcode, std::array<OP_TYPE, 0x86>,
                                   +Opcode::Count + 1 /* UnknownOrInvalid */>
    OPCODE_BYTECODE_MAP;
extern const std::array<std::array<Opcode, 0x2000>, 2>
    OPCODE_JUMPTABLE_ENTRY_MAP;
} // namespace op
} // namespace vm
} // namespace scr
} // namespace game
