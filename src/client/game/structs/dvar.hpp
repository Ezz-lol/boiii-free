#pragma once

#include <macros.hpp>
#include "macros.hpp"
#include "func.hpp"
#include "quake/vec.hpp"
#include "core.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <optional>
#include <string>

namespace game {

template <typename T>
concept PtrLike = std::is_convertible_v<T, uintptr_t>;

struct dvar_t;
struct encryptedDvar_t;

enum class DvarSetSource : uint32_t {
  INTERNAL = 0x0,
  EXTERNAL = 0x1,
  SCRIPT = 0x2,
};

bool is_server();
bool is_client();

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

template <typename T>
concept DvarFlagLike = IntegralLike<T, uint32_t>;

#pragma pack(push, 1)
union DvarFlags {
  uint32_t _raw;

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

  inline constexpr operator uint32_t() const noexcept { return _raw; }

  template <DvarFlagLike T> static constexpr DvarFlags from(T val) noexcept {
    return DvarFlags{static_cast<uint32_t>(val)};
  }
  template <DvarFlagLike T> inline constexpr void add(T flags) noexcept {
    _raw |= static_cast<uint32_t>(flags);
  }

  template <DvarFlagLike T> inline constexpr void remove(T flags) noexcept {
    _raw &= static_cast<uint32_t>(~flags);
  }

  inline constexpr void clear() noexcept { _raw = 0; }

  template <DvarFlagLike T> inline constexpr void set(T flags) noexcept {
    _raw = static_cast<uint32_t>(flags);
  }

  template <DvarFlagLike T>
  inline constexpr DvarFlags add(T flags) const noexcept {
    return DvarFlags{_raw | static_cast<uint32_t>(flags)};
  }

  template <DvarFlagLike T>
  inline constexpr DvarFlags remove(T flags) const noexcept {
    return DvarFlags{_raw & static_cast<uint32_t>(~flags)};
  }

  inline constexpr DvarFlags clear() const noexcept { return DvarFlags{0}; }

  template <DvarFlagLike T>
  inline constexpr DvarFlags set(T flags) const noexcept {
    return DvarFlags{static_cast<uint32_t>(flags)};
  }
};
#pragma pack(pop)

inline constexpr DvarFlags operator~(DvarFlags flag) noexcept {
  return DvarFlags{~flag._raw};
}

template <DvarFlagLike T>
inline constexpr DvarFlags &operator|=(DvarFlags &lhs, T rhs) noexcept {
  lhs._raw |= static_cast<uint32_t>(rhs);
  return lhs;
}

template <DvarFlagLike T>
inline constexpr DvarFlags &operator&=(DvarFlags &lhs, T rhs) noexcept {
  lhs._raw &= static_cast<uint32_t>(rhs);
  return lhs;
}

template <DvarFlagLike T>
inline constexpr DvarFlags &operator^=(DvarFlags &lhs, T rhs) noexcept {
  lhs._raw ^= static_cast<uint32_t>(rhs);
  return lhs;
}

template <DvarFlagLike T>
inline constexpr DvarFlags operator|(DvarFlags lhs, T rhs) noexcept {
  return DvarFlags{lhs._raw | static_cast<uint32_t>(rhs)};
}

template <DvarFlagLike T, typename = std::enable_if_t<
                              !std::is_same_v<std::decay_t<T>, DvarFlags>>>
inline constexpr DvarFlags operator|(T lhs, DvarFlags rhs) noexcept {
  return DvarFlags{static_cast<uint32_t>(lhs) | rhs._raw};
}

template <DvarFlagLike T>
inline constexpr DvarFlags operator&(DvarFlags lhs, T rhs) noexcept {
  return DvarFlags{lhs._raw & static_cast<uint32_t>(rhs)};
}

template <DvarFlagLike T, typename = std::enable_if_t<
                              !std::is_same_v<std::decay_t<T>, DvarFlags>>>
inline constexpr DvarFlags operator&(T lhs, DvarFlags rhs) noexcept {
  return DvarFlags{static_cast<uint32_t>(lhs) & rhs._raw};
}

template <DvarFlagLike T>
inline constexpr DvarFlags operator^(DvarFlags lhs, T rhs) noexcept {
  return DvarFlags{lhs._raw ^ static_cast<uint32_t>(rhs)};
}
template <DvarFlagLike T, typename = std::enable_if_t<
                              !std::is_same_v<std::decay_t<T>, DvarFlags>>>
inline constexpr DvarFlags operator^(T lhs, DvarFlags rhs) noexcept {
  return DvarFlags{static_cast<uint32_t>(lhs) ^ rhs._raw};
}

template <DvarFlagLike T>
inline constexpr bool operator==(DvarFlags lhs, T rhs) noexcept {
  return lhs._raw == static_cast<uint32_t>(rhs);
}

template <DvarFlagLike T, typename = std::enable_if_t<
                              !std::is_same_v<std::decay_t<T>, DvarFlags>>>
inline constexpr bool operator==(T lhs, DvarFlags rhs) noexcept {
  return static_cast<uint32_t>(lhs) == rhs._raw;
}

template <DvarFlagLike T>
inline constexpr bool operator!=(DvarFlags lhs, T rhs) noexcept {
  return lhs._raw != static_cast<uint32_t>(rhs);
}

template <DvarFlagLike T, typename = std::enable_if_t<
                              !std::is_same_v<std::decay_t<T>, DvarFlags>>>
inline constexpr bool operator!=(T lhs, DvarFlags rhs) noexcept {
  return static_cast<uint32_t>(lhs) != rhs._raw;
};
ASSERT_SIZE(DvarFlags, sizeof(uint32_t));
ASSERT_POD(DvarFlags);

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

  inline constexpr DvarColor() noexcept = default;
  inline constexpr DvarColor(uint8_t r, uint8_t g, uint8_t b,
                             uint8_t a) noexcept
      : r(r), g(g), b(b), a(a) {}
  inline constexpr DvarColor(uint8_t x, uint8_t y, uint8_t z) noexcept
      : xyz{x, y, z} {}

