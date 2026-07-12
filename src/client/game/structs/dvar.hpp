#pragma once

#include "core.hpp"
#include "macros.hpp"
#include "func.hpp"
#include "quake/vec.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace game {

typedef uint32_t dvarStrHash_t;

dvarStrHash_t Dvar_GenerateHash_Impl(const char *str);

enum class dvarType_t : uint32_t {
  INVALID = 0x0,
  BOOL = 0x1,
  FLOAT = 0x2,
  FLOAT_2 = 0x3,
  FLOAT_3 = 0x4,
  FLOAT_4 = 0x5,
  INT = 0x6,
  ENUM = 0x7,
  STRING = 0x8,
  COLOR = 0x9,
  INT64 = 0xA,
  UINT64 = 0xB,
  LINEAR_COLOR_RGB = 0xC, // RGBA
  COLOR_XYZ = 0xD,        // CIE XYZ - unused, legacy
  COLOR_LAB = 0xE,        // CIELAB - unused, legacy
  SESSIONMODE_BASE_DVAR = 0xF,
  COUNT = 0x10,
};

enum dvarFlags_e : uint32_t {
  DVAR_NONE = 0,
  DVAR_ARCHIVE = 1 << 0,
  DVAR_USERINFO = 1 << 1,
  DVAR_SERVERINFO = 1 << 2,
  DVAR_SYSTEMINFO = 1 << 3,
  DVAR_LATCH = 1 << 4,
  DVAR_ROM = 1 << 5,
  DVAR_SAVED = 1 << 6,
  DVAR_INIT = 1 << 7,
  DVAR_CHEAT = 1 << 8,
  // DVAR_UNKNOWN = 1 << 9,
  DVAR_EXTERNAL = 1 << 10,
  // DVAR_UNKNOWN3x = 1 << 11-14,
  DVAR_SESSIONMODE = 1 << 15,
  // Modvars have flags |= DVAR_MODVAR|DVAR_SESSIONMODE -
  // dvars specific to mod session
  DVAR_MODVAR = 1 << 17
};

#pragma pack(push, 1)
union DvarFlags {
private:
  template <typename T>
  static constexpr bool is_allowed_flag_v =
      std::is_convertible<T, uint32_t>::value;

public:
  struct {
    uint32_t archive : 1;
    uint32_t userinfo : 1;
    uint32_t serverinfo : 1;
    uint32_t systeminfo : 1;
    uint32_t latch : 1;
    uint32_t rom : 1;
    uint32_t saved : 1;
    uint32_t init : 1;
    uint32_t cheat : 1;
    uint32_t unknown_9 : 1;
    uint32_t external : 1;
    uint32_t unknown_11_14 : 4;
    uint32_t sessionmode : 1;
    uint32_t unknown_16 : 1;
    uint32_t modvar : 1;
    uint32_t reserved : 14;
  };
  uint32_t _raw;

  constexpr DvarFlags() noexcept : _raw(0) {}

  template <typename T, typename = std::enable_if_t<is_allowed_flag_v<T>>>
  constexpr DvarFlags(T raw) noexcept : _raw(static_cast<uint32_t>(raw)) {}

  constexpr operator uint32_t() const noexcept { return _raw; }

  constexpr DvarFlags operator~() const noexcept { return DvarFlags(~_raw); }

  template <typename T, typename = std::enable_if_t<is_allowed_flag_v<T>>>
  constexpr DvarFlags &operator|=(T rhs) noexcept {
    _raw |= static_cast<uint32_t>(rhs);
    return *this;
  }

  template <typename T, typename = std::enable_if_t<is_allowed_flag_v<T>>>
  constexpr DvarFlags &operator&=(T rhs) noexcept {
    _raw &= static_cast<uint32_t>(rhs);
    return *this;
  }

  template <typename T, typename = std::enable_if_t<is_allowed_flag_v<T>>>
  constexpr DvarFlags &operator^=(T rhs) noexcept {
    _raw ^= static_cast<uint32_t>(rhs);
    return *this;
  }

  // DvarFlags [op] T
  template <typename T, typename = std::enable_if_t<is_allowed_flag_v<T>>>
  friend constexpr DvarFlags operator|(DvarFlags lhs, T rhs) noexcept {
    return DvarFlags(lhs._raw | static_cast<uint32_t>(rhs));
  }

  // T [op] DvarFlags (DvarFlags == T excluded - prevents ambiguity)
  template <typename T, typename = std::enable_if_t<
                            is_allowed_flag_v<T> &&
                            !std::is_same_v<std::decay_t<T>, DvarFlags>>>
  friend constexpr DvarFlags operator|(T lhs, DvarFlags rhs) noexcept {
    return DvarFlags(static_cast<uint32_t>(lhs) | rhs._raw);
  }

