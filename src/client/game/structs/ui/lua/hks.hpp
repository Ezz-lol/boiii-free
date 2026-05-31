#pragma once

#include "../../core.hpp"

namespace game {
namespace ui {
namespace lua {
namespace hks {

enum class luaReturnCount_e : uint32_t {
  LUA_RETURNCOUNT_NONE = 0x0,
  LUA_RETURNCOUNT_ONE = 0x1,
  LUA_RETURNCOUNT_TWO = 0x2,
  LUA_RETURNCOUNT_THREE = 0x3,
  LUA_RETURNCOUNT_FOUR = 0x4,
  LUA_RETURNCOUNT_FIVE = 0x5,
  LUA_RETURNCOUNT_SIX = 0x6,
  LUA_RETURNCOUNT_SEVEN = 0x7,
  LUA_RETURNCOUNT_EIGHT = 0x8,
  LUA_RETURNCOUNT_NINE = 0x9,
};

struct lua_State;
struct HashTable;
struct StringTable;
struct cclosure;

using hksBool = int;
using hksChar = char;
using hksByte = uint8_t;
using hksShort16 = int16_t;
using hksUshort16 = uint16_t;
using HksNumber = float;
using hksInt32 = int;
using hksUint32 = unsigned int;
using hksInt64 = int64_t;
using hksUint64 = uint64_t;

using HksGcCost = int;

using hksSize = size_t;
using lua_Alloc = void *(*)(void *, void *, size_t, size_t);
using lua_CFunction = hksInt32 (*)(lua_State *);

typedef hksInt32 lua_Integer;

struct GenericChunkHeader {
  hksSize m_flags;
};

struct ChunkHeader : GenericChunkHeader {
  ChunkHeader *m_next;
};

struct ChunkList {
  ChunkHeader m_head;
};

struct UserData : ChunkHeader {
  uint64_t m_envAndSizeOffsetHighBits;
  uint64_t m_metaAndSizeOffsetLowBits;
  char m_data[8];
};

struct InternString {
  uint64_t m_flags;
  uint64_t m_lengthbits;
  uint32_t m_hash;
  char m_data[30];
};

union HksValue {
  cclosure *cClosure;
  void *closure;
  UserData *userData;
  HashTable *table;
  void *tstruct;
  InternString *str;
  void *thread;
  void *ptr;
  float number;
  uint32_t native;
  bool boolean;
};

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
  COUNT = 0xE,
};

struct HksObject {
  HksObjectType t;
  HksValue v;
};

struct hksInstruction {
  uint32_t code;
};

struct ActivationRecord {
  HksObject *m_base;
  const hksInstruction *m_returnAddress;
  int16_t m_tailCallDepth;
  int16_t m_numVarargs;
  int32_t m_numExpectedReturns;
};

struct CallStack {
  ActivationRecord *m_records;
  ActivationRecord *m_lastrecord;
  ActivationRecord *m_current;
  const hksInstruction *m_current_lua_pc;
  const hksInstruction *m_hook_return_addr;
  int32_t m_hook_level;
};

struct ApiStack {
  HksObject *top;
  HksObject *base;
  HksObject *alloc_top;
  HksObject *bottom;
};

struct UpValue : ChunkHeader {
  HksObject m_storage;
  HksObject *loc;
  UpValue *m_next;
};

struct CallSite {
  float128_t m_jumpBuffer[16];
  CallSite *m_prev;
};

enum class Status : uint32_t {
  NEW = 0x1,
  RUNNING = 0x2,
  YIELDED = 0x3,
  DEAD_ERROR = 0x4,
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
  int32_t event;
  const char *name;
  const char *namewhat;
  const char *what;
  const char *source;
  int32_t currentline;
  int32_t nups;
  int32_t nparams;
  int32_t ishksfunc;
  int32_t linedefined;
  int32_t lastlinedefined;
  char short_src[512];
  int32_t callstack_level;
  int32_t is_tail_call;
};

using lua_function = fastcall_t<int, lua_State *>;

struct luaL_Reg {
  const char *name;
  lua_function function;
};

struct Node {
  HksObject m_key;
  HksObject m_value;
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
  uint32_t m_count;
  uint32_t m_mask;
  StringPinner *m_pinnedStrings;
};

struct Metatable {};

struct HashTable : ChunkHeader {
  Metatable *m_meta;
  uint32_t m_version;
  uint32_t m_mask;
  Node *m_hashPart;
  HksObject *m_arrayPart;
  uint32_t m_arraySize;
  Node *m_freeNode;
};

struct cclosure : ChunkHeader {
  lua_function m_function;
  HashTable *m_env;
  int16_t m_numUpvalues;
  int16_t m_flags;
  InternString *m_name;
  HksObject m_upvalues[1];
};

enum class HksCompilerSettings_BytecodeSharingFormat : uint32_t {
  DEFAULT = 0x0,
  INPLACE = 0x1,
  REFERENCED = 0x2,
};

enum class HksCompilerSettings_IntLiteralOptions : uint32_t {
  INT_LITERALS_NONE = 0x0,
  INT_LITERALS_LUD = 0x1,
  INT_LITERALS_32BIT = 0x1,
  INT_LITERALS_UI64 = 0x2,
  INT_LITERALS_64BIT = 0x2,
  INT_LITERALS_ALL = 0x3,
};

struct HksCompilerSettings {
  int32_t m_emitStructCode;
  const char **m_stripNames;
  int32_t m_emitGlobalMemoization;
  int32_t _m_isHksGlobalMemoTestingMode;
  HksCompilerSettings_BytecodeSharingFormat m_bytecodeSharingFormat;
  HksCompilerSettings_IntLiteralOptions m_enableIntLiterals;
  int32_t (*m_debugMap)(const char *, int);
};

enum class HksBytecodeSharingMode : int64_t { OFF = 0, ON = 1, SECURE = 2 };

struct HksGcWeights {
  int32_t m_removeString;
  int32_t m_finalizeUserdataNoMM;
  int32_t m_finalizeUserdataGcMM;
  int32_t m_cleanCoroutine;
  int32_t m_removeWeak;
  int32_t m_markObject;
  int32_t m_traverseString;
  int32_t m_traverseUserdata;
  int32_t m_traverseCoroutine;
  int32_t m_traverseWeakTable;
  int32_t m_freeChunk;
  int32_t m_sweepTraverse;
};

struct GarbageCollector_Stack {
  void *m_storage;
  uint32_t m_numEntries;
  uint32_t m_numAllocated;
};

struct ProtoList {
  void **m_protoList;
  uint16_t m_protoSize;
  uint16_t m_protoAllocSize;
};

struct MemoryManager;

struct GarbageCollector {
  struct ResumeStack {
    void *m_storage;
    hksInt32 m_numEntries;
    hksUint32 m_numAllocated;
  };

