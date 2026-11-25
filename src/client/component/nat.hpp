#pragma once

#include <game/game.hpp>

#include <functional>
#include <string>
#include <vector>

namespace nat {
struct friend_presence {
  uint64_t steam_id{};
  std::string token;
  std::string endpoint;
};

using lookup_callback = std::function<void(std::vector<friend_presence>)>;

std::string get_host_token();
std::string get_host_endpoint();
void begin_join(const std::string &token, const std::string &fallback_address,
                uint64_t friend_id);
bool set_open_to_friends(bool enabled);
std::string joinable_lobby();
// Optional caller lifetime cancels queued work. Strict custom joins use a fresh
// nonce token and never fall back to a raw match or another host.
void join_match_lobby(const game::net::netadr_t &host,
                      std::function<void()> fallback,
                      std::function<bool()> still_current = {},
                      bool strict_custom = false);
bool is_lobby_peer(const game::net::netadr_t &address);
std::vector<game::net::netadr_t> get_lobby_peers();
bool has_party_lobby();
bool in_game_lobby_of(const game::net::netadr_t &host);
void join_server_lobby(const game::net::netadr_t &server);
void leave_server_lobby();
bool has_party_members();
void send_party_to_server(const game::net::netadr_t &server);
bool is_rendezvous(const game::net::netadr_t &address);
void send_invite(uint64_t steam_id, const std::string &sender_name);
void refresh_friends(const std::vector<uint64_t> &steam_ids,
                     lookup_callback callback);
} // namespace nat
