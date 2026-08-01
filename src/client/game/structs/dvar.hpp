#pragma once

#include <macros.hpp>
#include "game/ptr.hpp"
#include "macros.hpp"
#include <structs/func.hpp>
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

struct dvarCallBack_t;
struct dvar_t;
struct encryptedDvar_t;

enum class DvarSetSource : uint32_t {
  INTERNAL = 0x0,
  EXTERNAL = 0x1,
  SCRIPT = 0x2,
  DEVGUI = 0x3
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
  // DVAR_UNKNOWN2x = 1<< 11-12
  // Sets `findCallBackForDvar(dvar)->needsCallback = true;` on change
  DVAR_MODIFIED_CALLBACK = 1 << 13,
  // DVAR_UNKNOWN1x = 1 << 14,
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
    uint32_t unknown_11_12 : 2;
    uint32_t modifiedCallback : 1;
    uint32_t unknown_14 : 1;
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
ASSERT_CPP03_POD(DvarFlags);

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
ASSERT_CPP03_POD(DvarColor);
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

  inline T_DvarValue *indirect(eModes mode) {
    return valid_mode(mode) ? _indirect[mode] : nullptr;
  }
  inline SessionModePool<T_DvarValue *> &indirect() { return _indirect; }
  inline const T_DvarValue *sessionModeSpecific(eModes mode) const {
    return indirect(mode);
  }
  inline T_DvarValue *sessionModeSpecific(eModes mode) {
    return indirect(mode);
  }
  inline const T_DvarValue *sessionModeSpecific() const {
    return indirect(com::Com_SessionMode_GetMode());
  }

  inline T_DvarValue *sessionModeSpecific() {
    return indirect(com::Com_SessionMode_GetMode());
  }
  inline constexpr bool
  set(bool val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      [[maybe_unused]] bool shouldEncrypt = true) noexcept {
    const bool prev = enabled();
    enabled() = val;
    return prev;
  }

  inline constexpr int32_t
  set(int32_t val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      [[maybe_unused]] bool shouldEncrypt = true) noexcept {
    const int32_t prev = integer();
    integer() = val;
    return prev;
  }

  inline constexpr uint32_t
  set(uint32_t val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      [[maybe_unused]] bool shouldEncrypt = true) noexcept {
    const uint32_t prev = unsignedInt();
    unsignedInt() = val;
    return prev;
  }

  inline constexpr int64_t
  set(int64_t val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      [[maybe_unused]] bool shouldEncrypt = true) noexcept {
    const int64_t prev = integer64();
    integer64() = val;
    return prev;
  }

  inline constexpr uint64_t
  set(uint64_t val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      [[maybe_unused]] bool shouldEncrypt = true) noexcept {
    const uint64_t prev = unsignedInt64();
    unsignedInt64() = val;
    return prev;
  }

  inline constexpr float
  set(float val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      [[maybe_unused]] bool shouldEncrypt = true) noexcept {
    const float prev = value();
    value() = val;
    return prev;
  }

  inline constexpr vec4_t
  set(vec4_t val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      [[maybe_unused]] bool shouldEncrypt = true) noexcept {
    const vec4_t prev = vector();
    vector() = val;
    return prev;
  }

  /*
    Note: this needs particular handling in the `dvar` struct
    to either re-use the `reset` value string pointer or re-allocate the string
    in the SL string pool, then assign with `sl::CopyString`. Direct assignment
    with a new string pointer will lead to unexpected behaviour.
  */
  // TODO: overrides for `std::string`, `std::string_view`
  inline constexpr const char *
  set(const char *val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      [[maybe_unused]] bool shouldEncrypt = true) noexcept {

    const char *prev = string();
    string() = val;
    return prev;
  }
  inline constexpr const char *
  set(const std::string_view &val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      bool shouldEncrypt = true) {
    return set(val.data(), shouldEncrypt);
  }
  inline constexpr const char *
  set(const std::string &val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      bool shouldEncrypt = true) {
    return set(val.c_str(), shouldEncrypt);
  }
  inline constexpr DvarColor
  set(DvarColor val,
      /* added for API compatbility with `EncryptionCapableDvarValue`.
          Should be optimized away by compiler. */
      [[maybe_unused]] bool shouldEncrypt = true) noexcept {
    const DvarColor prev = color();
    color() = val;
    return prev;
  }
};
ASSERT_CPP03_POD(TemplateDvarValue<void>);

