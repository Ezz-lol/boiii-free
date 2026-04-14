#pragma once
#include <utils/info_string.hpp>

#include "game/game.hpp"

namespace party {
using query_callback_func = void(bool success, const game::net::netadr_t &host,
                                 const utils::info_string &info, uint32_t ping);
using query_callback = std::function<query_callback_func>;

void query_server(const game::net::netadr_t &host, query_callback callback);

game::net::netadr_t get_connected_server();

bool is_host(const game::net::netadr_t &addr);

void join_session(const game::net::netadr_t &addr, const std::string &hostname,
                  uint64_t xuid, game::eModes mode);

uint16_t get_local_port();

std::string get_server_hostname();
int get_server_max_clients();
void clear_server_info();

void connect_to_lobby_with_mode(const game::net::netadr_t &addr,
                                game::eModes mode, const std::string &mapname,
                                const std::string &gametype,
                                const std::string &usermap_id,
                                const std::string &mod_id);

game::net::netadr_t get_connect_host();
} // namespace party