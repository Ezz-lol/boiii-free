#pragma once

#include "command.hpp"
#include <game/game.hpp>

namespace client_command {
using clientCommandHandler_t = fastcallPtr_t<void(
    game::level::gentity_s *ent, const command::params_sv &params)>;

void register_handler(std::string name, clientCommandHandler_t cmd);
} // namespace client_command
