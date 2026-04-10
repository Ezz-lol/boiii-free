#pragma once

#include "core.hpp"
#include "quake.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>
namespace game {

using dvarStrHash_t = uint32_t;

enum dvarType_t {
  DVAR_TYPE_INVALID = 0x0,
  DVAR_TYPE_BOOL = 0x1,
  DVAR_TYPE_FLOAT = 0x2,
  DVAR_TYPE_FLOAT_2 = 0x3,
  DVAR_TYPE_FLOAT_3 = 0x4,
  DVAR_TYPE_FLOAT_4 = 0x5,
  DVAR_TYPE_INT = 0x6,
  DVAR_TYPE_ENUM = 0x7,
  DVAR_TYPE_STRING = 0x8,
  DVAR_TYPE_COLOR = 0x9,
  DVAR_TYPE_INT64 = 0xA,
  DVAR_TYPE_UINT64 = 0xB,
  DVAR_TYPE_LINEAR_COLOR_RGB = 0xC, // RGBA
  DVAR_TYPE_COLOR_XYZ = 0xD,        // CIE XYZ - unused, legacy
  DVAR_TYPE_COLOR_LAB = 0xE,        // CIELAB - unused, legacy
  DVAR_TYPE_SESSIONMODE_BASE_DVAR = 0xF,
  DVAR_TYPE_COUNT = 0x10,
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
  // DVAR_UNKNOWN3x = 1 << 11-13,
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
      std::is_same_v<std::decay_t<T>, int32_t> ||
      std::is_same_v<std::decay_t<T>, uint32_t> ||
      std::is_same_v<std::decay_t<T>, dvarFlags_e> ||
      std::is_same_v<std::decay_t<T>, qboolean> ||
      std::is_same_v<std::decay_t<T>, DvarFlags>;

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
    uint32_t unknown_11_13 : 3;
    uint32_t sessionmode : 1;
    uint32_t unknown_16 : 1;
    uint32_t modvar : 1;
    uint32_t reserved : 15;
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
#pragma pack(pop)

struct dvar_t;

enum DvarSetSource : int32_t {
  DVAR_SOURCE_INTERNAL = 0x0,
  DVAR_SOURCE_EXTERNAL = 0x1,
  DVAR_SOURCE_SCRIPT = 0x2,
};

struct DvarValue {
  union {
    bool enabled;
    int32_t integer;
    uint32_t unsignedInt;
    int64_t integer64;
    uint64_t unsignedInt64;
    float value;
    vec4_t vector;
    const char *string;
    DvarColor color[4];
    const dvar_t *indirect[3];
  } value;

  uint64_t encryptedValue;
};

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

struct dvar_t {
  dvarStrHash_t name;
  const char *debugName;
  const char *description;
  DvarFlags flags;
  dvarType_t type;
  bool modified;
  DvarValue current;
  DvarValue latched;
  DvarValue reset;
  DvarLimits domain;
  dvar_t *hashNext;
};

typedef void (*modifiedCallback)(const dvar_t *);

} // namespace game