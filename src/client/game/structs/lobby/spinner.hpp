#pragma once

#include <game/structs/lobby/core.hpp>
#include <game/structs/macros.hpp>

#include <cstdint>

namespace game {
namespace lobby {
namespace spinner {

enum class SpinnerSystem : int32_t {
  INVALID = -1,
  JOIN_RESULT = 0x0,
  JOIN_PROGRESS = 0x1,
  LOBBY_SEARCH = 0x2,
  LOBBY_CREATE = 0x3,
  LOBBY_LEAVE = 0x4,
  LOBBY_MOVE = 0x5,
  COUNT = 0x6,
};
IMPL_ENUM_OPERATORS(SpinnerSystem);

} // namespace spinner
} // namespace lobby
} // namespace game
