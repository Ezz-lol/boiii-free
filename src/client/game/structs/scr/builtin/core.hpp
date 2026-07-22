#pragma once

#include "game/structs/scr/core.hpp"
namespace game {
namespace scr {
namespace builtin {
typedef uint32_t ScrVarCanonicalName_t;

typedef fastcallPtr_t<void(scriptInstance_t inst)> BuiltinFunction;
typedef fastcallPtr_t<void(scriptInstance_t inst, scr_entref_t *entref)>
    BuiltinMethod;

enum class BuiltinTypeFlags : uint32_t {
  DEFAULT = 0x0,
  DEVBLOCK_ONLY = 0x1,
};

#pragma pack(push, 1)
union BuiltinType {
  uint32_t raw;

  struct {
    uint32_t devblockOnly : 1;

    uint32_t _unknownB1 : 1;
    uint32_t _unknownB2 : 1;
    uint32_t _unknownB3 : 1;
    uint32_t _unknownB4 : 1;
    uint32_t _unknownB5 : 1;
    uint32_t _unknownB6 : 1;
    uint32_t _unknownB7 : 1;
    uint32_t _unknownB8 : 1;
    uint32_t _unknownB9 : 1;
    uint32_t _unknownB10 : 1;
    uint32_t _unknownB11 : 1;
    uint32_t _unknownB12 : 1;
    uint32_t _unknownB13 : 1;
    uint32_t _unknownB14 : 1;
    uint32_t _unknownB15 : 1;
    uint32_t _unknownB16 : 1;
    uint32_t _unknownB17 : 1;
    uint32_t _unknownB18 : 1;
    uint32_t _unknownB19 : 1;
    uint32_t _unknownB20 : 1;
    uint32_t _unknownB21 : 1;
    uint32_t _unknownB22 : 1;
    uint32_t _unknownB23 : 1;
    uint32_t _unknownB24 : 1;
    uint32_t _unknownB25 : 1;
    uint32_t _unknownB26 : 1;
    uint32_t _unknownB27 : 1;
    uint32_t _unknownB28 : 1;
    uint32_t _unknownB29 : 1;
    uint32_t _unknownB30 : 1;
    uint32_t _unknownB31 : 1;
  };
  BuiltinTypeFlags flag;

  inline constexpr explicit operator bool() const noexcept { return raw != 0; }
  inline constexpr bool has(BuiltinType mask) const noexcept {
    return (raw & mask.raw) == mask.raw;
  }
  inline constexpr bool has_any(BuiltinType mask) const noexcept {
    return (raw & mask.raw) != 0;
  }

  friend inline constexpr bool operator==(BuiltinType lhs,
                                          BuiltinType rhs) noexcept {
    return lhs.raw == rhs.raw;
  }
  friend inline constexpr bool operator!=(BuiltinType lhs,
                                          BuiltinType rhs) noexcept {
    return lhs.raw != rhs.raw;
  }

  friend inline constexpr BuiltinType operator|(BuiltinType lhs,
                                                BuiltinType rhs) noexcept {
    return BuiltinType{lhs.raw | rhs.raw};
  }
  friend inline constexpr BuiltinType operator&(BuiltinType lhs,
                                                BuiltinType rhs) noexcept {
    return BuiltinType{lhs.raw & rhs.raw};
  }
  friend inline constexpr BuiltinType operator^(BuiltinType lhs,
                                                BuiltinType rhs) noexcept {
    return BuiltinType{lhs.raw ^ rhs.raw};
  }

  inline constexpr BuiltinType &operator|=(BuiltinType rhs) noexcept {
    raw |= rhs.raw;
    return *this;
  }
  inline constexpr BuiltinType &operator&=(BuiltinType rhs) noexcept {
    raw &= rhs.raw;
    return *this;
  }
  inline constexpr BuiltinType &operator^=(BuiltinType rhs) noexcept {
    raw ^= rhs.raw;
    return *this;
  }

  friend inline constexpr BuiltinType operator~(BuiltinType val) noexcept {
    return BuiltinType{~val.raw};
  }
};
constexpr BuiltinType DEFAULT_BUILTIN_TYPE =
    BuiltinType{.flag = BuiltinTypeFlags::DEFAULT};
ASSERT_SIZE(BuiltinType, sizeof(uint32_t));
#pragma pack(pop)

constexpr uint32_t MIN_BUILTIN_ARGS = 0;
constexpr uint32_t MAX_BUILTIN_ARGS = 0x7FFFFFFF;

#pragma pack(push, 1)
template <typename ActionFunc> struct BuiltinDef {
  ScrVarCanonicalName_t canonId;
  uint32_t min_args;
  uint32_t max_args;
  uint8_t _padding0C[4];
  ActionFunc actionFunc;
  BuiltinType type;
  uint8_t _padding1C[4];
};
#pragma pack(pop)

typedef BuiltinDef<BuiltinFunction> BuiltinFunctionDef;
ASSERT_SIZE(BuiltinFunctionDef, 0x20);

typedef BuiltinDef<BuiltinMethod> BuiltinMethodDef;
ASSERT_SIZE(BuiltinMethodDef, 0x20);
} // namespace builtin
} // namespace scr
} // namespace game