typedef TemplateDvarValue<dvar_t> DvarValue;
ASSERT_SIZE(DvarValue, 0x18);
ASSERT_CPP03_POD(DvarValue);

struct EncryptionCapableDvarValue {
  TemplateDvarValue<encryptedDvar_t> _value;
  uint64_t _encryptedValue;

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

  inline const uint64_t &encryptedValue() const noexcept {
    return _encryptedValue;
  }

  inline uint64_t &encryptedValue() noexcept { return _encryptedValue; }

  static inline uint64_t encrypt(bool val) noexcept {
    uint32_t peb = PEB32();
    uint32_t x = val ? 1 : 0;
    uint32_t z = 0;

    // 1) key == 0x4F48F38
    z = static_cast<uint16_t>((peb ^ 0xC8C0u) - x * 0x56DDu);
    x = std::rotr(x, 16);

    // 2) key == 0xCAE3A886
    x ^= z;

    // 3) key == 0x6250C961
    uint32_t t = std::rotr(x, 16);
    uint16_t u = static_cast<uint16_t>((peb ^ 0xC3ECu) - x * 0xC67u);
    x = u ^ t;
    z = static_cast<uint16_t>(x);

    // 4) key == 0x5C395230
    z = static_cast<uint16_t>((peb ^ 0xBC84u) - z * 0x1C72u);

    // 5) key == 0x6AAA441D
    uint32_t tmp = z ^ std::rotl(x, 16);
    z = static_cast<uint16_t>((peb ^ 0xC8ADu) + tmp * 0x5C92u);
    x = std::rotl(tmp, 16);

    // 6) key == 0x62078B4F
    x ^= z;

    // 7) key == 0x69456FAD
    x = std::rotr(x, 16);

    return x;
  }

  static inline uint32_t encrypt(float val) {
    uint32_t peb = PEB32();
    uint32_t result = std::bit_cast<uint32_t>(val);
    uint32_t s = 0;

    // 1) key == 0x3546219F
    s = static_cast<uint16_t>(result);

    // 2) key == 0xD96A6946
    result = std::rotl(result, 16);
    uint32_t t = static_cast<uint16_t>((peb ^ 0xAE44u) + s * 450u);
    result ^= t;
    s = static_cast<uint16_t>(result);

    // 3) key == 0x2E207F95
    s = static_cast<uint16_t>((peb ^ 0x8065u) - s * 0x760Bu);

    // 4) key == 0x3C08BA56
    result = std::rotl(result, 16);

    // 5) key == 0xB5A9DE39
    result ^= s;
    s = static_cast<uint16_t>(result);

    // 6) key == 0xB1242083
    result = std::rotl(result, 16);
    s = static_cast<uint16_t>((peb ^ 0x252u) + s * 0x78F8u);

    // 7) key == 0x1CD815D5
    result ^= s;
    s = static_cast<uint16_t>(result);

    // 8) key == 0xC65161C4
    s = static_cast<uint16_t>((peb ^ 0x9FFAu) + s * 0x2C23u);

    // 9) key == 0xFF6FF984 -> terminal 0x4843D1BC
    result = std::rotl(s ^ std::rotl(result, 16), 16);

    return result; // zero‑extend to 64 bits when stored
  }

