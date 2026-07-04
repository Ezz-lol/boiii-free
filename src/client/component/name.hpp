#pragma once
#include <string>
#include "com.hpp"
#include "sv.hpp"
#include "game_event.hpp"
#include "game/game.hpp"

namespace name {
const char *get_player_name();

void set_name_override(game::ClientNum_t client_num, const std::string &n);
void set_clan_abbrev_override(game::ClientNum_t client_num,
                              const std::string &t);
void clear_name_override(game::ClientNum_t client_num);
void clear_clan_abbrev_override(game::ClientNum_t client_num);
bool has_name_override(game::ClientNum_t client_num);
std::optional<std::string> get_name_override(game::ClientNum_t client_num);
bool has_clan_abbrev_override(game::ClientNum_t client_num);
std::optional<std::string>
get_clan_abbrev_override(game::ClientNum_t client_num);
void sync_name_override_to_clients(game::ClientNum_t client_num);
void sync_clan_abbrev_override_to_clients(game::ClientNum_t client_num);
void sync_name_reset_to_clients(game::ClientNum_t client_num);
void sync_clan_abbrev_reset_to_clients(game::ClientNum_t client_num);
void sync_all_overrides_to_client(game::ClientNum_t target_client);
void client_update(game::sv::client_s *cl);
void trigger_client_update(game::ClientNum_t client_num);
} // namespace name