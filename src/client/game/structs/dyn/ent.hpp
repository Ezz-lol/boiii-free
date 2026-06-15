#pragma once

#include <cstdint>
#include "../snd/snd.hpp"
#include "../scr/core.hpp"
#include "../phys.hpp"
#include "../gfx/core.hpp"

namespace game {
namespace dyn {
namespace ent {

struct DynEntityPose {
  gfx::GfxPlacement pose;
  float radius;
  gfx::GfxEntityPreFrame prevFrame;
};

#pragma pack(push, 1)
struct DynEntityServer {
  uint16_t flags;
  uint8_t _padding02[2];
  int health;
};
#pragma pack(pop)

struct DynEntityColl {
  uint16_t sector;
  uint16_t nextEntInSector;
  vec3_t linkMins;
  vec3_t linkMaxs;
  contents_t contents;
};

#pragma pack(push, 1)
struct DynEntityClient {
  intptr_t physObjId;
  uint16_t flags;
  uint8_t _padding0A[2];
  int32_t health;
  uint16_t burnTime;
  uint16_t fadeTime;
  int32_t physicsStartTime;
  phys::LocalClientFxUniqueHandle fxHandles[10];
  int32_t numFxHandles;
  uint8_t _padding6C[4];
};

#pragma pack(pop)

#pragma pack(push, 1)
struct DynEntityDef {
  gfx::GfxPlacement pose;
  uint8_t _padding1C[4];
  db::xasset::XModel *xModel;
  db::xasset::XModel *destroyedxModel;
  uint16_t brushModel;
  uint16_t physicsBrushModel;
  uint8_t _padding34[4];
  db::xasset::FxEffectDefHandle destroyFx;
  snd::SndAliasId destroySound;
  uint8_t _padding44[4];
  db::xasset::FxEffectDefHandle rattleFx;
  snd::SndAliasId rattleSound;
  uint8_t _padding54[4];
  db::xasset::FxEffectDefHandle splashUpFx[3];
  phys::PhysPreset *physPreset;
  int16_t physConstraints[4];
  int32_t health;
  int32_t flags;
  contents_t contents;
  scr::ScrString_t targetname;
  scr::ScrString_t target;
  uint8_t _padding94[4];
};
ASSERT_SIZE(DynEntityDef, 0x98);
#pragma pack(pop)

typedef uint16_t DynEntityId;
} // namespace ent
} // namespace dyn
} // namespace game