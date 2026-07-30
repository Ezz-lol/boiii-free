#pragma once

#include "core.hpp"
#include "game/structs/ui/he.hpp"
namespace game {
namespace scr {

inline constexpr bool scr_entref_t::is_hudelem() const noexcept {
  // TODO: define `classnum`s as a discrete enumeration
  return classnum == 1 && u.hudElemIndex < game::ui::he::HUD_ELEMENT_POOL_SIZE;
}

namespace he {} // namespace he
} // namespace scr
} // namespace game