  template <typename T, typename = std::enable_if_t<is_allowed_flag_v<T>>>
  friend constexpr DvarFlags operator&(DvarFlags lhs, T rhs) noexcept {
    return DvarFlags(lhs._raw & static_cast<uint32_t>(rhs));
  }

  template <typename T, typename = std::enable_if_t<
                            is_allowed_flag_v<T> &&
                            !std::is_same_v<std::decay_t<T>, DvarFlags>>>
  friend constexpr DvarFlags operator&(T lhs, DvarFlags rhs) noexcept {
    return DvarFlags(static_cast<uint32_t>(lhs) & rhs._raw);
  }

  template <typename T, typename = std::enable_if_t<is_allowed_flag_v<T>>>
  friend constexpr DvarFlags operator^(DvarFlags lhs, T rhs) noexcept {
    return DvarFlags(lhs._raw ^ static_cast<uint32_t>(rhs));
  }

  template <typename T, typename = std::enable_if_t<
                            is_allowed_flag_v<T> &&
                            !std::is_same_v<std::decay_t<T>, DvarFlags>>>
  friend constexpr DvarFlags operator^(T lhs, DvarFlags rhs) noexcept {
    return DvarFlags(static_cast<uint32_t>(lhs) ^ rhs._raw);
  }

  template <typename T, typename = std::enable_if_t<is_allowed_flag_v<T>>>
  friend constexpr bool operator==(DvarFlags lhs, T rhs) noexcept {
    return lhs._raw == static_cast<uint32_t>(rhs);
  }

  template <typename T, typename = std::enable_if_t<
                            is_allowed_flag_v<T> &&
                            !std::is_same_v<std::decay_t<T>, DvarFlags>>>
  friend constexpr bool operator==(T lhs, DvarFlags rhs) noexcept {
    return static_cast<uint32_t>(lhs) == rhs._raw;
  }

  template <typename T, typename = std::enable_if_t<is_allowed_flag_v<T>>>
  friend constexpr bool operator!=(DvarFlags lhs, T rhs) noexcept {
    return lhs._raw != static_cast<uint32_t>(rhs);
  }

  template <typename T, typename = std::enable_if_t<
                            is_allowed_flag_v<T> &&
                            !std::is_same_v<std::decay_t<T>, DvarFlags>>>
  friend constexpr bool operator!=(T lhs, DvarFlags rhs) noexcept {
    return static_cast<uint32_t>(lhs) != rhs._raw;
  }
};
ASSERT_SIZE(DvarFlags, sizeof(uint32_t));
#pragma pack(pop)

/*
  Labeled as rgba in engine.
  XYZ and LAB color types, as labeled by dvarType_t,
  seem to be legacy - unused in engine.
*/
#pragma pack(push, 1)
union DvarColor {
  // Linear RGB
  struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
  };

  // CIE XYZ
  struct {
    uint8_t x; // Red-Green axis
    uint8_t y; // Luminance
    uint8_t z; // Blue-Yellow axis
  } xyz;

  // CIELAB
  struct {
    uint8_t l; // Luminance
    uint8_t a; // Green-Red axis
    uint8_t b; // Blue-Yellow axis
  } lab;

  uint8_t raw[4];

  constexpr DvarColor() noexcept : raw{0, 0, 0, 0} {}
  constexpr DvarColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
      : r(r), g(g), b(b), a(a) {}
  constexpr DvarColor(uint8_t x, uint8_t y, uint8_t z) noexcept
      : xyz{x, y, z} {}

  // operators for implicit conversion to uint8_t[4], array semantics
  constexpr operator uint8_t *() noexcept { return raw; }
  constexpr operator const uint8_t *() const noexcept { return raw; }
  constexpr uint8_t &operator[](size_t index) noexcept { return raw[index]; }
  constexpr const uint8_t &operator[](size_t index) const noexcept {
    return raw[index];
  }
};
ASSERT_SIZE(DvarColor, 4);
#pragma pack(pop)

struct dvar_t;
struct dvar_t_cl;

enum class DvarSetSource : uint32_t {
  INTERNAL = 0x0,
  EXTERNAL = 0x1,
  SCRIPT = 0x2,
};

