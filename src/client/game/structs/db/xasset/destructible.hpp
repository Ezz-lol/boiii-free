#pragma once

#include "game/structs/db/xasset/core.hpp"
#include "game/structs/db/xasset/phys.hpp"
#include <cstdint>

namespace game {

namespace snd {
struct SoundsImpactTable;
typedef SoundsImpactTable *SoundsImpactTablePtr;
} // namespace snd

namespace db {
namespace xasset {
namespace destructible {

PACKED(struct DestructibleStage {
  scr::ScrString_t showBone;
  scr::ScrString_t cosmeticShowBones[4];
  float breakHealth;
  float maxTime;
  uint32_t flags;
  db::xasset::FxEffectDefHandle baseEffect;
  scr::ScrString_t baseEffectTag;
  scr::ScrString_t breakAnim;
  db::xasset::FxEffectDefHandle breakEffect;
  scr::ScrString_t breakEffectTag;
  uint8_t _padding3C[4];
  const char *breakSound;
  const char *breakNotify;
  const char *loopSound;
  db::xasset::xmodel::XModelPtr spawnModel[3];
  db::xasset::phys::PhysPreset *physPreset;
});
ASSERT_SIZE(DestructibleStage, 0x78);

PACKED(struct DestructiblePiece {
  DestructibleStage stages[7];
  uint8_t parentPiece;
  uint8_t _padding151[3];
  float parentDamagePercent;
  float bulletDamageScale;
  float explosiveDamageScale;
  float meleeDamageScale;
  float impactDamageScale;
  float entityDamageTransfer;
  uint8_t _padding16C[4];
  phys::PhysConstraintsPtr physConstraints;
  int32_t health;
  uint8_t _padding17C[4];
  const char *damageSound;
  db::xasset::FxEffectDefHandle burnEffect;
  const char *burnSound;
  scr::ScrString_t enableLabel;
  XPartBits hideBones;
  uint8_t _padding1CC[4];
});
ASSERT_SIZE(DestructiblePiece, 0x3C8);

PACKED(struct DestructibleDef {
  const char *name;
  db::xasset::xmodel::XModelPtr model;
  db::xasset::xmodel::XModelPtr pristineModel;
  int32_t numPieces;
  uint8_t _padding1C[4];
  DestructiblePiece *pieces;
  qboolean clientOnly;
  qboolean syncBaseHealthWithEntity;
});
// Verified
ASSERT_SIZE(DestructibleDef, 0x30);
} // namespace destructible
} // namespace xasset
} // namespace db
} // namespace game