  static inline uint64_t encrypt(uint32_t val) noexcept {
    int32_t peb = PEB32();

    uint32_t result = (uint32_t)val; // ebx
    uint32_t y = 0;                  // edx

    // 1) key == 0x9CF31C5
    y = static_cast<uint16_t>((peb ^ 0x7C35u) - result * 0x565Bu);
    // key ^= 0xA6B5A745 == new key = 0xAF7A9680

    // 2) key == 0xAF7A9680 (break)
    result = std::rotl(result, 16);
    // key ^= 0x0F78FD3E == new key = 0xA0026BBE

    // 3) key == 0xA0026BBE
    uint32_t temp = result ^ y; // ebx ^ edx
    y = static_cast<uint16_t>((peb ^ 0x6BFAu) + temp * 0x54F1u);
    result = std::rotr(temp, 16) ^ y;
    // key ^= 0x9521BEB4 == 0x3523D50A

    // 4) key == 0x3523D50A
    y = static_cast<uint16_t>(result); // movzx edx, bx
    // key ^= 0xD7A4E0B7 == 0xE28735BD

    // 5) key == 0xE28735BD
    y = static_cast<uint16_t>((peb ^ 0x0FFFF9674u) - y * 0x5534u);
    // key ^= 0x08E45694 == 0xEA636329

    // 6) key == 0xEA636329
    result = std::rotl(result, 16);
    // key ^= 0x02A35792 == 0xE8C034BB

    // 7) key == 0xE8C034BB
    result ^= y;
    y = static_cast<uint16_t>(result);
    // key ^= 0xB69644E4 == 0x5E56705F

    // 8) key == 0x5E56705F
    y = static_cast<uint16_t>((peb ^ 0x6A32u) + y * 0x1757u);
    // key ^= 0x79E6DDA5 == 0x27B0ADFA

    // 9) key == 0x27B0ADFA
    result = std::rotr(result, 16) ^ y;
    // key ^= 0x8CC787D4 == 0xAB772A2E

    // 10) key == 0xAB772A2E
    result = std::rotl(result, 16);
    // key ^= 0x0D0BAF21 == terminal 0xA67C850F

    return result; // zero‑extended to 64 bits
  }

  // Enum only. String is not encrypted.
  static inline uint64_t encrypt(const char *val) noexcept {
    return encrypt(static_cast<uint32_t>(reinterpret_cast<uint64_t>(val)));
  }

  static inline uint64_t encrypt(int32_t val) noexcept {
    return encrypt(std::bit_cast<uint32_t>(val));
  }

  static constexpr uint32_t INVALID_DVAR_TYPE_DEFAULT_SEED = 0;

  // Invalid or otherwise unknown dvar type
  static inline uint64_t encrypt() noexcept {
    return encrypt(INVALID_DVAR_TYPE_DEFAULT_SEED);
  }

  static inline uint64_t encrypt(uint64_t value) {
    uint64_t peb = PEB();
    uint64_t enc = value;
    uint64_t t = 0;

    // 1) key == 0x1EC7CB6
    int64_t c1 = static_cast<int64_t>(0xFFFFFFFF836D435AULL);
    uint64_t p1 = peb ^ 0xFFFFFFFF8131F8BEULL;
    uint64_t prod1 = static_cast<uint64_t>(static_cast<int64_t>(enc) * c1);
    t = static_cast<uint32_t>(p1 + prod1);

    // 2) key == 0xFD2C81D
    enc = std::rotr(enc, 32) ^ t;

    // 3) key == 0xD02E43C5
    t = static_cast<uint32_t>(enc);

    // 4) key == 0x567C7575
    int64_t c4 = static_cast<int64_t>(0xFFFFFFFFAD28AB13ULL);
    uint64_t p4 = peb ^ 0x6BD3B7D9ULL; // zero‑extended 32‑bit constant
    uint64_t prod4 = static_cast<uint64_t>(static_cast<int64_t>(t) * c4);
    t = static_cast<uint32_t>(p4 + prod4);

    // 5) key == 0x5208D556
    enc = std::rotr(enc, 32) ^ t;

    // 6) key == 0x4EB19C7F
    uint64_t original = enc;
    int64_t c6 = static_cast<int64_t>(0xFFFFFFFFE6A38132ULL);
    uint64_t p6 = peb ^ 0xFFFFFFFF8671D5D2ULL;
    uint64_t prod6 = static_cast<uint64_t>(static_cast<int64_t>(original) * c6);
    uint32_t trunc6 = static_cast<uint32_t>(p6 + prod6);
    enc = std::rotl(original, 32) ^ trunc6;
    t = static_cast<uint32_t>(enc);

    // 7) key == 0x4279D8C4
    int64_t c7 = 0x4075B22; // positive, sign‑extended to 64 bits
    uint64_t p7 = peb ^ 0x4D3B7FF4ULL;
    uint64_t prod7 = static_cast<uint64_t>(static_cast<int64_t>(t) * c7);
    t = static_cast<uint32_t>(p7 + prod7);

    // 8) key == 0xE70F62A0
    enc = std::rotr(enc, 32) ^ t;

    // 9) key == 0x3B923398 -> terminal 0x9B48C62B
    enc = std::rotr(enc, 32);

    return enc;
  }

