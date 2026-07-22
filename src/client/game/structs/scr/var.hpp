#pragma once

#include "game/structs/macros.hpp"
#include "game/structs/quake/vec.hpp"
#include "game/structs/scr/primitives.hpp"
#include "game/structs/scr/builtin/core.hpp"
#include <cstdint>
namespace game {
namespace scr {
namespace var {
/*
  Reverse engineered and defined as enum further below, using the
  var_typename table as reference.
  typedef uint32_t ScrVarType_t;
  Index on VM stack - normally used to access with `stack[-index]` - index from
  top.
*/
typedef uint32_t ScrVarIndex_t;
typedef uint32_t ScrVarNameType_t;
typedef uint64_t ScrVarNameIndex_t;

/*
  There is no typed, defined enumeration in the engine -
  ScrVarType_t is defined as `typedef uint32_t ScrVarType_t;`.

  These type enumerations were extracted from the `var_typename` table in the
  engine.

  These can be easily verified by comparison to clear, inline type value
  matching in the engine, e.g. in BGScr_GetArrayObject for the array type
  value (0x19).
*/
enum class ScrVarType : uint32_t {
  UNDEFINED = 0x00,
  POINTER = 0x01,
  STRING = 0x02,
  LOCALIZED_STRING = 0x03,
  VECTOR = 0x04,
  HASH = 0x05,
  FLOAT = 0x06,
  INT = 0x07,
  UINT64 = 0x08,
  UINTPTR_T = 0x09,
  ENTITY_OFFSET = 0x0A,
  CODEPOS = 0x0B,
  PRECODEPOS = 0x0C,
  // builtin::Builtin function
  API_FUNCTION = 0x0D,
  FUNCTION = 0x0E,
  STACK = 0x0F,
  ANIMATION = 0x10,
  THREAD = 0x11,
  NOTIFY_THREAD = 0x12,
  TIME_THREAD = 0x13,
  CHILD_THREAD = 0x14,
  CLASS = 0x15,
  STRUCT = 0x16,
  REMOVED_ENTITY = 0x17,
  ENTITY = 0x18,
  ARRAY = 0x19,
  REMOVED_THREAD = 0x1A,
  FREE = 0x1B,
  THREAD_LIST = 0x1C,
  ENT_LIST = 0x1D,
  COUNT = 0x1E
};
IMPL_ENUM_OPERATORS(ScrVarType);

typedef ScrVarType ScrVarType_t;

#pragma pack(push, 1)

template <typename T> union ScrVarTypePool {
  struct {
    T t_undefined;
    T t_pointer;
    T t_string;
    T t_localized_string;
    T t_vector;
    T t_hash;
    T t_float;
    T t_int;
    T t_uint64;
    T t_uintptr_t;
    T t_entity_offset;
    T t_codepos;
    T t_precodepos;
    T t_api_function;
    T t_function;
    T t_stack;
    T t_animation;
    T t_thread;
    T t_notify_thread;
    T t_time_thread;
    T t_child_thread;
    T t_class;
    T t_struct;
    T t_removed_entity;
    T t_entity;
    T t_array;
    T t_removed_thread;
    T t_free;
    T t_thread_list;
    T t_ent_list;
  };
  T pool[static_cast<size_t>(ScrVarType::COUNT)];
};
#pragma pack(pop)

// Note: unverified as of initial addition
// Verify before use.
#pragma pack(push, 1)
struct ScrVarStackBuffer_t {
  uint8_t *pos;
  uint8_t *creationPos;
  int32_t waitTime;
  uint16_t size;
  uint16_t bufLen;
  ScrVarIndex_t localId;
  uint8_t buf[1];
  uint8_t _padding[3];
};
ASSERT_SIZE(ScrVarStackBuffer_t, 0x20);
#pragma pack(pop)

// Note: unverified as of initial addition
// Verify before use.
union ScrVarValueUnion_t {
  int32_t intValue;
  uint32_t uintValue;
  ScrString_t string;
  int64_t int64Value;
  uint64_t uint64Value;
  builtin::BuiltinFunction func;
  builtin::BuiltinMethod method;
  uintptr_t uintptrValue;
  float floatValue;
  ScrString_t stringValue;
  union {
    const vec_t *vectorValue;
    const vec3_t *vec3;
    const vec2_t *vec2;
    const vec4_t *vec4;
  };
  uint8_t *codePosValue;
  ScrVarIndex_t pointerValue;
  ScrVarStackBuffer_t *stackValue;
  struct {
    ScrVarIndex_t firstChild;
    ScrVarIndex_t lastChild;
  };
};
ASSERT_SIZE(ScrVarValueUnion_t, 0x8);

struct ScrVar_t;

#pragma pack(push, 1)
struct ScrVarValue_t {
  ScrVarValueUnion_t u;
  ScrVarType_t type;
  uint8_t _padding[4];