  // operators for implicit conversion to uint8_t[4], array semantics
  inline constexpr operator uint8_t *() noexcept { return raw; }
  inline constexpr operator const uint8_t *() const noexcept { return raw; }
  inline constexpr uint8_t &operator[](size_t index) noexcept {
    return raw[index];
  }
  inline constexpr const uint8_t &operator[](size_t index) const noexcept {
    return raw[index];
  }
};
ASSERT_POD(DvarColor);
ASSERT_SIZE(DvarColor, 4);
#pragma pack(pop)

namespace com {
eModes Com_SessionMode_GetMode();
}

#pragma pack(push, 1)
template <typename T_DvarValue> union TemplateDvarValue {
  bool _enabled;
  int32_t _integer;
  uint32_t _unsignedInt;
  int64_t _integer64;
  uint64_t _unsignedInt64;
  float _value;
  vec4_t _vector;
  const char *_string;
  DvarColor _color;
  SessionModePool<T_DvarValue *> _indirect;

  inline constexpr bool enabled() const { return _enabled; }

  inline constexpr int32_t integer() const { return _integer; }

  inline constexpr uint32_t unsignedInt() const { return _unsignedInt; }

  inline constexpr int64_t integer64() const { return _integer64; }

  inline constexpr uint64_t unsignedInt64() const { return _unsignedInt64; }

  inline constexpr float value() const { return _value; }

  inline constexpr vec4_t vector() const { return _vector; }
  inline constexpr const char *string() const { return _string; }
  inline constexpr DvarColor color() const { return _color; }

  inline constexpr bool &enabled() { return _enabled; }

  inline constexpr int32_t &integer() { return _integer; }

  inline constexpr uint32_t &unsignedInt() { return _unsignedInt; }

  inline constexpr int64_t &integer64() { return _integer64; }

  inline constexpr uint64_t &unsignedInt64() { return _unsignedInt64; }

  inline constexpr float &value() { return _value; }

  inline constexpr vec4_t &vector() { return _vector; }
  inline constexpr const char *&string() { return _string; }
  inline constexpr DvarColor &color() { return _color; }

  inline const T_DvarValue *indirect(eModes mode) const {
    return valid_mode(mode) ? _indirect[mode] : nullptr;
  }
  inline const SessionModePool<T_DvarValue *> &indirect() const {
    return _indirect;
  }
  inline const T_DvarValue *sessionModeSpecific(eModes mode) const {
    return indirect(mode);
  }
  inline const T_DvarValue *sessionModeSpecific() const {
    return indirect(com::Com_SessionMode_GetMode());
  }

  inline T_DvarValue *indirect(eModes mode) {
    return valid_mode(mode) ? _indirect[mode] : nullptr;
  }
  inline SessionModePool<T_DvarValue *> &indirect() { return _indirect; }
  inline T_DvarValue *sessionModeSpecific(eModes mode) {
    return indirect(mode);
  }
  inline T_DvarValue *sessionModeSpecific() {
    return indirect(com::Com_SessionMode_GetMode());
  }
};
ASSERT_POD(TemplateDvarValue<void>);

typedef TemplateDvarValue<dvar_t> DvarValue;
ASSERT_SIZE(DvarValue, 0x18);
ASSERT_POD(DvarValue);

struct EncryptionCapableDvarValue {
  TemplateDvarValue<encryptedDvar_t> _value;
  uint64_t encryptedValue;

  inline constexpr EncryptionCapableDvarValue() noexcept = default;
  // Fields renamed with "_" prefix in favor of consolidated method interface
  // to allow identical usage of both DvarValue and EncryptionCapableDvarValue.
  // in the `EngineDependentDvar` methods
  inline constexpr bool enabled() const noexcept { return _value.enabled(); }

  inline constexpr int32_t integer() const noexcept { return _value.integer(); }

  inline constexpr uint32_t unsignedInt() const noexcept {
    return _value.unsignedInt();
  }

  inline constexpr int64_t integer64() const noexcept {
    return _value.integer64();
  }

  inline constexpr uint64_t unsignedInt64() const noexcept {
    return _value.unsignedInt64();
  }

  inline constexpr float value() const noexcept { return _value.value(); }

  inline constexpr vec4_t vector() const noexcept { return _value.vector(); }
  inline constexpr const char *string() const noexcept {
    return _value.string();
  }
  inline constexpr DvarColor color() const noexcept { return _value.color(); }

  inline constexpr bool &enabled() { return _value.enabled(); }

  inline constexpr int32_t &integer() { return _value.integer(); }

  inline constexpr uint32_t &unsignedInt() { return _value.unsignedInt(); }

  inline constexpr int64_t &integer64() { return _value.integer64(); }

  inline constexpr uint64_t &unsignedInt64() { return _value.unsignedInt64(); }

  inline constexpr float &value() { return _value.value(); }

  inline constexpr vec4_t &vector() { return _value.vector(); }
  inline constexpr const char *&string() { return _value.string(); }
  inline constexpr DvarColor &color() { return _value.color(); }

  inline constexpr const encryptedDvar_t *indirect(eModes mode) const noexcept {
    return _value.indirect(mode);
  }
  inline constexpr const SessionModePool<encryptedDvar_t *> &
  indirect() const noexcept {
    return _value.indirect();
  }
  inline constexpr encryptedDvar_t *indirect(eModes mode) {
    return _value.indirect(mode);
  }
  inline SessionModePool<encryptedDvar_t *> &indirect() {
    return _value.indirect();
  }

