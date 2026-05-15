#pragma once

#include <cstdint>
#include <game/game.hpp>
namespace auth {
extern const game::dvar_t *password;

uint64_t
get_guid(game::ControllerIndex_t controllerIndex = game::CONTROLLER_INDEX_0);
uint64_t get_guid(size_t client_num);
void clear_stored_guids();
void clear_stored_challenge();
bool send_fragmented_connect_packet(game::ControllerIndex_t controllerIndex,
                                    const game::net::netsrc_t sock,
                                    game::net::netadr_t *adr, const char *data,
                                    const int length);
} // namespace auth