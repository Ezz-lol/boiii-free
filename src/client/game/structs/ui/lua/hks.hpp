#pragma once

#include <game/structs/core.hpp>
#include <game/structs/func.hpp>
#include <game/structs/quake/core.hpp>
#include <game/ptr.hpp>

namespace game {

namespace ui {
namespace lua {

struct LuaStateContext;

namespace plus {
class LuaObject;
}

namespace hks {

enum class AllocTypes : uint32_t {
  Table_Hdr = 0x0,
  Table_Hash = 0x1,
  Table_Array = 0x2,
  CClosure = 0x3,
  Closure = 0x4,
  IFunction_Hdr = 0x5,
  IFunction_Constants = 0x6,
  IFunction_Debug_LineInfo = 0x7,
  IFunction_Debug_LocalInfo = 0x8,
  IFunction_Debug_UpvalInfo = 0x9,
  IFunction_Instructions = 0xA,
  IFunction_Children = 0xB,
  ByteCodeReader_Data = 0xC,
  CharacterBuffer = 0xD,
  State = 0xE,
  State_APIStack = 0xF,
  State_CallStack = 0x10,
  State_Globals = 0x11,
  Upvalue = 0x12,
  Struct_Proto_List = 0x13,
  Struct_Instance = 0x14,
  Serialize_MemBuffer = 0x15,
  Serialize_StringReader = 0x16,
  GC_VisitStack = 0x17,
  GC_WeakStack = 0x18,
  GC_GreyStack = 0x19,
  GC_GreyStack_RemarkStack = 0x1A,
  UserData = 0x1B,
  String = 0x1C,
  String_Table = 0x1D,
  String_Pinner = 0x1E,
  String_Pinner_Node = 0x1F,
  Script_Profiler = 0x20,
  Dynamic_Buffer = 0x21,
  Bytecode_Optimizer = 0x22,
  Compiler_Preprocessor = 0x23,
  Compiler_Func_Object = 0x24,
  IFunction_DebugInfo = 0x25,
  Num_Types = 0x26,
};
IMPL_ENUM_OPERATORS(AllocTypes);

enum class luaState_e : int32_t {
  INVALID = -1,
  FIRST = 0x0,
  UI_VM = 0x0,
  DEFAULT = 0x0,
  LOBBY_VM = 0x1,
  COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(luaState_e);

enum class luaReturnCount_e : uint32_t {
  INVALID = 0xFFFFFFFF,
  NONE = 0x0,
  ONE = 0x1,
  TWO = 0x2,
  THREE = 0x3,
  FOUR = 0x4,
  FIVE = 0x5,
  SIX = 0x6,
  SEVEN = 0x7,
  EIGHT = 0x8,
  NINE = 0x9,
};
IMPL_ENUM_OPERATORS(luaReturnCount_e);

enum class Opcode : uint32_t {
  GETFIELD = 0x0,
  TEST = 0x1,
  CALL_I = 0x2,
  CALL_C = 0x3,
  EQ = 0x4,
  EQ_BK = 0x5,
  GETGLOBAL = 0x6,
  MOVE = 0x7,
  SELF = 0x8,
  RETURN = 0x9,
  GETTABLE_S = 0xa,
  GETTABLE_N = 0xb,
  GETTABLE = 0xc,
  LOADBOOL = 0xd,
  TFORLOOP = 0xe,
  SETFIELD = 0xf,
  SETTABLE_S = 0x10,
  SETTABLE_S_BK = 0x11,
  SETTABLE_N = 0x12,
  SETTABLE_N_BK = 0x13,
  SETTABLE = 0x14,
  SETTABLE_BK = 0x15,
  TAILCALL_I = 0x16,
  TAILCALL_C = 0x17,
  TAILCALL_M = 0x18,
  LOADK = 0x19,
  LOADNIL = 0x1a,
  SETGLOBAL = 0x1b,
  JMP = 0x1c,
  CALL_M = 0x1d,
  CALL = 0x1e,
  INTRINSIC_INDEX = 0x1f,
  INTRINSIC_NEWINDEX = 0x20,
  INTRINSIC_SELF = 0x21,
  INTRINSIC_INDEX_LITERAL = 0x22,
  INTRINSIC_NEWINDEX_LITERAL = 0x23,
  INTRINSIC_SELF_LITERAL = 0x24,
  TAILCALL = 0x25,
  GETUPVAL = 0x26,
  SETUPVAL = 0x27,
  ADD = 0x28,
  ADD_BK = 0x29,
  SUB = 0x2a,
  SUB_BK = 0x2b,
  MUL = 0x2c,
  MUL_BK = 0x2d,
  DIV = 0x2e,
  DIV_BK = 0x2f,
  MOD = 0x30,
  MOD_BK = 0x31,
  POW = 0x32,
  POW_BK = 0x33,
  NEWTABLE = 0x34,
  UNM = 0x35,
  NOT = 0x36,
  LEN = 0x37,
  LT = 0x38,
  LT_BK = 0x39,
  LE = 0x3a,
  LE_BK = 0x3b,
  LEFT_SHIFT = 0x3c,
  LEFT_SHIFT_BK = 0x3d,
  RIGHT_SHIFT = 0x3e,
  RIGHT_SHIFT_BK = 0x3f,
  BIT_AND = 0x40,
  BIT_AND_BK = 0x41,
  BIT_OR = 0x42,
  BIT_OR_BK = 0x43,
  CONCAT = 0x44,
  TESTSET = 0x45,
  FORPREP = 0x46,
  FORLOOP = 0x47,
  SETLIST = 0x48,
  CLOSE = 0x49,
  CLOSURE = 0x4a,
  VARARG = 0x4b,
  TAILCALL_I_R1 = 0x4c,
  CALL_I_R1 = 0x4d,
  SETUPVAL_R1 = 0x4e,
  TEST_R1 = 0x4f,
  NOT_R1 = 0x50,
  GETFIELD_R1 = 0x51,
  SETFIELD_R1 = 0x52,
  NEWSTRUCT = 0x53,
  DATA = 0x54,
  SETSLOTN = 0x55,
  SETSLOTI = 0x56,
  SETSLOT = 0x57,
  SETSLOTS = 0x58,
  SETSLOTMT = 0x59,
  CHECKTYPE = 0x5a,
  CHECKTYPES = 0x5b,
  GETSLOT = 0x5c,
  GETSLOTMT = 0x5d,
  SELFSLOT = 0x5e,
  SELFSLOTMT = 0x5f,
  GETFIELD_MM = 0x60,
  CHECKTYPE_D = 0x61,
  GETSLOT_D = 0x62,
  GETGLOBAL_MEM = 0x63,
  MAX = 0x64
};
IMPL_ENUM_OPERATORS(Opcode);

enum class Metamethod : uint32_t {
  M_MODE = 0x0,
  M_INDEX = 0x1,
  M_NEWINDEX = 0x2,
  M_ADD = 0x3,
  M_SUB = 0x4,
  M_MUL = 0x5,
  M_DIV = 0x6,
  M_MOD = 0x7,
  M_POW = 0x8,
  M_LEFT_SHIFT = 0x9,
  M_RIGHT_SHIFT = 0xA,
  M_BIT_AND = 0xB,
  M_BIT_OR = 0xC,
  M_CONCAT = 0xD,
  M_LEN = 0xE,
  M_UNM = 0xF,
  M_LE = 0x10,
  M_EQ = 0x11,
  M_LT = 0x12,
  M_GC = 0x13,
  M_CALL = 0x14,
  M_VIZ = 0x15,
  M_SELF = 0x16,
  N_METAMETHODS = 0x17,
};
IMPL_ENUM_OPERATORS(Metamethod);

struct lua_State;
struct HashTable;
struct StringTable;
struct cclosure;
struct HksObject;
union hksInstruction;
struct UpValue;

typedef double HksDouble;
typedef qboolean hksBool;
typedef char hksChar;
typedef uint8_t hksByte;
typedef int16_t hksShort16;
typedef uint16_t hksUshort16;
typedef float HksNumber;
typedef int32_t hksInt32;
typedef uint32_t hksUint32;
typedef hksUint32 hksUint;
typedef hksUint32 HksNativeValueAsInt;
typedef int64_t hksInt64;
typedef uint64_t hksUint64;
typedef uint64_t hksSize;

typedef int32_t HksGcCost;
typedef void *hks_fixedheap;

typedef fastcall_t<void *(void *userData, void *ptr, size_t osize,
                          size_t nsize)>
    lua_Alloc;
typedef fastcall_t<hksInt32(lua_State *s)> lua_CFunction;
typedef fastcall_t<char *(lua_State *s, void *data, size_t *size)> lua_Reader;
typedef fastcall_t<void(lua_State *s, const char *fmt, ...)> HksLogFunc;
typedef fastcall_t<void(lua_State *s, size_t requestSize)>
    HksEmergencyGCFailFunc;
typedef fastcall_t<size_t(hks_fixedheap heap, size_t request)> hks_outofmemory;
typedef fastcall_t<int32_t(const char *filename, int32_t lua_line)>
    hks_debug_map;
typedef fastcall_t<void(lua_State *s, int64_t nargs, int32_t nresults,
                        const hksInstruction *pc)>
    Hkslua_Caller;

typedef hksInt32 lua_Integer;
typedef HksDouble LightUserData;

struct GenericChunkHeader {
  hksSize m_flags;
};

struct ChunkHeader : public GenericChunkHeader {
  ChunkHeader *m_next;
};
ASSERT_SIZE(ChunkHeader, 0x10);

struct ChunkList {
  ChunkHeader m_head;
};
ASSERT_SIZE(ChunkList, 0x10);

struct UserData : ChunkHeader {
  hksSize m_envAndSizeOffsetHighBits;
  hksSize m_metaAndSizeOffsetLowBits;
  char m_data[8];
};

#pragma pack(push, 1)
struct InternString : GenericChunkHeader {
  hksSize m_lengthbits;
  hksUint32 m_hash;
  char m_data[30];
  uint8_t _padding32[6];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Method : ChunkHeader {

  template <typename T> struct Array {
    hksUint32 size;
    uint8_t _padding04[4];
    const T *data;
  };
  typedef Array<hksInstruction> Instructions;
  typedef Array<HksObject> Constants;
  typedef Array<Method *> Children;
  hksUint32 hash;
  hksUshort16 num_upvals;
  hksUshort16 m_numRegisters;
  hksByte num_params;
  hksByte m_flags;
  Method::Instructions instructions;
  Method::Constants constants;
  Method::Children children;
};

#pragma pack(pop)

#pragma pack(push, 1)

struct HksClosure : ChunkHeader {
  struct MethodCache {
    const HksObject *consts;
    const hksInstruction *inst;
    hksUshort16 m_numRegisters;
    hksByte m_flags;
    hksByte num_params;
  };

