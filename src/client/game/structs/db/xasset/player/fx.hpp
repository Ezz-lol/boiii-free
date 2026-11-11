#pragma once

#include <game/structs/db/xasset/core.hpp>
#include <game/structs/macros.hpp>

namespace game {
namespace db {
namespace xasset {
namespace player {
namespace fx {

// Verified
PACKED(struct PlayerFXTable {
  const char *name;
  FxEffectDefHandle wallRunHand;
  SurfaceFXTableDefPtr footstep;
  SurfaceFXTableDefPtr footstepTracker;
  SurfaceFXTableDefPtr land;
  SurfaceFXTableDefPtr jumpStart;
  SurfaceFXTableDefPtr leapStart;
  SurfaceFXTableDefPtr doubleJumpStart;
  SurfaceFXTableDefPtr slideLeftKnee;
  SurfaceFXTableDefPtr slideRightKnee;
  SurfaceFXTableDefPtr slideLeftFoot;
  FxEffectDefHandle playerJet;
  FxEffectDefHandle playerJetWeak;
  FxEffectDefHandle playerJetEMP;
  FxEffectDefHandle underWaterBreathing;
  FxEffectDefHandle splashSmall;
  FxEffectDefHandle splashMed;
  FxEffectDefHandle splashLarge;
  FxEffectDefHandle splashMassive;
});
ASSERT_SIZE(PlayerFXTable, 0x98);
} // namespace fx
} // namespace player
} // namespace xasset
} // namespace db
} // namespace game
