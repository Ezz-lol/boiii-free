#pragma once
#include <game/game.hpp>
#include <optional>
#include <string>

namespace name {
const char *get_player_name();

void set_name_override(game::ClientNum_t client_num, const std::string &name);
void set_clan_abbrev_override(game::ClientNum_t client_num,
                              const std::string &tag);
void clear_name_override(game::ClientNum_t client_num);
void clear_clan_abbrev_override(game::ClientNum_t client_num);
std::optional<std::string> get_name_override(game::ClientNum_t client_num);
std::optional<std::string>
get_clan_abbrev_override(game::ClientNum_t client_num);
} // namespace name