  Method *m_method;
  HashTable *m_env;
  hksByte m_mayHaveUpvalues;
  MethodCache m_cache;
  uint8_t _padding35[3];
  UpValue *m_upvalues[1];
};
#pragma pack(pop)

// Only ever declared. Intentionally, correctly undefined.
struct StructInst;

union HksValue {
  cclosure *cClosure;
  HksClosure *closure;
  UserData *userData;
  HashTable *table;
  StructInst *tstruct;
  InternString *str;
  lua_State *thread;
  void *ptr;
  uintptr_t uintPtr;
  uint64_t raw;
  HksNumber number;
  LightUserData lightUserData;
  HksNativeValueAsInt native;
  hksBool boolean;
};
ASSERT_SIZE(HksValue, sizeof(uint64_t));

enum class HksObjectType : int32_t {
  TANY = -1,
  TNONE = -1,
  TNIL = 0x0,
  TBOOLEAN = 0x1,
  TLIGHTUSERDATA = 0x2,
  TNUMBER = 0x3,
  TSTRING = 0x4,
  TTABLE = 0x5,
  TFUNCTION = 0x6,
  // idk
  TUSERDATA = 0x7,
  TTHREAD = 0x8,
  TIFUNCTION = 0x9,
  // Lua function
  TCFUNCTION = 0xA,
  // C function
  TUI64 = 0xB,
  TSTRUCT = 0xC,
  COUNT = 0xD,
};
static_assert(min_bits_unsigned(static_cast<int32_t>(HksObjectType::COUNT)) ==
              4);
constexpr uint32_t HKS_OBJECT_TYPE_MASK =
    min_bits_mask(static_cast<int32_t>(HksObjectType::COUNT));
static_assert(HKS_OBJECT_TYPE_MASK == 0xF, "HKS_OBJECT_TYPE_MASK != 0xF");

#pragma pack(push, 1)
struct HksObject {
  HksObjectType t;
  uint8_t _padding04[4];
  HksValue v;