#pragma pack(push, 1)
template <typename T_Dvar> union TemplateDvarValue {
  bool _enabled;
  int32_t _integer;
  uint32_t _unsignedInt;
  int64_t _integer64;
  uint64_t _unsignedInt64;
  float _value;
  vec4_t _vector;
  const char *_string;
  DvarColor _color;
  const T_Dvar *indirect[3];

  inline constexpr bool enabled() const { return _enabled; }

  inline constexpr int32_t integer() const { return _integer; }

  inline constexpr uint32_t unsignedInt() const { return _unsignedInt; }

  inline constexpr int64_t integer64() const { return _integer64; }

  inline constexpr uint64_t unsignedInt64() const { return _unsignedInt64; }

  inline constexpr float value() const { return _value; }

  inline constexpr vec4_t vector() const { return _vector; }
  inline constexpr const char *string() const { return _string; }
  inline constexpr DvarColor color() const { return _color; }
};
typedef TemplateDvarValue<dvar_t> DvarValue;
struct EncryptionCapableDvarValue {
  TemplateDvarValue<dvar_t_cl> _value;
  uint64_t encryptedValue;

  // Fields renamed with "_" prefix in favor of consolidated method interface
  // to allow identical usage of both Dvarvalue and EncryptionCapableDvarValue.
  // Otherwise, field accesses differ, and we cannot use the same std::visit
  // callbacks to access them identically.
  inline constexpr bool enabled() const { return _value._enabled; }

  inline constexpr int32_t integer() const { return _value._integer; }

  inline constexpr uint32_t unsignedInt() const { return _value._unsignedInt; }

  inline constexpr int64_t integer64() const { return _value._integer64; }

  inline constexpr uint64_t unsignedInt64() const {
    return _value._unsignedInt64;
  }

  inline constexpr float value() const { return _value._value; }

  inline constexpr vec4_t vector() const { return _value._vector; }
  inline constexpr const char *string() const { return _value._string; }
  inline constexpr DvarColor color() const { return _value._color; }
};
ASSERT_SIZE(DvarValue, 0x18);
ASSERT_SIZE(EncryptionCapableDvarValue, 0x20);
#pragma pack(pop)

union DvarLimits {
  struct {
    int32_t stringCount;
    const char **strings;
  } enumeration;

  struct {
    int32_t min;
    int32_t max;
  } integer;

  struct {
    int64_t min;
    int64_t max;
  } integer64;

  struct {
    uint64_t min;
    uint64_t max;
  } unsignedInt64;

  struct {
    float min;
    float max;
  } value;

  struct {
    vec_t min;
    vec_t max;
  } vector;
};
ASSERT_SIZE(DvarLimits, 0x10);

#pragma pack(push, 1)
struct dvar_t {
public:
  dvarStrHash_t name;
  uint8_t _padding04[4];
  const char *debugName;
  const char *description;
  DvarFlags flags;
  dvarType_t type;
  bool modified;
  uint8_t _padding21[7];
  DvarValue current;
  DvarValue latched;
  DvarValue reset;
  DvarLimits domain;
  dvar_t *hashNext;
};
#pragma pack(pop)

ASSERT_OFFSET(dvar_t, debugName, 0x8);
ASSERT_OFFSET(dvar_t, description, 0x10);
ASSERT_OFFSET(dvar_t, flags, 0x18);
ASSERT_OFFSET(dvar_t, type, 0x1C);
ASSERT_OFFSET(dvar_t, modified, 0x20);
ASSERT_OFFSET(dvar_t, current, 0x28);
ASSERT_SIZE(dvar_t, 0x88);

#pragma pack(push, 1)
struct dvar_t_cl {
public:
  dvarStrHash_t name;
  uint8_t _padding04[4];
  const char *debugName;
  const char *description;
  DvarFlags flags;
  dvarType_t type;
  bool modified;
  uint8_t _padding21[7];
  EncryptionCapableDvarValue current;
  EncryptionCapableDvarValue latched;
  EncryptionCapableDvarValue reset;
  DvarLimits domain;
  dvar_t_cl *hashNext;
};
#pragma pack(pop)
ASSERT_OFFSET(dvar_t_cl, debugName, 0x8);
ASSERT_OFFSET(dvar_t_cl, description, 0x10);
ASSERT_OFFSET(dvar_t_cl, flags, 0x18);
ASSERT_OFFSET(dvar_t_cl, type, 0x1C);
ASSERT_OFFSET(dvar_t_cl, modified, 0x20);
ASSERT_OFFSET(dvar_t_cl, current, 0x28);
ASSERT_SIZE(dvar_t_cl, 0xA0);

/*
  TODO:
  Change this to:
  ```cpp
  union EngineDependentDvar {
    const dvar_t *dvar;
    const dvar_t_cl* dvar_cl;
  };
  ```

  Provide an interface of `inline constexpr` methods
  to allow an identical field API and `operator`s to
  allow implicit cast to/from the corresponding pointer where needed.

  This should already be done, but refactoring from the current `std::visit`-
  based API consolidation will be substantial, so this is left for the
  future.
*/
typedef EngineDependent<const dvar_t_cl *, const dvar_t *> EngineDependentDvar;
typedef EngineDependent<dvar_t_cl *, dvar_t *> EngineDependentDvarMut;