  struct GreyStack {
    HksObject *m_storage;
    hksSize m_numEntries;
    hksSize m_numAllocated;
  };

  struct RemarkStack {
    HashTable **m_storage;
    hksSize m_numAllocated;
    hksSize m_numEntries;
  };

  struct WeakStack_Entry {
    hksInt32 m_weakness;
    HashTable *m_table;
  };

  struct WeakStack {
    WeakStack_Entry *m_storage;
    hksInt32 m_numEntries;
    hksUint32 m_numAllocated;
  };

  HksGcCost m_target;
  HksGcCost m_stepsLeft;
  HksGcCost m_stepLimit;
  HksGcWeights m_costs;
  HksGcCost m_unit;
  void *m_jumpPoint;
  lua_State *m_mainState;
  lua_State *m_finalizerState;
  MemoryManager *m_memory;
  void *m_emergencyGCMemory;
  hksInt32 m_phase;
  ResumeStack m_resumeStack;
  GreyStack m_greyStack;
  RemarkStack m_remarkStack;
  WeakStack m_weakStack;
  hksBool m_finalizing;
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
  lua_CFunction m_gcPolicy;
  hksSize m_pauseTriggerMemoryUsage;
  hksInt32 m_stepTriggerCountdown;
  hksUint32 m_stringTableIndex;
  hksUint32 m_stringTableSize;
  UserData *m_lastBlackUD;
  UserData *m_activeUD;
};

enum class MemoryManager_ChunkColor : uint32_t {
  RED = 0x0,
  BLACK = 0x1,
};

enum class Hks_DeleteCheckingMode : uint32_t {
  OFF = 0x0,
  ACCURATE = 0x1,
  SAFE = 0x2,
};

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

struct StaticStringCache {
  HksObject m_objects[41];
};

enum class HksBytecodeEndianness : uint32_t {
  DEFAULT = 0x0,
  BIG = 0x1,
  LITTLE = 0x2,
};

struct RuntimeProfileData_Stats {
  uint64_t hksTime;
  uint64_t callbackTime;
  uint64_t gcTime;
  uint64_t cFinalizerTime;
  uint64_t compilerTime;
  uint32_t hkssTimeSamples;
  uint32_t callbackTimeSamples;
  uint32_t gcTimeSamples;
  uint32_t compilerTimeSamples;
  uint32_t num_newuserdata;
  uint32_t num_tablerehash;
  uint32_t num_pushstring;
  uint32_t num_pushcfunction;
  uint32_t num_newtables;
};

struct RuntimeProfileData {
  int64_t stackDepth;
  int64_t callbackDepth;
  uint64_t lastTimer;
  RuntimeProfileData_Stats frameStats;
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

struct HksGlobal {
  MemoryManager m_memory;
  GarbageCollector m_collector;
  StringTable m_stringTable;
  int64_t padding3;
  HksBytecodeSharingMode m_bytecodeSharingMode;
  int32_t padding;
  HksObject m_registry;
  ChunkList m_userDataList;
  lua_State *m_root;
  StaticStringCache m_staticStringCache;
  void *m_debugger;
  void *m_profiler;
  RuntimeProfileData m_runProfilerData;
  HksCompilerSettings m_compilerSettings;
  int32_t (*m_panicFunction)(lua_State *);
  void *m_luaplusObjectList;
  int32_t m_heapAssertionFrequency;
  int32_t m_heapAssertionCount;
  void (*m_logFunction)(lua_State *, const char *, ...);
  void (*m_emergencyGCFailFunction)(lua_State *, size_t);
  HksBytecodeEndianness m_bytecodeDumpEndianness;
  int32_t padding2;
};

struct lua_State {
  ChunkHeader baseclass;
  HksGlobal *m_global;
  CallStack m_callStack;
  ApiStack m_apistack;
  UpValue *pending;
  HksObject globals;
  HksObject m_cEnv;
  CallSite *m_callsites;
  int32_t m_numberOfCCalls;
  void *m_context;
  InternString *m_name;
  lua_State *m_nextState;
  lua_State *m_nextStateStack;
  Status m_status;
  HksError m_error;
};
} // namespace hks
} // namespace lua
} // namespace ui
} // namespace game