  inline constexpr bool truthy() const {
    return this->t != HksObjectType::TNIL &&
           (this->t != HksObjectType::TBOOLEAN || this->v.native != 0);
  }

  inline constexpr HksObjectType type() const {
    return static_cast<HksObjectType>(static_cast<uint32_t>(this->t) &
                                      HKS_OBJECT_TYPE_MASK);
  }

  constexpr HksObject() noexcept = default;
  constexpr HksObject(const HksObject &) noexcept = default;
  constexpr HksObject &operator=(const HksObject &) = default;

  constexpr HksObject(const HksObjectType ty, const HksValue value) noexcept
      : t(ty), _padding04{0, 0, 0, 0}, v(value) {}
};
ASSERT_SIZE(HksObject, 0x10);
#pragma pack(pop)

typedef HksObject HksRegister;

union hksInstruction {
  hksUint32 code;

  // Layout 1: Standard iABC format
  struct {
    hksUint32 opA : 8;    // Bits 0-7
    hksUint32 opC_RK : 9; // Bits 8-16
    hksUint32 opB : 8;    // Bits 17-24
    Opcode opcode : 7;    // Bits 25-31 (Opcode portion)
  };

  // Layout 2: ABC format with 8-bit opC
  struct {
    hksUint32 : 8;     // Bits 0-7. Skip.
    hksUint32 opC : 8; // Bits 8-16
    hksUint32 : 16;
  };

  // Layout 3: ABC format with 9-bit Registor-or-Constant (RK) index in opB
  struct {
    hksUint32 : 17; // Bits 0-16. opA, opC; same as in Layout 1. Skip.
    // 9-bit Register-or-Constant (RK)
    hksUint32 opB_RK : 9; // Bits 17-25. Shares first bit of opcode (opc).
    hksUint32 : 6;        // Bits 26-31. Skip - access as opc above instead.
  };

  // Layout 4: Extended Bx Format
  struct {
    hksUint32 : 8;      // Skip opA (Bits 0-7)
    hksInt32 opBx : 17; // Bits 8-24
                        // The remaining 7 bits are shared with the opcode above
    hksUint32 : 7;      // Bits 25-31. Unused.
  };

  inline constexpr Opcode Opcode() const { return opcode; }

  inline constexpr hksInt32 OpSBx() const { return opBx - 0xFFFF; }

  constexpr hksInstruction() noexcept = default;
  constexpr hksInstruction(const hksInstruction &) noexcept = default;
  constexpr hksInstruction(hksInstruction &&) noexcept = default;

  constexpr hksInstruction(bool b) noexcept : code(b ? 1 : 0) {}

  // Implicit conversion from all standard integer types
  template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
  constexpr hksInstruction(T val) noexcept : code(static_cast<int32_t>(val)) {}

  constexpr operator bool() const noexcept { return code != 0; }

  template <typename T, typename = std::enable_if_t<std::is_integral_v<T> &&
                                                    !std::is_same_v<T, bool>>>
  constexpr operator T() const noexcept {
    return static_cast<T>(code);
  }

  constexpr hksInstruction &
  operator=(const hksInstruction &) noexcept = default;
  constexpr hksInstruction &operator=(hksInstruction &&) noexcept = default;

