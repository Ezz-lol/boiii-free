#pragma once

#include "game/game.hpp"

namespace zombies::coop {
int wrap_score_model_chunk(game::lua::hks::lua_State* state, const char* name, int compiler_status) noexcept;
} // namespace zombies::coop