  const ScrVar_t *var() const noexcept;
  ScrVar_t *var() noexcept;
  operator const ScrVar_t *() const noexcept { return var(); }
  operator ScrVar_t *() noexcept { return var(); }
};
ASSERT_SIZE(ScrVarValue_t, 0x10);
#pragma pack(pop)

struct ScrVarEntityInfo_t {
  uint16_t classnum;
  uint16_t clientNum;
};

#pragma pack(push, 1)
struct ScrVar_t {
  ScrVarValue_t value;
  struct {
    int32_t nameType : 3;
    uint32_t flags : 5;
    uint32_t refCount : 24;
  };
  uint8_t _padding14[4];
  union {
    uint64_t object_o;
    uint32_t size;
    EntRefUnion entRefUnion;
    ScrVarIndex_t nextEntId;
    ScrVarIndex_t self;
    ScrVarIndex_t free;
  } o;
  union {
    uint32_t object_w;
    ScrVarEntityInfo_t varEntityInfo;
    ScrVarCanonicalName_t notifyName;
    uint32_t waitTime;
  } w;
  uint8_t _padding24[4];
  ScrVarNameIndex_t nameIndex;
  ScrVarIndex_t nextSibling;
  ScrVarIndex_t prevSibling;
  ScrVarIndex_t parentId;
  ScrVarIndex_t nameSearchHashList;
};
ASSERT_SIZE(ScrVar_t, 0x40);
#pragma pack(pop)

inline const ScrVar_t *ScrVarValue_t::var() const noexcept {
  return reinterpret_cast<const ScrVar_t *>(reinterpret_cast<uintptr_t>(this) -
                                            offsetof(ScrVar_t, value) /* 0 */);
}

inline ScrVar_t *ScrVarValue_t::var() noexcept {
  return reinterpret_cast<ScrVar_t *>(reinterpret_cast<uintptr_t>(this) -
                                      offsetof(ScrVar_t, value) /* 0 */);
}

#pragma pack(push, 1)
struct scrVarGlob_t {
  ScrVarIndex_t *scriptNameSearchHashList;
  uint8_t _padding08[0x78];
  ScrVar_t *scriptVariables;
  uint8_t _padding88[0x78];
};
ASSERT_SIZE(scrVarGlob_t, 0x100);
ASSERT_OFFSET(scrVarGlob_t, scriptVariables, 0x80);
#pragma pack(pop)

typedef ScrPool<scrVarGlob_t> ScrVarGlobPool;

#pragma pack(push, 1)
struct scrVarPub_t {
  const char *fieldBuffer;
  bool developer;
  bool evaluate;
  uint8_t _padding0A[6];
  const char *error_message;
  uint32_t time;
  ScrVarIndex_t timeArrayId;
  ScrVarIndex_t pauseArrayId;
  ScrVarIndex_t worldId;
  ScrVarIndex_t classesId;
  ScrVarIndex_t levelId;
  ScrVarIndex_t gameId;
  ScrVarIndex_t animId;
  ScrVarIndex_t freeEntList;
  ScrVarIndex_t tempVariable;
  bool bInited;
  uint8_t _padding41[3];
  uint32_t animChecksum;
  scrChecksum_t checksum;
  uint32_t entId;
  ScrVarNameIndex_t entFieldNameIndex;
  hunk::HunkUser *programHunkUser;
  uint8_t *programBuffer;
  uint8_t *endScriptBuffer;
};

ASSERT_SIZE(scrVarPub_t, 0x78);
ASSERT_OFFSET(scrVarPub_t, programHunkUser, 0x60);
ASSERT_OFFSET(scrVarPub_t, programBuffer, 0x68);
ASSERT_OFFSET(scrVarPub_t, endScriptBuffer, 0x70);
ASSERT_OFFSET(scrVarPub_t, animChecksum, 0x44);
ASSERT_OFFSET(scrVarPub_t, checksum, 0x48);
ASSERT_OFFSET(scrVarPub_t, entId, 0x54);
ASSERT_OFFSET(scrVarPub_t, entFieldNameIndex, 0x58);
ASSERT_OFFSET(scrVarPub_t, bInited, 0x40);
ASSERT_OFFSET(scrVarPub_t, error_message, 0x10);
ASSERT_OFFSET(scrVarPub_t, timeArrayId, 0x1C);
ASSERT_OFFSET(scrVarPub_t, pauseArrayId, 0x20);
ASSERT_OFFSET(scrVarPub_t, worldId, 0x24);
ASSERT_OFFSET(scrVarPub_t, classesId, 0x28);
ASSERT_OFFSET(scrVarPub_t, levelId, 0x2C);
ASSERT_OFFSET(scrVarPub_t, gameId, 0x30);
ASSERT_OFFSET(scrVarPub_t, animId, 0x34);
ASSERT_OFFSET(scrVarPub_t, freeEntList, 0x38);
ASSERT_OFFSET(scrVarPub_t, tempVariable, 0x3C);
ASSERT_OFFSET(scrVarPub_t, fieldBuffer, 0x00);
ASSERT_OFFSET(scrVarPub_t, developer, 0x08);
ASSERT_OFFSET(scrVarPub_t, evaluate, 0x09);
ASSERT_OFFSET(scrVarPub_t, time, 0x18);
#pragma pack(pop)

typedef ScrPool<scrVarPub_t> ScrVarPubPool;

constexpr ScrPool<ScrVarIndex_t> SCRIPTVARIABLE_POOL_SIZE = {.server = 130000,
                                                             .client = 65000};
} // namespace var
} // namespace scr
} // namespace game