#pragma once

#include <functional>
#include <game/game.hpp>
namespace sv {
using EnterWorldTask = std::function<void(
    (game::sv::client_s * client, game::user::usercmd_t *cmd))>;
using RemoveTask =
    std::function<void(game::sv::client_s *client, const char *reason)>;

void on_cliententerworld(const EnterWorldTask &callback);
void on_removeclient(const RemoveTask &callback);

game::ClientNum_t get_client_num(game::sv::client_s *client);
game::sv::client_s *get_client(game::ClientNum_t clientNum);
} // namespace sv