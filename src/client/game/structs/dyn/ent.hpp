#pragma once

#include <cstdint>
#include <game/structs/gfx/core.hpp>
#include <game/structs/phys/core.hpp>
#include <game/structs/scr/core.hpp>
#include <game/structs/snd/snd.hpp>

namespace game {
namespace dyn {
namespace ent {

struct DynEntityPose {
  gfx::GfxPlacement pose;
  float radius;
  gfx::GfxEntityPreFrame prevFrame;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"pose\": {}, \"radius\": {}, \"prevFrame\": {} }}",
                       pose.serialize(), radius, prevFrame.serialize());
  }
};

PACKED(struct DynEntityServer {
  uint16_t flags;
  uint8_t _padding02[2];
  int health;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"flags\": 0x{:04X}, \"health\": {} }}", flags,
                       health);
  }
});

struct DynEntityColl {
  uint16_t sector;
  uint16_t nextEntInSector;
  vec3_t linkMins;
  vec3_t linkMaxs;
  contents_t contents;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"sector\": {}, \"nextEntInSector\": {}, \"linkMins\": {}, "
        "\"linkMaxs\": {}, \"contents\": 0x{:08X} }}",
        sector, nextEntInSector, linkMins.serialize(), linkMaxs.serialize(),
        contents);
  }
};

PACKED(struct DynEntityClient {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"physObjId\": {:p}, \"flags\": 0x{:04X}, \"health\": {}, "
        "\"burnTime\": {}, \"fadeTime\": {}, \"physicsStartTime\": {}, "
        "\"fxHandles\": {}, \"numFxHandles\": {} }}",
        reinterpret_cast<const void *>(physObjId), flags, health, burnTime,
        fadeTime, physicsStartTime, ::serialize(fxHandles), numFxHandles);
  }
});

PACKED(struct DynEntityDef {
  gfx::GfxPlacement pose;
  uint8_t _padding1C[4];
  db::xasset::xmodel::XModel *xModel;
  db::xasset::xmodel::XModel *destroyedxModel;
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
  game::db::xasset::phys::PhysPreset *physPreset;
  int16_t physConstraints[4];
  int32_t health;
  int32_t flags;
  contents_t contents;
  scr::ScrString_t targetname;
  scr::ScrString_t target;
  uint8_t _padding94[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"pose\": {}, \"xModel\": {:p}, \"destroyedxModel\": {:p}, "
        "\"brushModel\": {}, \"physicsBrushModel\": {}, \"destroyFx\": {}, "
        "\"destroySound\": 0x{:08X}, \"rattleFx\": {}, \"rattleSound\": "
        "0x{:08X}, "
        "\"splashUpFx\": {}, \"physPreset\": {:p}, \"physConstraints\": {}, "
        "\"health\": {}, \"flags\": 0x{:08X}, \"contents\": 0x{:08X}, "
        "\"targetname\": 0x{:08X}, \"target\": 0x{:08X} }}",
        pose.serialize(), reinterpret_cast<const void *>(xModel),
        reinterpret_cast<const void *>(destroyedxModel), brushModel,
        physicsBrushModel, ::serialize(destroyFx), destroySound,
        ::serialize(rattleFx), rattleSound,
        ::serialize(reinterpret_cast<const uintptr_t *const>(splashUpFx),
                    std::size(splashUpFx)),
        reinterpret_cast<const void *>(physPreset),
        ::serialize(physConstraints), health, flags, contents, targetname,
        target);
  }
});
ASSERT_SIZE(DynEntityDef, 0x98);

typedef uint16_t DynEntityId;
} // namespace ent
} // namespace dyn
} // namespace game