  constexpr bool operator!() const noexcept { return code == 0; }
};
ASSERT_SIZE(hksInstruction, sizeof(hksUint32));
ASSERT_POD(hksInstruction);
#pragma pack(push, 1)
struct CallStack {
  struct ActivationRecord {
    HksObject *m_base;
    const hksInstruction *m_returnAddress;
    hksShort16 m_tailCallDepth;
    hksShort16 m_numVarargs;
    luaReturnCount_e m_numExpectedReturns;
  };

  ActivationRecord *m_records;
  ActivationRecord *m_lastrecord;
  ActivationRecord *m_current;
  const hksInstruction *m_current_lua_pc;
  const hksInstruction *m_hook_return_addr;
  hksInt32 m_hook_level;
  uint8_t _padding2C[4];

  static inline HksRegister *functionCall(hks::CallStack *callstack,
                                          lua_State *s, hksInt32 nresults,
                                          HksObject *arg_end,
                                          HksObject *stackTop,
                                          const hksInstruction *instruction) {
    using funcType = decltype(functionCall);
    funcType *functionCallImpl =
        reinterpret_cast<funcType *>(game::select(0x141D362A0, 0x1403DF640));
    return functionCallImpl(callstack, s, nresults, arg_end, stackTop,
                            instruction);
  }

  static inline void push(hks::CallStack *callstack, lua_State *s,
                          int32_t m_numExpectedReturns, HksObject *fp,
                          HksObject *stackTop,
                          const hksInstruction *m_returnAddress) {
    using funcType = decltype(push);
    funcType *pushImpl =
        reinterpret_cast<funcType *>(game::select(0x141D36410, 0x1403DF7B0));
    return pushImpl(callstack, s, m_numExpectedReturns, fp, stackTop,
                    m_returnAddress);
  }

  static inline HksObject *pop(CallStack *callStack, lua_State *s,
                               luaReturnCount_e returnCount) {
    using funcType = decltype(pop);
    funcType *popImpl =
        reinterpret_cast<funcType *>(game::select(0x141D35FF0, 0x1403DF390));
    return popImpl(callStack, s, returnCount);
  }

  static inline void functionTailCall(CallStack *callstack, lua_State *s,
                                      HksObject *function, HksObject *arg_end) {
    using funcType = decltype(functionTailCall);
    funcType *functionTailCallImpl =
        reinterpret_cast<funcType *>(game::select(0x141D44D70, 0x1403ED990));
    return functionTailCallImpl(callstack, s, function, arg_end);
  }

  static inline void growApiStack(CallStack *callstack, lua_State *s,
                                  hksInt32 extra) {
    using funcType = decltype(growApiStack);
    funcType *growApiStackImpl =
        reinterpret_cast<funcType *>(game::select(0x141D48E90, 0x1403F1A30));
    return growApiStackImpl(callstack, s, extra);
  }
};
ASSERT_SIZE(CallStack, 0x30);
#pragma pack(pop)

struct ApiStack {
  HksObject *top;
  HksObject *base;
  HksObject *alloc_top;
  HksObject *bottom;
};
ASSERT_SIZE(ApiStack, 0x20);

struct UpValue : ChunkHeader {
  HksObject m_storage;
  HksObject *loc;
  UpValue *m_next;
};

struct CallSite {
  float128_t m_jumpBuffer[16];
  CallSite *m_prev;
};

enum class HksError : int32_t {
  HKS_NO_ERROR = 0,
  ERRSYNTAX = -4,
  ERRFILE = -5,
  ERRRUN = -100,
  ERRMEM = -200,
  ERRERR = -300,
  THROWING_ERROR = -500,
  GC_YIELD = 1,
};

struct lua_Debug {
  hksInt32 event;
  const char *name;
  const char *namewhat;
  const char *what;
  const char *source;
  hksInt32 currentline;
  hksInt32 nups;
  hksInt32 nparams;
  hksInt32 ishksfunc;
  hksInt32 linedefined;
  hksInt32 lastlinedefined;
  char short_src[512];
  hksInt32 callstack_level;
  hksInt32 is_tail_call;
};

using lua_function = fastcall_t<hksInt32(lua_State *s)>;

struct luaL_Reg {
  const char *name;
  lua_function function;
};

struct StringPinner {
  struct Node {
    InternString *m_strings[32];
    Node *m_prev;
  };

  lua_State *const m_state;
  StringPinner *const m_prev;
  InternString **m_nextStringsPlace;
  Node m_firstNode;
  Node *m_currentNode;
};

struct StringTable {
  InternString **m_data;
  hksUint32 m_count;
  hksUint32 m_mask;
  StringPinner *m_pinnedStrings;
};
ASSERT_SIZE(StringTable, 0x18);

struct MetaTable {
  uint8_t gap0;
};

#pragma pack(push, 1)
struct HashTable : public ChunkHeader {
  struct Node {
    HksObject m_key;
    HksObject m_value;
  };
  ASSERT_SIZE(Node, 0x20);

  MetaTable *m_meta;
  hksUint32 m_mask;
  hksUint32 m_version;
  Node *m_hashPart;
  HksObject *m_arrayPart;
  hksUint32 m_arraySize;
  uint8_t _padding34[4];
  Node *m_freeNode;

  inline constexpr HashTable() noexcept = default;
  static HksRegister *getByString(HashTable *self, HksRegister *retstr,
                                  const HksRegister *key) {
    using funcType = decltype(getByString);
    funcType *getByStringImpl =
        reinterpret_cast<funcType *>(game::select(0x141D45550, 0x1403EE160));

    return getByStringImpl(self, retstr, key);
  }