  static inline uint64_t encrypt(int64_t val) noexcept {
    return encrypt(std::bit_cast<uint64_t>(val));
  }

  static constexpr uint64_t UNENCRYPTED_DVAR_TYPE_ENCRYPTED_VALUE = 0;
  static inline uint64_t encrypt([[maybe_unused]] vec2_t val) noexcept {
    return UNENCRYPTED_DVAR_TYPE_ENCRYPTED_VALUE;
  }
  static inline uint64_t encrypt([[maybe_unused]] vec3_t val) noexcept {
    return UNENCRYPTED_DVAR_TYPE_ENCRYPTED_VALUE;
  }
  static inline uint64_t encrypt([[maybe_unused]] vec4_t val) noexcept {
    return UNENCRYPTED_DVAR_TYPE_ENCRYPTED_VALUE;
  }
  static inline uint64_t encrypt([[maybe_unused]] DvarColor val) noexcept {
    return UNENCRYPTED_DVAR_TYPE_ENCRYPTED_VALUE;
  }

  inline constexpr bool set(bool val, bool shouldEncrypt = true) noexcept {
    if (shouldEncrypt) {
      encryptedValue() = encrypt(val);
    }

    return _value.set(val);
  }

  inline constexpr int32_t set(int32_t val,
                               bool shouldEncrypt = true) noexcept {
    if (shouldEncrypt) {
      encryptedValue() = encrypt(val);
    }

    return _value.set(val);
  }

  inline constexpr uint32_t set(uint32_t val,
                                bool shouldEncrypt = true) noexcept {
    if (shouldEncrypt) {
      encryptedValue() = encrypt(val);
    }

    return _value.set(val);
  }

  inline constexpr int64_t set(int64_t val,
                               bool shouldEncrypt = true) noexcept {
    encryptedValue() = encrypt(val);
    return _value.set(val);
  }

  inline constexpr uint64_t set(uint64_t val,
                                bool shouldEncrypt = true) noexcept {
    if (shouldEncrypt) {
      encryptedValue() = encrypt(val);
    }

    return _value.set(val);
  }

  inline constexpr float set(float val, bool shouldEncrypt = true) noexcept {
    if (shouldEncrypt) {
      encryptedValue() = encrypt(val);
    }

    return _value.set(val);
  }

  inline constexpr vec4_t set(vec4_t val, bool shouldEncrypt = true) noexcept {
    if (shouldEncrypt) {
      encryptedValue() = encrypt(val);
    }

    return _value.set(val);
  }

