#pragma once

#include <cstdint>

#include "core.hpp"

#include "game/structs/scr/primitives.hpp"
#include "game/structs/weapon.hpp"

namespace game {
namespace scr {
namespace field {
namespace weapon {
enum class WeaponFieldType : uint32_t {
  INVALID = 0x0,
  DEF = 0x1,
  VARIANT_DEF = 0x2,
};
IMPL_ENUM_OPERATORS(WeaponFieldType);

struct scr_weapon_field_s;

typedef fastcall_t<void(scriptInstance_t inst, game::weapon::Weapon weapon,
                        const scr_weapon_field_s *field)>
    ScriptCallbackWeapon;

PACKED(struct scr_weapon_field_s {
  ScrVarCanonicalName_t canonId;
  int32_t ofs;
  int32_t size;
  fieldtype_t type;
  WeaponFieldType weaponType;
  uint8_t _padding14[4];
  ScriptCallbackWeapon getter;
});

typedef scr_weapon_field_s scr_weapon_field_t;

} // namespace weapon
} // namespace field
} // namespace scr
} // namespace game
