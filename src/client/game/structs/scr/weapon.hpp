#pragma once

#include <cstdint>

#include "game/structs/scr/primitives.hpp"
#include "game/structs/weapon.hpp"

namespace game {
namespace scr {
namespace weapon {
enum class fieldtype_t : uint32_t {
  INT = 0x0,
  SHORT = 0x1,
  BYTE = 0x2,
  FLOAT = 0x3,
  LSTRING = 0x4,
  STRING = 0x5,
  VECTOR = 0x6,
  ENTITY = 0x7,
  ENTHANDLE = 0x8,
  ACTOR = 0x9,
  SENTIENT = 0xA,
  SENTIENTHANDLE = 0xB,
  CLIENT = 0xC,
  PATHNODE = 0xD,
  ACTORGROUP = 0xE,
  VECTORHACK = 0xF,
  OBJECT = 0x10,
  XMODEL_INDEX = 0x11,
  XMODEL = 0x12,
  BITFLAG = 0x13,
  BITFLAG64 = 0x14,
  FX = 0x15,
  WEAPON = 0x16,
  RUMBLE = 0x17,
  COUNT = 0x18
};
IMPL_ENUM_OPERATORS(fieldtype_t);

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
} // namespace scr
} // namespace game