  /*
    Note: this needs particular handling in the `dvar` struct
    to either re-use the `reset` value string pointer or re-allocate the string
    in the SL string pool, then assign with `sl::CopyString`. Direct assignment
    with a new string pointer will lead to unexpected behaviour.

    The caller also needs to set `shouldEncrypt` to `false` for string-type
    dvar values, and `true` for enum-type dvar values.
  */
  // TODO: overrides for `std::string`, `std::string_view`
  inline constexpr const char *set(const char *val,
                                   bool shouldEncrypt = true) noexcept {
    if (shouldEncrypt) {
      encryptedValue() = encrypt(val);
    }
    return _value.set(val);
  }
  inline constexpr const char *set(const std::string_view &val,
                                   bool shouldEncrypt) {
    return set(val.data(), shouldEncrypt);
  }
  inline constexpr const char *set(const std::string &val,
                                   bool shouldEncrypt = true) {
    return set(val.c_str(), shouldEncrypt);
  }
  inline constexpr DvarColor set(DvarColor val,
                                 bool shouldEncrypt = true) noexcept {
    if (shouldEncrypt) {
      encryptedValue() = encrypt(val);
    }
    return _value.set(val);
  }
};
ASSERT_CPP03_POD(EncryptionCapableDvarValue);
ASSERT_SIZE(EncryptionCapableDvarValue, 0x20);
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
  PrimitiveLimit<uint32_t> unsignedInteger;
  PrimitiveLimit<int64_t> integer64;
  PrimitiveLimit<uint64_t> unsignedInt64;
  PrimitiveLimit<float> value;
  PrimitiveLimit<vec_t> vector;

  inline constexpr bool contains(const char *val) noexcept {
    if (val) {
      for (int32_t i = 0; i < enumeration.stringCount; ++i) {
        if (enumeration.strings[i] &&
            std::strcmp(enumeration.strings[i], val) == 0) {
          return true;
        }
      }
    }

    return false;
  }

  inline constexpr bool contains([[maybe_unused]] bool val) noexcept {
    return true;
  }

  inline constexpr bool contains(int32_t val) noexcept {
    assert(integer.min <= integer.max &&
           "invalid DvarLimits domain - minimum is greater than maximum!");
    return val >= integer.min && val <= integer.max;
  }

  inline constexpr bool contains(uint32_t val) noexcept {
    assert(unsignedInteger.min <= unsignedInteger.max &&
           "invalid DvarLimits domain - minimum is greater than maximum!");
    return val >= unsignedInteger.min && val <= unsignedInteger.max;
  }
  inline constexpr bool contains(int64_t val) noexcept {
    assert(integer64.min <= integer64.max &&
           "invalid DvarLimits domain - minimum is greater than maximum!");
    return val >= integer64.min && val <= integer64.max;
  }
  inline constexpr bool contains(uint64_t val) noexcept {
    assert(unsignedInt64.min <= unsignedInt64.max &&
           "invalid DvarLimits domain - minimum is greater than maximum!");
    return val >= unsignedInt64.min && val <= unsignedInt64.max;
  }
  inline constexpr bool contains(vec_t val) noexcept {
    assert(vector.min <= vector.max &&
           "invalid DvarLimits domain - minimum is greater than maximum!");
    return val >= vector.min && val <= vector.max;
  }

  inline constexpr bool contains(vec2_t val) noexcept {
    return contains(val.x) && contains(val.y);
  }

  inline constexpr bool contains(vec3_t val) noexcept {
    return contains(val.x) && contains(val.y) && contains(val.z);
  }

  inline constexpr bool contains(vec4_t val) noexcept {
    return contains(val.x) && contains(val.y) && contains(val.z) &&
           contains(val.w);
  }

  inline constexpr bool contains(DvarColor val) noexcept {
    return contains(static_cast<vec_t>(val.r)) &&
           contains(static_cast<vec_t>(val.g)) &&
           contains(static_cast<vec_t>(val.b)) &&
           contains(static_cast<vec_t>(val.a));
  }
};
ASSERT_CPP03_POD(DvarLimits);
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

  inline constexpr const uint64_t &encryptedValue() const noexcept {
    return _current.encryptedValue();
  }
  inline constexpr uint64_t &encryptedValue() noexcept {
    return _current.encryptedValue();
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

  dvarCallBack_t *modifiedCallback() noexcept;

  template <typename T>
    requires(!std::same_as<T, const char *> &&
             !std::same_as<T, const std::string_view &> &&
             !std::same_as<T, const std::string &>)
  std::optional<T> set(T val) noexcept;

  std::optional<std::string> set(const char *val) noexcept;
  std::optional<std::string> set(const std::string_view &val) noexcept;
  std::optional<std::string> set(const std::string &val) noexcept;

  inline int32_t get_int() noexcept { return resolve()->current().integer(); }
  inline constexpr uint32_t get_uint() noexcept {
    return resolve()->current().unsignedInt();
  }
  inline int64_t get_int64() noexcept {
    return resolve()->current().integer64();
  }
  inline uint64_t get_uint64() noexcept {
    return resolve()->current().unsignedInt64();
  }
  inline bool get_bool() noexcept { return resolve()->current().enabled(); }
  inline float get_float() noexcept { return resolve()->current().value(); }
  inline const char *get_cstring() noexcept {
    return resolve()->current().string();
  }
  inline std::optional<std::string_view> get_string() noexcept {
    const char *str = get_cstring();
    return str ? std::optional(std::string_view(str)) : std::nullopt;
  }
  template <typename T> inline constexpr T get() noexcept {
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

  constexpr operator EngineDependentDvar() noexcept;
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
ASSERT_CPP03_POD(dvar_t);

struct encryptedDvar_t : public dvar<EncryptionCapableDvarValue> {};
ASSERT_OFFSET(encryptedDvar_t, _debugName, 0x8);
ASSERT_OFFSET(encryptedDvar_t, _description, 0x10);
ASSERT_OFFSET(encryptedDvar_t, _flags, 0x18);
ASSERT_OFFSET(encryptedDvar_t, _type, 0x1C);
ASSERT_OFFSET(encryptedDvar_t, _modified, 0x20);
ASSERT_OFFSET(encryptedDvar_t, _current, 0x28);
ASSERT_SIZE(encryptedDvar_t, 0xA0);
ASSERT_CPP03_POD(encryptedDvar_t);

union EngineDependentDvarMut {
  dvar_t *sv;
  encryptedDvar_t *cl;

  inline std::optional<std::string> set(const char *val) {
    if (is_server()) {
      return sv->set(val);
    }
    return cl->set(val);
  }
  inline std::optional<float> set(float val) {
    if (is_server()) {
      return sv->set(val);
    }
    return cl->set(val);
  }
  inline std::optional<uint64_t> set(uint64_t val) {
    if (is_server()) {
      return sv->set(val);
    }
    return cl->set(val);
  }
  inline std::optional<int64_t> set(int64_t val) {
    if (is_server()) {
      return sv->set(val);
    }
    return cl->set(val);
  }
  inline std::optional<int32_t> set(int32_t val) {
    if (is_server()) {
      return sv->set(val);
    }
    return cl->set(val);
  }
  inline std::optional<uint32_t> set(uint32_t val) {
    if (is_server()) {
      return sv->set(val);
    }
    return cl->set(val);
  }
  inline std::optional<bool> set(bool val) {
    if (is_server()) {
      return sv->set(val);
    }
    return cl->set(val);
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

  inline uint64_t encryptedValue() const noexcept {
    if (is_server()) {
      return 0;
    }
    return cl->encryptedValue();
  }

  inline uint64_t encryptedValue() noexcept {
    if (is_server()) {
      return 0;
    }
    return cl->encryptedValue();
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
  inline ConstCharPtr &debugName() noexcept {
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
  inline ConstCharPtr &description() noexcept {
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

  inline constexpr bool nonnull() noexcept { return sv != nullptr; }
  inline constexpr bool null() noexcept { return sv == nullptr; }

  inline constexpr bool operator!() noexcept { return null(); }
  inline constexpr bool
  operator>(const EngineDependentDvarMut &rhs) const noexcept {
    return sv > rhs.sv;
  }
  inline constexpr bool
  operator<(const EngineDependentDvarMut &rhs) const noexcept {
    return sv < rhs.sv;
  }

  inline constexpr bool operator>(const EngineDependentDvarMut &rhs) noexcept {
    return sv > rhs.sv;
  }
  inline constexpr bool operator<(const EngineDependentDvarMut &rhs) noexcept {
    return sv < rhs.sv;
  }

  inline constexpr operator bool() noexcept { return nonnull(); }
  inline constexpr operator dvar_t *() noexcept { return sv; }
  inline constexpr operator encryptedDvar_t *() noexcept { return cl; }
  inline constexpr operator dvar_t *() const noexcept { return sv; }
  inline constexpr operator encryptedDvar_t *() const noexcept { return cl; }

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
ASSERT_CPP03_POD(EngineDependentDvarMut);

union EngineDependentDvar {
  const dvar_t *sv;
  const encryptedDvar_t *cl;

  inline uint64_t encryptedValue() const noexcept {
    if (is_server()) {
      return 0;
    }
    return cl->encryptedValue();
  }

  inline uint64_t encryptedValue() noexcept {
    if (is_server()) {
      return 0;
    }
    return cl->encryptedValue();
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
ASSERT_CPP03_POD(EngineDependentDvar);

inline constexpr bool operator==(const EngineDependentDvarMut &lhs,
                                 const EngineDependentDvar &rhs) noexcept {
  return lhs.sv == rhs.sv;
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

typedef fastcallPtr_t<void(EngineDependentDvarMut dvar)> modifiedCallback;

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