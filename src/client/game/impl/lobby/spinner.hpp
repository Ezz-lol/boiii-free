#pragma once

#include "game/game.hpp"

namespace game {
namespace lobby {
namespace spinner {

/*
   Same as engine's s_spinnerName (@ 0x1433909B0), but
   reorders the first two spinner entries such that they are in order of
   SpinnerSystem enumeration value, allowing O(1) lookup by system.
*/
inline constexpr SpinnerPool<Spinner> s_spinnerName{
    .join_result = Spinner(SpinnerSystem::JOIN_RESULT, "join_result"),
    .join_progress = Spinner(SpinnerSystem::JOIN_PROGRESS, "join_progress"),
    .lobby_search = Spinner(SpinnerSystem::LOBBY_SEARCH, "lobby_search"),
    .lobby_create = Spinner(SpinnerSystem::LOBBY_CREATE, "lobby_create"),
    .lobby_leave = Spinner(SpinnerSystem::LOBBY_LEAVE, "lobby_leave"),
    .lobby_move = Spinner(SpinnerSystem::LOBBY_MOVE, "lobby_move"),

};

constexpr const char *GetName_Impl(SpinnerSystem system) {
  return s_spinnerName[+system].name;
}

void LobbySpinner_Update_Impl(SpinnerSystem system, int32_t state);
} // namespace spinner
} // namespace lobby
} // namespace game