  inline const encryptedDvar_t *
  sessionModeSpecific(eModes mode) const noexcept {
    return _value.sessionModeSpecific(mode);
  }
  inline const encryptedDvar_t *sessionModeSpecific() const noexcept {
    return _value.sessionModeSpecific();
  }
  inline encryptedDvar_t *sessionModeSpecific(eModes mode) {
    return _value.sessionModeSpecific(mode);
  }
  inline encryptedDvar_t *sessionModeSpecific() {
    return _value.sessionModeSpecific();
  }
};
ASSERT_SIZE(EncryptionCapableDvarValue, 0x20);
ASSERT_POD(EncryptionCapableDvarValue);
#pragma pack(pop)

template <typename T> struct PrimitiveLimit {
  T min;
  T max;
};

union DvarLimits {
  struct {
    int32_t stringCount;
    const char **strings;
  } enumeration;

  PrimitiveLimit<int32_t> integer;
  PrimitiveLimit<int64_t> integer64;
  PrimitiveLimit<uint64_t> unsignedInt64;
  PrimitiveLimit<float> value;
  PrimitiveLimit<vec_t> vector;
};
ASSERT_SIZE(DvarLimits, 0x10);

union EngineDependentDvar;
union EngineDependentDvarMut;

#pragma pack(push, 1)
template <typename T_DvarValue> struct dvar {
public:
  dvarStrHash_t _name;
  uint8_t _padding04[4];
  const char *_debugName;
  const char *_description;
  DvarFlags _flags;
  dvarType_t _type;
  bool _modified;
  uint8_t _padding21[7];
  T_DvarValue _current;
  T_DvarValue _latched;
  T_DvarValue _reset;
  DvarLimits _domain;
  dvar<T_DvarValue> *_hashNext;

  std::optional<std::string>
  set(const char *val, DvarSetSource source = DvarSetSource::INTERNAL) const;
  float set(float val, DvarSetSource source = DvarSetSource::INTERNAL) const;
  uint64_t set(uint64_t val,
               DvarSetSource source = DvarSetSource::INTERNAL) const;
  int64_t set(int64_t val,
              DvarSetSource source = DvarSetSource::INTERNAL) const;
  int32_t set(int32_t val,
              DvarSetSource source = DvarSetSource::INTERNAL) const;
  inline uint32_t set(uint32_t val, DvarSetSource source) const noexcept {
    return static_cast<uint32_t>(set(static_cast<int32_t>(val), source));
  }
  bool set(bool val, DvarSetSource source = DvarSetSource::INTERNAL) const;
  inline dvarType_t type() const noexcept { return _type; }
  inline dvarType_t &type() noexcept { return _type; }

  inline bool modified() const noexcept { return _modified; }
  inline bool &modified() noexcept { return _modified; }

  inline const T_DvarValue &current() const noexcept { return _current; }
  inline T_DvarValue &current() noexcept { return _current; }

  inline const T_DvarValue &latched() const noexcept { return _latched; }
  inline T_DvarValue &latched() noexcept { return _latched; }

  inline const T_DvarValue &reset() const noexcept { return _reset; }
  inline T_DvarValue &reset() noexcept { return _reset; }

  inline DvarLimits domain() const noexcept { return _domain; }
  inline DvarLimits &domain() noexcept { return _domain; }

  inline dvarStrHash_t name() const noexcept { return _name; }
  inline dvarStrHash_t &name() noexcept { return _name; }

  inline const char *debugName() const noexcept { return _debugName; }
  inline ConstCharPtr &debugName() noexcept { return _debugName; }

  inline const char *description() const noexcept { return _description; }
  inline ConstCharPtr &description() noexcept { return _description; }

  inline dvar<T_DvarValue> *hashNext() const noexcept { return _hashNext; }
  inline dvar<T_DvarValue> *&hashNext() noexcept { return _hashNext; }

  inline constexpr const SessionModePool<dvar<T_DvarValue> *> &
  indirect() const noexcept {
    return reinterpret_cast<const SessionModePool<dvar<T_DvarValue> *> &>(
        _current.indirect());
  }

  inline constexpr SessionModePool<dvar<T_DvarValue> *> &indirect() noexcept {
    return reinterpret_cast<SessionModePool<dvar<T_DvarValue> *> &>(
        _current.indirect());
  }

  inline dvar<T_DvarValue> *sessionModeSpecific() noexcept {
    if (type() == dvarType_t::SESSIONMODE_BASE_DVAR) {
      return _current.sessionModeSpecific();
    }
    return nullptr;
  }
  inline const dvar<T_DvarValue> *sessionModeSpecific() const noexcept {
    if (type() == dvarType_t::SESSIONMODE_BASE_DVAR) {
      return _current.sessionModeSpecific();
    }
    return nullptr;
  }

  inline dvar<T_DvarValue> *sessionModeSpecific(eModes mode) noexcept {
    if (type() == dvarType_t::SESSIONMODE_BASE_DVAR) {
      return _current.sessionModeSpecific(mode);
    }

    return nullptr;
  }
  inline const dvar<T_DvarValue> *
  sessionModeSpecific(eModes mode) const noexcept {
    if (type() == dvarType_t::SESSIONMODE_BASE_DVAR) {
      return _current.sessionModeSpecific(mode);
    }
    return nullptr;
  }

  inline const dvar<T_DvarValue> *resolve() const noexcept {
    const dvar<T_DvarValue> *sessionModeSpecificDvar = sessionModeSpecific();
    return sessionModeSpecificDvar ? sessionModeSpecificDvar : this;
  }
  inline dvar<T_DvarValue> *resolve() noexcept {
    dvar<T_DvarValue> *sessionModeSpecificDvar = sessionModeSpecific();
    return sessionModeSpecificDvar ? sessionModeSpecificDvar : this;
  }

  inline DvarFlags flags() const noexcept { return resolve()->_flags; }
  inline DvarFlags &flags() noexcept { return resolve()->_flags; }

  inline int32_t get_int() const noexcept {
    return resolve()->current().integer();
  }
  inline constexpr uint32_t get_uint() const noexcept {
    return resolve()->current().unsignedInt();
  }
  inline int64_t get_int64() const noexcept {
    return resolve()->current().integer64();
  }
  inline uint64_t get_uint64() const noexcept {
    return resolve()->current().unsignedInt64();
  }
  inline bool get_bool() const noexcept {
    return resolve()->current().enabled();
  }
  inline float get_float() const noexcept {
    return resolve()->current().value();
  }
  inline const char *get_cstring() const noexcept {
    return resolve()->current().string();
  }
  inline std::optional<std::string_view> get_string() const noexcept {
    const char *str = get_cstring();
    return str ? std::optional(std::string_view(str)) : std::nullopt;
  }
  template <typename T> inline constexpr T get() const noexcept {
    constexpr size_t T_Size = sizeof(T);
    if constexpr (std::is_same_v<T, int32_t>) {
      return get_int();
    }
    if constexpr (std::is_same_v<T, uint32_t>) {
      return get_uint();
    }
    if constexpr (std::is_same_v<T, int64_t>) {
      return get_int64();
    }
    if constexpr (std::is_same_v<T, uint64_t>) {
      return get_uint64();
    }
    if constexpr (std::is_same_v<T, bool>) {
      return get_bool();
    }
    if constexpr (std::is_same_v<T, const char *>) {
      return get_cstring();
    }
    if constexpr (std::is_same_v<T, std::optional<std::string_view>>) {
      return get_string();
    }
    if constexpr (std::is_same_v<T, std::string>) {
      return get_string().value_or("");
    }
    if constexpr (std::is_same_v<T, std::string_view>) {
      return get_string().value_or("");
    }

    if constexpr (T_Size <= sizeof(uint64_t)) {
      T result;

      if constexpr (T_Size > sizeof(uint32_t)) {
        memcpy(result, get_uint64(), T_Size);
        return result;
      } else if constexpr (T_Size > sizeof(uint16_t)) {
        memcpy(result, get_uint32(), T_Size);
      } else if constexpr (T_Size > sizeof(uint8_t)) {
        memcpy(result, get_uint16(), T_Size);
      } else {
        memcpy(result, get_bool(), T_Size /* 1 */);
      }
      return result;
    }

    unreachable();
    return {};
  }

  constexpr operator EngineDependentDvar() const noexcept;
  constexpr operator EngineDependentDvarMut() const noexcept;
  constexpr operator EngineDependentDvarMut() noexcept;
};
#pragma pack(pop)

