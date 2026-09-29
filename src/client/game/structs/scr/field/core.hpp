#pragma once

#include <cstdint>

#include "game/structs/macros.hpp"

namespace game {
namespace scr {
namespace field {
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

} // namespace field
} // namespace scr
} // namespace game
