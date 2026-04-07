#pragma once

#include "gsc_parser.hpp"
#include "gsc_compiler.hpp"
#include <unordered_map>

namespace gsc_compiler {
// T7 PC Script OpCodes (logical values)
enum class script_opcode : uint8_t {
  OP_End = 0x0,
  OP_Return = 0x1,
  OP_GetUndefined = 0x2,
  OP_GetZero = 0x3,
  OP_GetByte = 0x4,
  OP_GetNegByte = 0x5,
  OP_GetUnsignedShort = 0x6,
  OP_GetNegUnsignedShort = 0x7,
  OP_GetInteger = 0x8,
  OP_GetFloat = 0x9,
  OP_GetString = 0xA,
  OP_GetIString = 0xB,
  OP_GetVector = 0xC,
  OP_GetLevelObject = 0xD,
  OP_GetAnimObject = 0xE,
  OP_GetSelf = 0xF,
  OP_GetLevel = 0x10,
  OP_GetGame = 0x11,
  OP_GetAnim = 0x12,
  OP_GetAnimation = 0x13,
  OP_GetGameRef = 0x14,
  OP_GetFunction = 0x15,
  OP_CreateLocalVariable = 0x16,
  OP_SafeCreateLocalVariables = 0x17,
  OP_RemoveLocalVariables = 0x18,
  OP_EvalLocalVariableCached = 0x19,
  OP_EvalArray = 0x1A,
  OP_EvalLocalArrayRefCached = 0x1B,
  OP_EvalArrayRef = 0x1C,
  OP_ClearArray = 0x1D,
  OP_GetEmptyArray = 0x1E,
  OP_GetSelfObject = 0x1F,
  OP_EvalFieldVariable = 0x20,
  OP_EvalFieldVariableRef = 0x21,
  OP_ClearFieldVariable = 0x22,
  OP_SafeSetVariableFieldCached = 0x23,
  OP_SetWaittillVariableFieldCached = 0x24,
  OP_ClearParams = 0x25,
  OP_CheckClearParams = 0x26,
  OP_EvalLocalVariableRefCached = 0x27,
  OP_SetVariableField = 0x28,
  OP_CallBuiltin = 0x29,
  OP_CallBuiltinMethod = 0x2A,
  OP_Wait = 0x2B,
  OP_WaitTillFrameEnd = 0x2C,
  OP_PreScriptCall = 0x2D,
  OP_ScriptFunctionCall = 0x2E,
  OP_ScriptFunctionCallPointer = 0x2F,
  OP_ScriptMethodCall = 0x30,
  OP_ScriptMethodCallPointer = 0x31,
  OP_ScriptThreadCall = 0x32,
  OP_ScriptThreadCallPointer = 0x33,
  OP_ScriptMethodThreadCall = 0x34,
  OP_ScriptMethodThreadCallPointer = 0x35,
  OP_DecTop = 0x36,
  OP_CastFieldObject = 0x37,
  OP_CastBool = 0x38,
  OP_BoolNot = 0x39,
  OP_BoolComplement = 0x3A,
  OP_JumpOnFalse = 0x3B,
  OP_JumpOnTrue = 0x3C,
  OP_JumpOnFalseExpr = 0x3D,
  OP_JumpOnTrueExpr = 0x3E,
  OP_Jump = 0x3F,
  OP_JumpBack = 0x40,
  OP_Inc = 0x41,
  OP_Dec = 0x42,
  OP_Bit_Or = 0x43,
  OP_Bit_Xor = 0x44,
  OP_Bit_And = 0x45,
  OP_Equal = 0x46,
  OP_NotEqual = 0x47,
  OP_LessThan = 0x48,
  OP_GreaterThan = 0x49,
  OP_LessThanOrEqualTo = 0x4A,
  OP_GreaterThanOrEqualTo = 0x4B,
  OP_ShiftLeft = 0x4C,
  OP_ShiftRight = 0x4D,
  OP_Plus = 0x4E,
  OP_Minus = 0x4F,
  OP_Multiply = 0x50,
  OP_Divide = 0x51,
  OP_Modulus = 0x52,
  OP_SizeOf = 0x53,
  OP_WaitTillMatch = 0x54,
  OP_WaitTill = 0x55,
  OP_Notify = 0x56,
  OP_EndOn = 0x57,
  OP_VoidCodePos = 0x58,
  OP_Switch = 0x59,
  OP_EndSwitch = 0x5A,
  OP_Vector = 0x5B,
  OP_GetHash = 0x5C,
  OP_RealWait = 0x5D,
  OP_VectorConstant = 0x5E,
  OP_IsDefined = 0x5F,
  OP_VectorScale = 0x60,
  OP_AnglesToUp = 0x61,
  OP_AnglesToRight = 0x62,
  OP_AnglesToForward = 0x63,
  OP_AngleClamp180 = 0x64,
  OP_VectorToAngles = 0x65,
  OP_Abs = 0x66,
  OP_GetTime = 0x67,
  OP_GetDvar = 0x68,
  OP_GetDvarInt = 0x69,
  OP_GetDvarFloat = 0x6A,
  OP_GetDvarVector = 0x6B,
  OP_FirstArrayKey = 0x70,
  OP_NextArrayKey = 0x71,
  OP_Nop = 0x75,
  // Extended opcodes (T7 PC)
  OP_DevblockBegin = 0x7B,
  OP_DevblockEnd = 0x7C,
  OP_GetObjectType = 0x82,
  OP_WaitRealTime = 0x83,
  OP_GetWorldObject = 0x84,
  OP_Bit_Not = 0x87,
  OP_GetWorld = 0x88,
  OP_EvalLevelFieldVariable = 0x89,
  OP_EvalLevelFieldVariableRef = 0x8A,
  OP_EvalSelfFieldVariable = 0x8B,
  OP_EvalSelfFieldVariableRef = 0x8C,
  OP_SuperEqual = 0x8D,
  OP_SuperNotEqual = 0x8E,
};

// T7 script header (0x48 bytes, little-endian PC)
#pragma pack(push, 1)
struct t7_script_header {
  uint64_t magic;
  uint32_t source_crc;
  uint32_t include_offset;
  uint32_t animtree_offset;
  uint32_t bytecode_offset;
  uint32_t string_offset;
  uint32_t debug_string_offset;
  uint32_t export_offset;
  uint32_t import_offset;
  uint32_t fixup_offset;
  uint32_t profile_offset;
  uint32_t bytecode_size;
  uint32_t name_offset;
  uint16_t string_count;
  uint16_t export_count;
  uint16_t import_count;
  uint16_t fixup_count;
  uint16_t profile_count;
  uint16_t debug_string_count;
  uint8_t include_count;
  uint8_t animtree_count;
  uint16_t flags;
};
#pragma pack(pop)

static_assert(sizeof(t7_script_header) == 0x48, "T7 header must be 0x48 bytes");

// Import flags (FunctionFlags | CallFlags combined in one byte)
enum import_flags : uint8_t {
  IMPORT_FUNC_GETFUNCTION = 0x01,
  IMPORT_FUNC_CALL = 0x02,
  IMPORT_FUNC_THREAD = 0x03,
  IMPORT_FUNC_METHOD = 0x04,
  IMPORT_FUNC_METHOD_THREAD = 0x05,
  IMPORT_CALL_EXTERNAL = 0x00,
  IMPORT_CALL_EXTERNAL_DEV = 0x10,
  IMPORT_CALL_LOCAL = 0x20,
  IMPORT_CALL_LOCAL_DEV = 0x30,
};

// Export flags
enum export_flags : uint8_t {
  EXPORT_NONE = 0,
  EXPORT_AUTOEXEC = 2,
  EXPORT_PRIVATE = 4,
};

struct hash_name_entry {
  uint32_t hash;
  std::string name;
  int line;
  uint8_t params;
};

struct emitter_result {
  bool success;
  std::vector<uint8_t> data;
  std::string error;
  int error_line;
  int error_column;
  std::vector<hash_name_entry> hash_names;
  std::vector<replacefunc_entry> replacefuncs;
};

emitter_result emit(const ast_ptr &root, const std::string &script_name);
} // namespace gsc_compiler