struct dvar_t : public dvar<DvarValue> {};
ASSERT_OFFSET(dvar_t, _debugName, 0x8);
ASSERT_OFFSET(dvar_t, _description, 0x10);
ASSERT_OFFSET(dvar_t, _flags, 0x18);
ASSERT_OFFSET(dvar_t, _type, 0x1C);
ASSERT_OFFSET(dvar_t, _modified, 0x20);
ASSERT_OFFSET(dvar_t, _current, 0x28);
ASSERT_SIZE(dvar_t, 0x88);
ASSERT_POD(dvar_t);

struct encryptedDvar_t : public dvar<EncryptionCapableDvarValue> {};
ASSERT_OFFSET(encryptedDvar_t, _debugName, 0x8);
ASSERT_OFFSET(encryptedDvar_t, _description, 0x10);
ASSERT_OFFSET(encryptedDvar_t, _flags, 0x18);
ASSERT_OFFSET(encryptedDvar_t, _type, 0x1C);
ASSERT_OFFSET(encryptedDvar_t, _modified, 0x20);
ASSERT_OFFSET(encryptedDvar_t, _current, 0x28);
ASSERT_SIZE(encryptedDvar_t, 0xA0);
ASSERT_POD(encryptedDvar_t);

union EngineDependentDvarMut {
  dvar_t *sv;
  encryptedDvar_t *cl;