template <typename T_Dvar> struct TemplateDvarPool {
  T_Dvar pool[0x2000];

  inline constexpr const T_Dvar &operator[](size_t index) const {
    return pool[index];
  }
  inline constexpr T_Dvar &operator[](size_t index) { return pool[index]; }
};
typedef TemplateDvarPool<dvar_t> DvarPool;
ASSERT_SIZE(DvarPool, 0x110000);

typedef TemplateDvarPool<dvar_t_cl> DvarPool_cl;
ASSERT_SIZE(DvarPool_cl, 0x140000);

typedef fastcall_t<void(const dvar_t *dvar)> modifiedCallback;

#pragma pack(push, 1)
template <typename T_Dvar> struct TemplateDvarCallBack {
  bool needsCallback;
  uint8_t _padding01[7];
  modifiedCallback callback;
  const T_Dvar *dvar;
};
#pragma pack(pop)

typedef TemplateDvarCallBack<dvar_t> dvarCallBack_t;
ASSERT_SIZE(dvarCallBack_t, 0x18);

typedef TemplateDvarCallBack<dvar_t_cl> dvarCallBack_t_cl;
ASSERT_SIZE(dvarCallBack_t_cl, 0x18);

template <typename T_Dvar> struct _DvarCallbackPool {
  using T_Cb = TemplateDvarCallBack<T_Dvar>;
  T_Cb pool[0x100];

  inline constexpr const T_Cb &operator[](size_t index) const {
    return pool[index];
  }
  inline constexpr T_Cb &operator[](size_t index) { return pool[index]; }
};
typedef _DvarCallbackPool<dvar_t> DvarCallbackPool;
ASSERT_SIZE(DvarCallbackPool, 0x1800);

typedef _DvarCallbackPool<dvar_t_cl> DvarCallbackPool_cl;
ASSERT_SIZE(DvarCallbackPool_cl, 0x1800);

constexpr dvarStrHash_t DVAR_HASH_TABLE_LEN = 0x800;
constexpr dvarStrHash_t DVAR_HASH_MASK = DVAR_HASH_TABLE_LEN - 1;
template <typename T_Dvar> struct TemplateDvarHashTable {
  T_Dvar *table[0x800];

  inline constexpr T_Dvar *get(dvarStrHash_t hash) const {
    T_Dvar *entry = table[hash & DVAR_HASH_MASK];
    while (entry && entry->name != hash) {
      entry = entry->hashNext;
    }
    return entry;
  }

  inline constexpr bool contains(dvarStrHash_t hash) const {
    T_Dvar *entry = get(hash);
    return entry != nullptr;
  }

  inline constexpr bool contains(const char *name) const {
    return contains(Dvar_GenerateHash_Impl(name));
  }

  inline constexpr T_Dvar *get(const char *name) const {
    return get(Dvar_GenerateHash_Impl(name));
  }

  inline constexpr T_Dvar *operator[](dvarStrHash_t hash) const {
    return get(hash);
  }
  inline constexpr T_Dvar *operator[](dvarStrHash_t hash) { return get(hash); }

  inline constexpr T_Dvar *operator[](const char *name) const {
    return get(name);
  }

  inline constexpr T_Dvar *operator[](const char *name) { return get(name); }
};
typedef TemplateDvarHashTable<dvar_t> DvarHashTable;
typedef TemplateDvarHashTable<dvar_t_cl> DvarHashTable_cl;
ASSERT_SIZE(DvarHashTable, 0x4000);
ASSERT_SIZE(DvarHashTable_cl, 0x4000);

enum class dvar_cmd_t : uint32_t {
  CG_OBJECTIVE_TEXT = 0x0,
  COMPASS = 0x1,
  DRAW_TALK = 0x2,
  FOCAL_LENGTH = 0x3,
  HUD_HARDCORE = 0x4,
  PLAYER_SPRINT_TIME = 0x5,
  MINI_SCOREBOARD_HIDE = 0x6,
  NUM_LIVES = 0x7,
  PLAYER_PUSH_AMOUNT = 0x8,
  PLAYER_STEP_ON_ACTORS = 0x9,
  SCRIPT_MAIN_MENU = 0xA,
  THIRD_PERSON = 0xB,
  THIRD_PERSON_ANGLE = 0xC,
  COUNT = 0xD,
};

} // namespace game