  static void insertString(HashTable *self, lua_State *s, InternString *key,
                           const HksObject *value) {
    using funcType = decltype(insertString);
    funcType *insertStringImpl =
        reinterpret_cast<funcType *>(game::select(0x141D51850, 0x1403FA3D0));
    return insertStringImpl(self, s, key, value);
  }
  static HashTable::Node *findKeyPosition(HashTable *self,
                                          const HksObject *key) {
    using funcType = decltype(findKeyPosition);
    funcType *findKeyPositionImpl =
        reinterpret_cast<funcType *>(game::select(0x141D43F00, 0x1403ECB20));
    return findKeyPositionImpl(self, key);
  }

  static void tableInsert(HashTable *self, lua_State *s, const HksObject *key,
                          const HksObject *value) {
    using funcType = decltype(tableInsert);
    funcType *tableInsertImpl =
        reinterpret_cast<funcType *>(game::select(0x141D6D310, 0x140415420));
    return tableInsertImpl(self, s, key, value);
  }

  static hksUint32 contiguousArraySize(const HashTable *self) {
    using funcType = decltype(contiguousArraySize);
    funcType *contiguousArraySizeImpl =
        reinterpret_cast<funcType *>(game::select(0x141D3E850, 0x1403E7930));
    return contiguousArraySizeImpl(self);
  }

  static HashTable *Create(lua_State *s, hksUint32 arraySize,
                           hksUint32 hashSize) {
    using funcType = decltype(Create);
    funcType *CreateImpl =
        reinterpret_cast<funcType *>(game::select(0x141D3B5F0, 0x1403E46D0));
    return CreateImpl(s, arraySize, hashSize);
  }

  static Node *getNext(HashTable *self, HksObject *key,
                       HksRegister *nextArray) {
    using funcType = decltype(getNext);
    funcType *getNextImpl =
        reinterpret_cast<funcType *>(game::select(0x141D45F40, 0x1403EEB40));
    return getNextImpl(self, key, nextArray);
  }

  static void hksNext(HashTable *self, HksObject *it, HksObject *key,
                      HksObject *val) {
    using funcType = decltype(hksNext);
    funcType *hksNextImpl =
        reinterpret_cast<funcType *>(game::select(0x141D49C10, 0x1403F27B0));
    return hksNextImpl(self, it, key, val);
  }

  static void setArraySize(HashTable *self, lua_State *s, hksUint32 arraySize) {
    using funcType = decltype(setArraySize);
    funcType *setArraySizeImpl =
        reinterpret_cast<funcType *>(game::select(0x141D6A430, 0x140412550));
    return setArraySizeImpl(self, s, arraySize);
  }

  static void arrayInserts(hks::HashTable *self, lua_State *s, hksUint32 from,
                           hksUint32 to, HksObject *src) {
    using funcType = decltype(arrayInserts);
    funcType *arrayInsertsImpl =
        reinterpret_cast<funcType *>(game::select(0x141D3CC00, 0x1403E5CE0));
    return arrayInsertsImpl(self, s, from, to, src);
  }
};
ASSERT_SIZE(HashTable, 0x40);
static_assert(std::is_trivially_copyable_v<HashTable>,
              "HashTable must be trivially copyable!");
static_assert(std::is_trivially_constructible_v<HashTable>,
              "HashTable must be trivially constructible!");
static_assert(std::is_trivially_destructible_v<HashTable>,
              "HashTable must be trivially destructible!");
static_assert(std::is_trivially_copy_constructible_v<HashTable>,
              "HashTable must be trivially copy constructible!");
#pragma pack(pop)

struct cclosure : ChunkHeader {
  lua_function m_function;
  HashTable *m_env;
  hksShort16 m_numUpvalues;
  hksShort16 m_flags;
  InternString *m_name;
  HksObject m_upvalues[1];
};

struct HksCompilerSettings {
  enum class BytecodeSharingFormat : uint32_t {
    DEFAULT = 0x0,
    INPLACE = 0x1,
    REFERENCED = 0x2,
  };

  enum class IntLiteralOptions : uint32_t {
    INT_LITERALS_NONE = 0x0,
    INT_LITERALS_LUD = 0x1,
    INT_LITERALS_32BIT = 0x1,
    INT_LITERALS_UI64 = 0x2,
    INT_LITERALS_64BIT = 0x2,
    INT_LITERALS_ALL = 0x3,
  };