  inline std::optional<std::string>
  set(const char *val, DvarSetSource source = DvarSetSource::INTERNAL) const {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline float set(float val,
                   DvarSetSource source = DvarSetSource::INTERNAL) const {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline uint64_t set(uint64_t val,
                      DvarSetSource source = DvarSetSource::INTERNAL) const {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline int64_t set(int64_t val,
                     DvarSetSource source = DvarSetSource::INTERNAL) const {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline int32_t set(int32_t val,
                     DvarSetSource source = DvarSetSource::INTERNAL) const {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline uint32_t set(uint32_t val,
                      DvarSetSource source = DvarSetSource::INTERNAL) const {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline bool set(bool val,
                  DvarSetSource source = DvarSetSource::INTERNAL) const {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline int32_t get_int() const {
    if (is_server()) {
      return sv->get_int();
    }
    return cl->get_int();
  }
  inline uint32_t get_uint() const {
    if (is_server()) {
      return sv->get_uint();
    }
    return cl->get_uint();
  }
  inline int64_t get_int64() const {
    if (is_server()) {
      return sv->get_int64();
    }
    return cl->get_int64();
  }
  inline uint64_t get_uint64() const {
    if (is_server()) {
      return sv->get_uint64();
    }
    return cl->get_uint64();
  }
  inline bool get_bool() const {
    if (is_server()) {
      return sv->get_bool();
    }
    return cl->get_bool();
  }
  inline float get_float() const {
    if (is_server()) {
      return sv->get_float();
    }
    return cl->get_float();
  }
  inline std::optional<std::string_view> get_string() const {
    if (is_server()) {
      return sv->get_string();
    }
    return cl->get_string();
  }
  inline const char *get_cstring() const noexcept {
    if (is_server()) {
      return sv->get_cstring();
    }
    return cl->get_cstring();
  }
  template <typename T> inline T get() const {
    if (is_server()) {
      return sv->get<T>();
    }
    return cl->get<T>();
  }

  inline dvarStrHash_t &name() noexcept {
    if (is_server()) {
      return sv->name();
    }
    return cl->name();
  }
  inline dvarStrHash_t name() const noexcept {
    if (is_server()) {
      return sv->name();
    }
    return cl->name();
  }
  inline void setDebugName(const char *name) noexcept {
    if (is_server()) {
      sv->debugName() = name;
    } else {
      cl->debugName() = name;
    }
  }
  inline const char *&debugName() noexcept {
    if (is_server()) {
      return sv->debugName();
    }
    return cl->debugName();
  }
  inline const char *debugName() const noexcept {
    if (is_server()) {
      return sv->debugName();
    }
    return cl->debugName();
  }
  inline const char *&description() noexcept {
    if (is_server()) {
      return sv->description();
    }
    return cl->description();
  }
  inline const char *description() const noexcept {
    if (is_server()) {
      return sv->description();
    }
    return cl->description();
  }
  inline DvarFlags &flags() noexcept {
    if (is_server()) {
      return sv->flags();
    }
    return cl->flags();
  }

  template <DvarFlagLike T> inline void removeFlags(T flags) noexcept {
    flags().remove(flags);
  }

  template <DvarFlagLike T> inline void addFlags(T flags) noexcept {
    flags().add(flags);
  }
  template <DvarFlagLike T> inline void setFlags(const T flags) noexcept {
    flags().set(flags);
  }

  inline void clearFlags() noexcept { flags().set(0); }
  inline DvarFlags flags() const noexcept {
    if (is_server()) {
      return sv->flags();
    }
    return cl->flags();
  }
  inline dvarType_t &type() noexcept {
    if (is_server()) {
      return sv->type();
    }
    return cl->type();
  }
  inline dvarType_t type() const noexcept {
    if (is_server()) {
      return sv->type();
    }
    return cl->type();
  }
  inline EngineDependentDvarMut sessionModeSpecific() const {
    EngineDependentDvarMut result{nullptr};
    if (is_server()) {
      result.sv = reinterpret_cast<dvar_t *>(sv->sessionModeSpecific());
    } else {
      result.cl =
          reinterpret_cast<encryptedDvar_t *>(cl->sessionModeSpecific());
    }
    return result;
  }

  inline EngineDependentDvarMut resolve() const {
    EngineDependentDvarMut result{};
    if (is_server()) {
      result.sv = reinterpret_cast<dvar_t *>(sv->resolve());
    } else {
      result.cl = reinterpret_cast<encryptedDvar_t *>(cl->resolve());
    }
    return result;
  }

  inline bool &modified() noexcept {
    if (is_server()) {
      return sv->modified();
    }
    return cl->modified();
  }
  inline bool modified() const noexcept {
    if (is_server()) {
      return sv->modified();
    }
    return cl->modified();
  }
  inline DvarLimits &domain() noexcept {
    if (is_server()) {
      return sv->domain();
    }
    return cl->domain();
  }
  inline DvarLimits &domain() const noexcept {
    if (is_server()) {
      return sv->domain();
    }
    return cl->domain();
  }
  inline EngineDependentDvarMut setHashNext(dvar_t *next) {
    EngineDependentDvarMut result = hashNext();
    if (is_server()) {
      sv->hashNext() = next;
    } else {
      cl->hashNext() = reinterpret_cast<encryptedDvar_t *>(next);
    }
    return result;
  }
  inline EngineDependentDvarMut setHashNext(encryptedDvar_t *next) {
    EngineDependentDvarMut result = hashNext();
    if (is_server()) {
      sv->hashNext() = reinterpret_cast<dvar_t *>(next);
    } else {
      cl->hashNext() = next;
    }
    return result;
  }
  inline EngineDependentDvarMut setHashNext(EngineDependentDvarMut next) {
    EngineDependentDvarMut result = hashNext();
    if (is_server()) {
      sv->hashNext() = next.sv;
    } else {
      cl->hashNext() = next.cl;
    }
    return result;
  }
  inline EngineDependentDvarMut hashNext() const {
    EngineDependentDvarMut result{};
    if (is_server()) {
      result.sv = reinterpret_cast<dvar_t *>(sv->hashNext());
    } else {
      result.cl = reinterpret_cast<encryptedDvar_t *>(cl->hashNext());
    }
    return result;
  }

  inline constexpr bool nonnull() const noexcept { return sv != nullptr; }
  inline constexpr bool null() const noexcept { return sv == nullptr; }

  inline constexpr bool operator!() const noexcept { return null(); }
  inline constexpr bool
  operator>(const EngineDependentDvarMut &rhs) const noexcept {
    return sv > rhs.sv;
  }
  inline constexpr bool
  operator<(const EngineDependentDvarMut &rhs) const noexcept {
    return sv < rhs.sv;
  }
  inline constexpr operator bool() const noexcept { return nonnull(); }
  inline constexpr operator dvar_t *() const noexcept { return sv; }
  inline constexpr operator encryptedDvar_t *() const noexcept { return cl; }
  inline constexpr operator const dvar_t *() const noexcept { return sv; }
  inline constexpr operator const encryptedDvar_t *() const noexcept {
    return cl;
  }

  template <typename T_DvarValue>
  inline constexpr operator const dvar<T_DvarValue> *() const noexcept {
    return reinterpret_cast<const dvar<T_DvarValue> *>(sv);
  }

  template <typename T_DvarValue>
  inline constexpr operator dvar<T_DvarValue> *() const noexcept {
    return reinterpret_cast<dvar<T_DvarValue> *>(sv);
  }

  constexpr operator EngineDependentDvar() const noexcept;
  constexpr operator EngineDependentDvar() noexcept;

  friend inline constexpr bool
  operator==(const EngineDependentDvarMut &lhs,
             const EngineDependentDvarMut &rhs) noexcept {
    return lhs.sv == rhs.sv;
  }
  friend constexpr bool operator==(const EngineDependentDvarMut &lhs,
                                   const EngineDependentDvar &rhs) noexcept;
  template <typename T>
  friend inline constexpr bool operator==(const EngineDependentDvarMut &lhs,
                                          const T *&rhs) noexcept {
    return lhs.cl == rhs;
  }
  template <PtrLike T>
  friend inline constexpr bool operator==(const EngineDependentDvarMut &lhs,
                                          const T &rhs) noexcept {
    return reinterpret_cast<uintptr_t>(lhs.cl) == static_cast<uintptr_t>(rhs);
  }

  friend inline constexpr bool operator==(const EngineDependentDvarMut &lhs,
                                          const std::nullptr_t &rhs) noexcept {
    return lhs.sv == rhs;
  }
};

ASSERT_SIZE(EngineDependentDvarMut, 8);
ASSERT_POD(EngineDependentDvarMut);

union EngineDependentDvar {
  const dvar_t *sv;
  const encryptedDvar_t *cl;

  inline std::optional<std::string>
  set(const char *val,
      DvarSetSource source = DvarSetSource::INTERNAL) const noexcept {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline float
  set(float val,
      DvarSetSource source = DvarSetSource::INTERNAL) const noexcept {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline uint64_t
  set(uint64_t val,
      DvarSetSource source = DvarSetSource::INTERNAL) const noexcept {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline int64_t
  set(int64_t val,
      DvarSetSource source = DvarSetSource::INTERNAL) const noexcept {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline int32_t
  set(int32_t val,
      DvarSetSource source = DvarSetSource::INTERNAL) const noexcept {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline uint32_t
  set(uint32_t val,
      DvarSetSource source = DvarSetSource::INTERNAL) const noexcept {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline bool
  set(bool val, DvarSetSource source = DvarSetSource::INTERNAL) const noexcept {
    if (is_server()) {
      return sv->set(val, source);
    }
    return cl->set(val, source);
  }
  inline int32_t get_int() const noexcept {
    if (is_server()) {
      return sv->get_int();
    }
    return cl->get_int();
  }
  inline uint32_t get_uint() const noexcept {
    if (is_server()) {
      return sv->get_uint();
    }
    return cl->get_uint();
  }
  inline int64_t get_int64() const noexcept {
    if (is_server()) {
      return sv->get_int64();
    }
    return cl->get_int64();
  }
  inline uint64_t get_uint64() const noexcept {
    if (is_server()) {
      return sv->get_uint64();
    }
    return cl->get_uint64();
  }
  inline bool get_bool() const noexcept {
    if (is_server()) {
      return sv->get_bool();
    }
    return cl->get_bool();
  }
  inline float get_float() const noexcept {
    if (is_server()) {
      return sv->get_float();
    }
    return cl->get_float();
  }
  inline std::optional<std::string_view> get_string() const noexcept {
    if (is_server()) {
      return sv->get_string();
    }
    return cl->get_string();
  }
  inline const char *get_cstring() const noexcept {
    if (is_server()) {
      return sv->get_cstring();
    }
    return cl->get_cstring();
  }
  template <typename T> inline T get() const noexcept {
    if (is_server()) {
      return sv->get<T>();
    }
    return cl->get<T>();
  }

  inline dvarStrHash_t name() const noexcept {
    if (is_server()) {
      return sv->name();
    }
    return cl->name();
  }

  inline const char *debugName() const noexcept {
    if (is_server()) {
      return sv->debugName();
    }
    return cl->debugName();
  }
  inline const char *description() const noexcept {
    if (is_server()) {
      return sv->description();
    }
    return cl->description();
  }
  inline DvarFlags flags() const noexcept {
    if (is_server()) {
      return sv->flags();
    }
    return cl->flags();
  }
  inline dvarType_t type() const noexcept {
    if (is_server()) {
      return sv->type();
    }
    return cl->type();
  }

  inline const SessionModePool<EngineDependentDvar> &indirect() const noexcept {
    if (is_server()) {
      return reinterpret_cast<const SessionModePool<EngineDependentDvar> &>(
          sv->indirect());
    }
    return reinterpret_cast<const SessionModePool<EngineDependentDvar> &>(
        cl->indirect());
  }

  inline EngineDependentDvar sessionModeSpecific() const noexcept {
    EngineDependentDvar result{nullptr};
    if (is_server()) {
      result.sv = reinterpret_cast<const dvar_t *>(sv->sessionModeSpecific());
    } else {
      result.cl =
          reinterpret_cast<const encryptedDvar_t *>(cl->sessionModeSpecific());
    }
    return result;
  }

  inline EngineDependentDvar sessionModeSpecific(eModes mode) const noexcept {
    EngineDependentDvar result{nullptr};
    if (is_server()) {
      result.sv =
          reinterpret_cast<const dvar_t *>(sv->sessionModeSpecific(mode));
    } else {
      result.cl = reinterpret_cast<const encryptedDvar_t *>(
          cl->sessionModeSpecific(mode));
    }
    return result;
  }

  inline EngineDependentDvar resolve() const noexcept {
    EngineDependentDvar result{};
    if (is_server()) {
      result.sv = reinterpret_cast<const dvar_t *>(sv->resolve());
    } else {
      result.cl = reinterpret_cast<const encryptedDvar_t *>(cl->resolve());
    }
    return result;
  }

  inline bool modified() const noexcept {
    if (is_server()) {
      return sv->modified();
    }
    return cl->modified();
  }
  inline DvarLimits domain() const noexcept {
    if (is_server()) {
      return sv->domain();
    }
    return cl->domain();
  }
  inline EngineDependentDvar hashNext() const noexcept {
    EngineDependentDvar result{};
    if (is_server()) {
      result.sv = const_cast<const dvar_t *>(
          reinterpret_cast<dvar_t *>(sv->hashNext()));
    } else {
      result.cl = const_cast<const encryptedDvar_t *>(
          reinterpret_cast<encryptedDvar_t *>(cl->hashNext()));
    }
    return result;
  }

  inline constexpr bool nonnull() const { return sv != nullptr; }
  inline constexpr bool null() const { return sv == nullptr; }

  inline constexpr bool operator!() const noexcept { return null(); }
  inline constexpr bool
  operator>(const EngineDependentDvar &rhs) const noexcept {
    return sv > rhs.sv;
  }
  inline constexpr bool
  operator<(const EngineDependentDvar &rhs) const noexcept {
    return sv < rhs.sv;
  }
  inline constexpr operator bool() const noexcept { return nonnull(); }
  inline constexpr operator const dvar_t *() const noexcept { return sv; }
  inline constexpr operator const encryptedDvar_t *() const noexcept {
    return cl;
  }
  template <typename T_DvarValue>
  inline constexpr operator const dvar<T_DvarValue> *() const noexcept {
    return reinterpret_cast<dvar<T_DvarValue>>(sv);
  }
  friend inline constexpr bool operator==(const EngineDependentDvar &lhs,
                                          const EngineDependentDvar &rhs) {
    return lhs.sv == rhs.sv;
  }
  friend constexpr bool operator==(const EngineDependentDvarMut &lhs,
                                   const EngineDependentDvar &rhs) noexcept;
  template <typename T>
  friend inline constexpr bool operator==(const EngineDependentDvar &lhs,
                                          const T *&rhs) {
    return lhs.cl == rhs;
  }
  template <PtrLike T>
  friend inline constexpr bool operator==(const EngineDependentDvar &lhs,
                                          const T &rhs) {
    return reinterpret_cast<uintptr_t>(lhs.cl) == static_cast<uintptr_t>(rhs);
  }

  friend inline constexpr bool operator==(const EngineDependentDvar &lhs,
                                          const std::nullptr_t &rhs) {
    return lhs.sv == rhs;
  }
};

ASSERT_SIZE(EngineDependentDvar, 8);
ASSERT_POD(EngineDependentDvar);

inline constexpr bool operator==(const EngineDependentDvarMut &lhs,
                                 const EngineDependentDvar &rhs) noexcept {
  return lhs.sv == rhs.sv;
}

void Dvar_SetStringFromSource(EngineDependentDvar dvar, const char *val,
                              DvarSetSource source);
void Dvar_SetIntFromSource(EngineDependentDvar dvar, int32_t val,
                           DvarSetSource source);
void Dvar_SetInt64FromSource(EngineDependentDvar dvar, int64_t val,
                             DvarSetSource source);
void Dvar_SetUInt64FromSource(EngineDependentDvar dvar, uint64_t val,
                              DvarSetSource source);
void Dvar_SetBoolFromSource(EngineDependentDvar dvar, bool val,
                            DvarSetSource source);
void Dvar_SetFloatFromSource(EngineDependentDvar dvar, float val,
                             DvarSetSource source);

template <typename T_DvarValue>
inline std::optional<std::string>
dvar<T_DvarValue>::set(const char *val, DvarSetSource source) const {
  const std::optional<std::string_view> prev_val = get_string();
  std::optional<std::string> prev_val_copy;
  if (prev_val.has_value()) {
    prev_val_copy = std::optional(std::string(prev_val.value()));
  } else {
    prev_val_copy = std::nullopt;
  }

  Dvar_SetStringFromSource(
      EngineDependentDvar{reinterpret_cast<const dvar_t *>(resolve())}, val,
      source);
  return prev_val_copy;
}
template <typename T_DvarValue>
inline float dvar<T_DvarValue>::set(float val, DvarSetSource source) const {
  const float prev_val = get_float();
  Dvar_SetFloatFromSource(
      EngineDependentDvar{reinterpret_cast<const dvar_t *>(resolve())}, val,
      source);
  return prev_val;
}
template <typename T_DvarValue>
inline uint64_t dvar<T_DvarValue>::set(uint64_t val,
                                       DvarSetSource source) const {
  const uint64_t prev_val = get_uint64();
  Dvar_SetUInt64FromSource(
      EngineDependentDvar{reinterpret_cast<const dvar_t *>(resolve())}, val,
      source);
  return prev_val;
}
template <typename T_DvarValue>
inline int64_t dvar<T_DvarValue>::set(int64_t val, DvarSetSource source) const {
  const int64_t prev_val = get_int64();
  Dvar_SetInt64FromSource(
      EngineDependentDvar{reinterpret_cast<const dvar_t *>(resolve())}, val,
      source);
  return prev_val;
}
template <typename T_DvarValue>
inline int32_t dvar<T_DvarValue>::set(int32_t val, DvarSetSource source) const {
  const int32_t prev_val = get_int();
  Dvar_SetIntFromSource(
      EngineDependentDvar{reinterpret_cast<const dvar_t *>(resolve())}, val,
      source);
  return prev_val;
}

template <typename T_DvarValue>
inline bool dvar<T_DvarValue>::set(bool val, DvarSetSource source) const {
  const bool prev_val = get_bool();
  Dvar_SetBoolFromSource(
      EngineDependentDvar{reinterpret_cast<const dvar_t *>(resolve())}, val,
      source);
  return prev_val;
}
template <typename T_DvarValue>

inline constexpr dvar<T_DvarValue>::operator EngineDependentDvar()
    const noexcept {
  return EngineDependentDvar{reinterpret_cast<const dvar_t *>(resolve())};
}
template <typename T_DvarValue>
inline constexpr dvar<T_DvarValue>::operator EngineDependentDvarMut()
    const noexcept {
  return EngineDependentDvarMut{reinterpret_cast<dvar_t *>(resolve())};
}
template <typename T_DvarValue>
inline constexpr dvar<T_DvarValue>::operator EngineDependentDvarMut() noexcept {
  return EngineDependentDvarMut{reinterpret_cast<dvar_t *>(resolve())};
}
inline constexpr EngineDependentDvarMut::operator EngineDependentDvar()
    const noexcept {
  return EngineDependentDvar{sv};
}
inline constexpr EngineDependentDvarMut::
operator EngineDependentDvar() noexcept {
  return EngineDependentDvar{sv};
}

constexpr size_t DVAR_POOL_LEN = 0x2000;
template <typename T_DvarValue> struct TemplateDvarPool {
  T_DvarValue pool[DVAR_POOL_LEN];

  inline constexpr const T_DvarValue &operator[](size_t index) const {
    return pool[index];
  }
  inline constexpr T_DvarValue &operator[](size_t index) { return pool[index]; }

  inline constexpr const T_DvarValue *base() const { return &pool[0]; }
  inline constexpr const T_DvarValue *end() const {
    return &pool[DVAR_POOL_LEN];
  }
  inline bool contains(const T_DvarValue *dvar) const {
    return dvar >= base() && dvar < end();
  }
};
typedef TemplateDvarPool<dvar_t> DvarPool;
ASSERT_SIZE(DvarPool, 0x110000);

typedef TemplateDvarPool<encryptedDvar_t> EncryptedDvarPool;
ASSERT_SIZE(EncryptedDvarPool, 0x140000);

union EngineDependentDvarPool {
  DvarPool *sv;
  EncryptedDvarPool *cl;

  inline const EngineDependentDvar operator[](size_t index) const {
    EngineDependentDvar result{};
    if (is_server()) {
      result.sv = &sv->pool[index];
    } else {
      result.cl = &cl->pool[index];
    }
    return result;
  }
  inline EngineDependentDvarMut operator[](size_t index) {
    EngineDependentDvarMut result{};
    if (is_server()) {
      result.sv = &sv->pool[index];
    } else {
      result.cl = &cl->pool[index];
    }
    return result;
  }

  inline EngineDependentDvar base() const {
    EngineDependentDvar result{};
    if (is_server()) {
      result.sv = sv->base();
    } else {
      result.cl = cl->base();
    }
    return result;
  }
  inline EngineDependentDvar end() const {
    EngineDependentDvar result{};
    if (is_server()) {
      result.sv = sv->end();
    } else {
      result.cl = cl->end();
    }
    return result;
  }

  inline bool contains(EngineDependentDvar dvar) const noexcept {
    if (is_server()) {
      return sv->contains(dvar.sv);
    }
    return cl->contains(dvar.cl);
  }

  inline constexpr bool null() const noexcept { return sv == nullptr; }
  inline constexpr bool nonnull() const noexcept { return sv != nullptr; }

  inline constexpr EngineDependentDvarPool() noexcept = default;
  inline constexpr EngineDependentDvarPool(DvarPool *pool) noexcept
      : sv(pool) {}
  inline constexpr EngineDependentDvarPool(EncryptedDvarPool *pool) noexcept
      : cl(pool) {}
  inline constexpr EngineDependentDvarPool(std::nullptr_t pool) noexcept
      : cl(pool) {}

  inline constexpr bool operator!() const noexcept { return null(); }
  inline constexpr operator bool() const noexcept { return nonnull(); }
  template <typename T>
  friend inline constexpr bool operator==(const EngineDependentDvarPool &lhs,
                                          const T *&rhs) {
    return lhs.cl == rhs;
  }
  template <PtrLike T>
  friend inline constexpr bool operator==(const EngineDependentDvarPool &lhs,
                                          const T &rhs) {
    return reinterpret_cast<uintptr_t>(lhs.cl) == static_cast<uintptr_t>(rhs);
  }

  friend inline constexpr bool operator==(const EngineDependentDvarPool &lhs,
                                          const std::nullptr_t &rhs) {
    return lhs.sv == rhs;
  }
};

typedef fastcallPtr_t<void(EngineDependentDvar dvar)> modifiedCallback;

#pragma pack(push, 1)
struct dvarCallBack_t {
  bool needsCallback;
  uint8_t _padding01[7];
  modifiedCallback callback;
  EngineDependentDvar dvar;
};
#pragma pack(pop)

struct DvarCallbackPool {
  dvarCallBack_t pool[0x100];

  inline constexpr const dvarCallBack_t &operator[](size_t index) const {
    return pool[index];
  }
  inline constexpr dvarCallBack_t &operator[](size_t index) {
    return pool[index];
  }
};
ASSERT_SIZE(DvarCallbackPool, 0x1800);

constexpr dvarStrHash_t DVAR_HASH_TABLE_LEN = 0x800;
constexpr dvarStrHash_t DVAR_HASH_MASK = DVAR_HASH_TABLE_LEN - 1;
struct DvarHashTable {
  EngineDependentDvarMut table[0x800];

  inline EngineDependentDvarMut get(dvarStrHash_t hash) const {
    EngineDependentDvarMut entry = table[hash & DVAR_HASH_MASK];
    while (entry && entry.name() != hash) {
      entry = entry.hashNext();
    }
    return entry;
  }

  inline bool contains(dvarStrHash_t hash) const {
    EngineDependentDvarMut entry = get(hash);
    return entry;
  }

  inline bool contains(const char *name) const {
    return contains(Dvar_GenerateHash_Impl(name));
  }

  inline EngineDependentDvarMut get(const char *name) const {
    return get(Dvar_GenerateHash_Impl(name));
  }

  inline EngineDependentDvarMut operator[](dvarStrHash_t hash) const {
    return get(hash);
  }
  inline EngineDependentDvarMut operator[](dvarStrHash_t hash) {
    return get(hash);
  }

  inline EngineDependentDvarMut operator[](const char *name) const {
    return get(name);
  }

  inline EngineDependentDvarMut operator[](const char *name) {
    return get(name);
  }
};
ASSERT_SIZE(DvarHashTable, 0x4000);

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

typedef fastcallPtr_t<void(EngineDependentDvar dvar, void *userData)>
    forEachCallback;
} // namespace game