  hksBool m_emitStructCode;
  uint8_t _padding04[4];
  const char **m_stripNames;
  BytecodeSharingFormat m_bytecodeSharingFormat;
  IntLiteralOptions m_enableIntLiterals;
  // Confirmed present
  hks_debug_map m_debugMap;
};
ASSERT_SIZE(HksCompilerSettings, 0x20);

enum class HksBytecodeSharingMode : int64_t { OFF = 0, ON = 1, SECURE = 2 };

struct HksGcWeights {
  hksInt32 m_removeString;
  hksInt32 m_finalizeUserdataNoMM;
  hksInt32 m_finalizeUserdataGcMM;
  hksInt32 m_cleanCoroutine;
  hksInt32 m_removeWeak;
  hksInt32 m_markObject;
  hksInt32 m_traverseString;
  hksInt32 m_traverseUserdata;
  hksInt32 m_traverseCoroutine;
  hksInt32 m_traverseWeakTable;
  hksInt32 m_freeChunk;
  hksInt32 m_sweepTraverse;
};

struct ProtoList {
  void **m_protoList;
  hksUshort16 m_protoSize;
  hksUshort16 m_protoAllocSize;
};

struct MemoryManager;

struct ResumeData_Header {
  HksObjectType m_type;
};

enum class GCResumePhase : uint32_t {
  GC_STATE_MARKING_UPVALUES = 0x0,
  GC_STATE_MARKING_GLOBAL_TABLE = 0x1,
  GC_STATE_MARKING_REGISTRY = 0x2,
  GC_STATE_MARKING_PROTOTYPES = 0x3,
  GC_STATE_MARKING_SCRIPT_PROFILER = 0x4,
  GC_STATE_MARKING_FINALIZER_STATE = 0x5,
  GC_TABLE_MARKING_ARRAY = 0x6,
  GC_TABLE_MARKING_HASH = 0x7,
};

#pragma pack(push, 1)
struct ResumeData_State {
  ResumeData_Header h;
  uint8_t _padding04[4];
  lua_State *m_state;
  GCResumePhase m_phase;
  uint8_t _padding14[4];
  UpValue *m_pending;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ResumeData_Table {
  ResumeData_Header h;
  uint8_t _padding04[4];
  HashTable *m_table;
  hksUint32 m_arrayIndex;
  hksUint32 m_hashIndex;
  hksInt32 m_weakness;
  uint8_t _padding1C[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ResumeData_Closure {
  ResumeData_Header h;
  uint8_t _padding04[4];
  HksClosure *m_closure;
  hksInt32 m_index;
  uint8_t _padding14[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ResumeData_CClosure {
  ResumeData_Header h;
  uint8_t _padding04[4];
  cclosure *m_cclosure;
  hksInt32 m_upvalueIndex;
  uint8_t _padding14[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ResumeData_Userdata {
  ResumeData_Header h;
  uint8_t _padding04[4];
  UserData *m_data;
};
#pragma pack(pop)

union ResumeData_Entry {
  ResumeData_State State;
  ResumeData_Table HashTable;
  ResumeData_Closure Closure;
  ResumeData_CClosure CClosure;
  ResumeData_Userdata Userdata;
};

#pragma pack(push, 1)
struct WeakStack_Entry {
  hksInt32 m_weakness;
  uint8_t _padding04[4];
  HashTable *m_table;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GarbageCollector {

  struct ResumeStack {
    ResumeData_Entry *m_storage;
    hksInt32 m_numEntries;
    hksUint32 m_numAllocated;
  };
  ASSERT_SIZE(ResumeStack, 0x10);

  struct WeakStack {
    void *unknown;
    WeakStack_Entry *m_storage;
    hksInt32 m_numEntries;
    hksUint32 m_numAllocated;
  };
  ASSERT_SIZE(WeakStack, 0x18);

  struct RemarkStack {
    hksSize m_numEntries;
    HashTable **m_storage;
    hksSize m_numAllocated;
  };
  ASSERT_SIZE(RemarkStack, 0x18);

  struct GreyStack {
    HksObject *m_storage;
    hksSize m_numEntries;
    hksSize m_numAllocated;
  };
  ASSERT_SIZE(GreyStack, 0x18);

  HksGcCost m_target;
  HksGcCost m_stepsLeft;
  HksGcCost m_stepLimit;
  HksGcWeights m_costs;
  HksGcCost m_unit;
  jmp_buf *m_jumpPoint;
  lua_State *m_mainState;
  lua_State *m_finalizerState;
  MemoryManager *m_memory;
  void *m_emergencyGCMemory;
  hksInt32 m_phase;
  uint8_t _padding6C[4];
  ResumeStack m_resumeStack;
  GreyStack m_greyStack;
  RemarkStack m_remarkStack;
  WeakStack m_weakStack;
  hksBool m_finalizing;
  uint8_t _paddingCC[4];
  HksObject m_safeTableValue;
  lua_State *m_startOfStateStackList;
  lua_State *m_endOfStateStackList;
  lua_State *m_currentState;
  HksObject m_safeValue;
  void *m_compiler;
  void *m_bytecodeReader;
  void *m_bytecodeWriter;
  hksInt32 m_pauseMultiplier;
  HksGcCost m_stepMultiplier;
  hksSize m_emergencyMemorySize;
  bool m_stopped;
  uint8_t _padding131[7];
  lua_CFunction m_gcPolicy;
  hksSize m_pauseTriggerMemoryUsage;
  hksInt32 m_stepTriggerCountdown;
  hksUint32 m_stringTableIndex;
  hksUint32 m_stringTableSize;
  uint8_t _padding154[4];
  UserData *m_lastBlackUD;
  UserData *m_activeUD;

  static void writeBarrier(GarbageCollector *self,
                           const hks::GenericChunkHeader *to,
                           const HksObject *from) {
    using funcType = decltype(writeBarrier);
    funcType *writeBarrierImpl =
        reinterpret_cast<funcType *>(game::select(0x141D71A10, 0x140419870));
    return writeBarrierImpl(self, to, from);
  }
};
ASSERT_SIZE(GarbageCollector, 0x168);
#pragma pack(pop)

struct MemoryManager {
  enum class ChunkColor : int32_t {
    WHITE = 0x0,
    BLACK = 0x1,
  };

  lua_Alloc m_allocator;
  void *m_allocatorUd;
  ChunkColor m_chunkColor;
  hksSize m_used;
  hksSize m_highwatermark;
  ChunkList m_allocationList;
  ChunkList m_sweepList;
  ChunkHeader *m_lastKeptChunk;
  lua_State *m_state;
};
ASSERT_SIZE(MemoryManager, 0x58);

struct StaticStringCache {
  HksObject m_objects[41];
};
ASSERT_SIZE(StaticStringCache, 0x290);

enum class HksBytecodeEndianness : uint32_t {
  DEFAULT = 0x0,
  BIG = 0x1,
  LITTLE = 0x2,
};

struct RuntimeProfileData {
  struct Stats {
    uint64_t hksTime;
    uint64_t callbackTime;
    uint64_t gcTime;
    uint64_t cFinalizerTime;
    uint64_t compilerTime;
    hksUint32 hkssTimeSamples;
    hksUint32 callbackTimeSamples;
    hksUint32 gcTimeSamples;
    hksUint32 compilerTimeSamples;
    hksUint32 num_newuserdata;
    hksUint32 num_tablerehash;
    hksUint32 num_pushstring;
    hksUint32 num_pushcfunction;
    hksUint32 num_newtables;
  };

  int64_t stackDepth;
  int64_t callbackDepth;
  uint64_t lastTimer;
  Stats frameStats;
  uint64_t gcStartTime;
  uint64_t finalizerStartTime;
  uint64_t compilerStartTime;
  uint64_t compilerStartGCTime;
  uint64_t compilerStartGCFinalizerTime;
  uint64_t compilerCallbackStartTime;
  int64_t compilerDepth;
  void *outFile;
  lua_State *rootState;
};

struct Debugger;
struct Breakpoint;

struct Breakpoint {

  enum expressionTypes : int32_t {
    LOWER_EXPRESSION_TYPES_BOUND = -2,
    HITCOUNT_EXP_LT = -1,
    HITCOUNT_EXP_EQ = 0x0,
    HITCOUNT_EXP_GT = 0x1,
    HITCOUNT_DISABLED = 0x2,
    UPPER_EXPRESSION_TYES_BOUND = 0x3,
  };

  enum class bpStates : uint32_t {
    NEW = 0x0,
    DISABLED = 0x1,
    ENABLED = 0x2,
  };

  char m_file[512];
  char m_expression[512];
  Breakpoint *m_next;
  Breakpoint *m_nextInChain;
  lua_State *m_coroutine;
  hksInt32 m_targetHitCount;
  hksInt32 m_hitCount;
  hksInt32 m_line;
  hksInt32 m_refCount;
  Breakpoint::bpStates m_bpState;
  Breakpoint::expressionTypes m_hitCountExpression;
};

struct BreakpointList {
  Breakpoint *m_list[128];
  Debugger *m_parent;
  Breakpoint *m_first;
};
struct DebugInstance;

#pragma pack(push, 1)
struct Debugger {
  enum class RunMode : uint32_t {
    RUN_CONTINUE = 0x0,
    RUN_STEP_IN = 0x1,
    RUN_STEP_OUT = 0x2,
    RUN_STEP_OVER = 0x3,
  };

  int (**vtbl)(void);
  const hksInstruction *m_entryPC;
  hks::Debugger::RunMode m_runMode;
  hksInt32 m_targetCallstackLevel;
  lua_State *m_targetState;
  hksInt32 m_stackLevelLineNumTracking[64];
  lua_Alloc m_allocatorFunction;
  void *m_allocatorUserData;
  hksSize m_allocated;
  hks::BreakpointList m_breakpoints;
  hks::DebugInstance *m_head;
  lua_State *m_profilerState;
  hksUint64 m_lastTime;
  hksBool m_ToggleProfile;
  uint8_t _padding564[4];
};

#pragma pack(pop)

struct DebugInstance;
#pragma pack(push, 1)
struct DebugInstance {

  struct RuntimeProfilerStats {
    hksInt32 hksTime;
    hksInt32 callbackTime;
    hksInt32 gcTime;
    hksInt32 cFinalizerTime;
    hksInt64 heapSize;
    hksInt64 num_newuserdata;
    hksInt64 num_pushstring;
  };

  hksInt32 m_savedObjects;
  hksInt32 m_keepAliveObjects;
  lua_State *m_activeState;
  lua_State *m_mainState;
  Debugger *m_owner;
  hksInt32 m_DebuggerLevel;
  hksInt32 stored_Hook_level;
  bool m_clearHook;
  uint8_t _padding29[7];
  const hksInstruction *stored_Hook_return_addr;
  hksInt32 m_debugStepLastLine;
  uint8_t _padding3C[4];
  DebugInstance *m_next;
  const hksInstruction *m_activePC;
  hksInt32 runtimeProfileSendBufferWritePosition;
  uint8_t _padding54[4];
  RuntimeProfilerStats runtimeProfileSendBuffer[30];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct HksGlobal {
  MemoryManager m_memory;
  GarbageCollector m_collector;
  StringTable m_stringTable;
  HksBytecodeSharingMode m_bytecodeSharingMode;
  HksObject m_registry;
  ChunkList m_userDataList;
  lua_State *m_root;
  StaticStringCache m_staticStringCache;
  DebugInstance *m_debugger;
  void *m_profiler;
  RuntimeProfileData m_runProfilerData;
  uint8_t _unknown558[16];
  HksCompilerSettings m_compilerSettings;
  lua_CFunction m_panicFunction;
  plus::LuaObject *m_luaplusObjectList;
  int32_t m_heapAssertionFrequency;
  int32_t m_heapAssertionCount;
  HksLogFunc m_logFunction;
  HksEmergencyGCFailFunc m_emergencyGCFailFunction;
  HksBytecodeEndianness m_bytecodeDumpEndianness;
  uint8_t _padding5B4[4];
};
/*
  ## Important Note on Correct Size

  In alpha build and dedicated server, **0x5B8** _is_ the correct size.

  In client, all relative offsets to the HksGlobal seen in this struct
  definition are correct. However, correct size of `HksGlobal` on client is
  **0x2DC0**. As such, it is likely that there are fields missing in the above
  struct which are present only in client build.

  Define and use a `struct ClientHksGlobal: HksGlobal` when more information is
  available.
*/
ASSERT_SIZE(HksGlobal, 0x5B8);

#pragma pack(pop)

struct lua_State : ChunkHeader {
  enum class Status : uint32_t {
    NEW = 0x1,
    RUNNING = 0x2,
    YIELDED = 0x3,
    DEAD_ERROR = 0x4,
  };

  HksGlobal *m_global;
  CallStack m_callStack;
  ApiStack m_apistack;
  UpValue *pending;
  HksObject globals;
  HksObject m_cEnv;
  CallSite *m_callsites;
  hksInt32 m_numberOfCCalls;
  uint8_t _padding9C[4];
  LuaStateContext *m_context;
  InternString *m_name;
  lua_State *m_next;
  lua_State *m_nextStateStack;
  Status m_status;
  HksError m_error;
};
ASSERT_SIZE(lua_State, 0xC8);

template <typename T> union ObjectTypePool {
  struct {
    T nil;
    T boolean;
    T lightuserdata;
    T number;
    T string;
    T table;
    T function;
    T userdata;
    T thread;
    T ifunction;
    T cfunction;
    T ui64;
    T _struct;
  };
  array<T, static_cast<size_t>(HksObjectType::COUNT)> pool;
};

#pragma pack(push, 1)
struct luaStateMem {
  void *memoryLocation;
  int32_t size;
  qboolean allocated;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct HksStateSettings {
  hksInt32 m_gcPause;
  hksInt32 m_gcStepMul;
  hksSize m_gcEmergencyMemorySize;
  HksEmergencyGCFailFunc m_emergencyGCFailFunction;
  lua_Alloc m_allocator;
  void *m_allocatorData;
  lua_CFunction m_panicFunction;
  HksLogFunc m_logFunction;
  const char *m_name;
  hksUint32 m_initialRegistrySize;
  hksUint32 m_initialRegistryArraySize;
  hksUint32 m_initialGlobalSize;
  hksUint32 m_initialStringTableSize;
  HksCompilerSettings m_compilerSettings;
  // Confirmed present
  hks::Debugger *m_debugObject;
  int32_t m_heapAssertionFrequency;
  uint8_t _padding7C[4];
  lua_CFunction m_gcPolicy;
  HksBytecodeSharingMode m_bytecodeSharingMode;
  HksBytecodeEndianness m_bytecodeDumpEndianness;
  hksUint32 m_gcWeakStackSize;
};
ASSERT_SIZE(HksStateSettings, 0x98);
#pragma pack(pop)

union Libs {
  struct {
    luaL_Reg base;
    luaL_Reg io;
    luaL_Reg package;
    luaL_Reg string;
    luaL_Reg table;
    luaL_Reg math;
    luaL_Reg debug;
    luaL_Reg serialize;
    luaL_Reg profile;
    luaL_Reg os;
    luaL_Reg havokscript;
    luaL_Reg _placeholder;
  };
  const luaL_Reg libs[12];
};

union PkgLoaders {
  struct {
    luaL_Reg preload;
    luaL_Reg luaLoader;
    luaL_Reg CLoader;
    luaL_Reg allInOneLoader;
  };
  luaL_Reg loaders[4];
};

union MathFunctions {
  struct {
    luaL_Reg abs;
    luaL_Reg acos;
    luaL_Reg asin;
    luaL_Reg atan;
    luaL_Reg atan2;
    luaL_Reg ceil;
    luaL_Reg cos;
    luaL_Reg cosh;
    luaL_Reg deg;
    luaL_Reg exp;
    luaL_Reg floor;
    luaL_Reg fmod;
    luaL_Reg frexp;
    luaL_Reg ldexp;
    luaL_Reg log;
    luaL_Reg log10;
    luaL_Reg max;
    luaL_Reg min;
    luaL_Reg modf;
    luaL_Reg pow;
    luaL_Reg rad;
    luaL_Reg random;
    luaL_Reg randomseed;
    luaL_Reg sqrt;
    luaL_Reg sin;
    luaL_Reg sinh;
    luaL_Reg tan;
    luaL_Reg tanh;
  };
  luaL_Reg functions[28];
};

struct OpcodeUnm {
  uint8_t _gap0;
};

struct OpcodeBitOr {
  uint8_t _gap0;
};

struct OpcodeBitAnd {
  uint8_t _gap0;
};

struct OpcodeRShift {
  uint8_t _gap0;
};

struct OpcodeLShift {
  uint8_t _gap0;
};

struct OpcodePow {
  uint8_t _gap0;
};
struct OpcodeMod {
  uint8_t _gap0;
};
struct OpcodeDiv {
  uint8_t _gap0;
};
struct OpcodeMul {
  uint8_t _gap0;
};
struct OpcodeSub {
  uint8_t _gap0;
};
struct OpcodeAdd {
  uint8_t _gap0;
};
} // namespace hks
} // namespace lua
} // namespace ui
} // namespace game