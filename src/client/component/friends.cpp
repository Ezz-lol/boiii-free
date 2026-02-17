#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "friends.hpp"

#include "game/game.hpp"
#include "game/utils.hpp"
#include "network.hpp"
#include "party.hpp"
#include "scheduler.hpp"
#include "steam/steam.hpp"
#include "steam_proxy.hpp"
#include "workshop.hpp"
#include "name.hpp"

#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/concurrency.hpp>
#include <utils/http.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace friends
{
	namespace
	{
		constexpr auto FRIENDS_FILE = "boiii_players/user/friends.json";
		constexpr int MAX_FRIENDS = 200;

		std::mutex public_ip_mutex;
		std::string cached_public_ip;
		std::atomic_bool public_ip_fetched{false};

		void fetch_public_ip()
		{
			try
			{
				auto resp = utils::http::get_data("https://api.ipify.org", {}, {}, 1);
				if (resp.has_value() && !resp->empty())
				{
					std::lock_guard lock(public_ip_mutex);
					cached_public_ip = *resp;
					public_ip_fetched.store(true);
				}
			}
			catch (...) {}
		}

		struct friend_state
		{
			std::vector<friend_entry> list;
		};

		utils::concurrency::container<friend_state> friends_data;

		std::mutex invite_mutex;
		std::string pending_invite_connect;

		void save_friends()
		{
			friends_data.access([](const friend_state& state)
			{
				rapidjson::StringBuffer buf;
				rapidjson::Writer<rapidjson::StringBuffer> w(buf);
				w.StartArray();
				for (const auto& f : state.list)
				{
					w.StartObject();
					w.Key("steam_id"); w.Uint64(f.steam_id);
					w.Key("name"); w.String(f.name.c_str());
					w.EndObject();
				}
				w.EndArray();
				utils::io::write_file(FRIENDS_FILE, std::string(buf.GetString(), buf.GetSize()));
			});
		}

		void load_friends()
		{
			if (!utils::io::file_exists(FRIENDS_FILE)) return;

			std::string data;
			if (!utils::io::read_file(FRIENDS_FILE, &data) || data.empty()) return;

			rapidjson::Document doc;
			if (doc.Parse(data.c_str()).HasParseError() || !doc.IsArray()) return;

			friends_data.access([&doc](friend_state& state)
			{
				state.list.clear();

				for (auto& item : doc.GetArray())
				{
					if (!item.IsObject()) continue;

					friend_entry entry{};

					auto si = item.FindMember("steam_id");
					if (si != item.MemberEnd())
					{
						if (si->value.IsUint64()) entry.steam_id = si->value.GetUint64();
						else if (si->value.IsString()) entry.steam_id = std::strtoull(si->value.GetString(), nullptr, 10);
					}

					// backwards compat with old "xuid" field
					if (entry.steam_id == 0)
					{
						auto xi = item.FindMember("xuid");
						if (xi != item.MemberEnd())
						{
							if (xi->value.IsUint64()) entry.steam_id = xi->value.GetUint64();
							else if (xi->value.IsString()) entry.steam_id = std::strtoull(xi->value.GetString(), nullptr, 10);
						}
					}

					if (entry.steam_id == 0) continue;

					auto ni = item.FindMember("name");
					if (ni != item.MemberEnd() && ni->value.IsString())
						entry.name = ni->value.GetString();
					else
						entry.name = "Unknown";

					entry.state = status::offline;

					bool exists = false;
					for (const auto& e : state.list)
					{
						if (e.steam_id == entry.steam_id)
						{
							exists = true;
							break;
						}
					}
					if (!exists) state.list.push_back(std::move(entry));
				}
			});
		}

		// Resolves the address other players can use to connect to us
		std::string get_own_connect_address()
		{
			auto local_port = party::get_local_port();

			if (game::Com_IsInGame())
			{
				auto connected = party::get_connected_server();

				if (connected.type == game::NA_LOOPBACK)
				{
					if (public_ip_fetched.load())
					{
						std::lock_guard lock(public_ip_mutex);
						if (!cached_public_ip.empty())
						{
							return utils::string::va("%s:%u", cached_public_ip.c_str(),
								static_cast<unsigned>(local_port));
						}
					}
				}
				else if ((connected.type == game::NA_IP || connected.type == game::NA_RAWIP)
					&& connected.port >= 1024
					&& connected.ipv4.a != 127 && connected.addr != 0)
				{
					bool is_private = (connected.ipv4.a == 10)
						|| (connected.ipv4.a == 172 && connected.ipv4.b >= 16 && connected.ipv4.b <= 31)
						|| (connected.ipv4.a == 192 && connected.ipv4.b == 168);

					if (is_private && public_ip_fetched.load())
					{
						std::lock_guard lock(public_ip_mutex);
						if (!cached_public_ip.empty())
						{
							return utils::string::va("%s:%u", cached_public_ip.c_str(), connected.port);
						}
					}

					return utils::string::va("%u.%u.%u.%u:%u",
						connected.ipv4.a, connected.ipv4.b, connected.ipv4.c, connected.ipv4.d,
						connected.port);
				}
			}

			if (public_ip_fetched.load())
			{
				std::lock_guard lock(public_ip_mutex);
				if (!cached_public_ip.empty())
				{
					return utils::string::va("%s:%u", cached_public_ip.c_str(),
						static_cast<unsigned>(local_port));
				}
			}

			std::string local_ip;
			SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (sock != INVALID_SOCKET)
			{
				sockaddr_in target{};
				target.sin_family = AF_INET;
				target.sin_port = htons(53);
				inet_pton(AF_INET, "8.8.8.8", &target.sin_addr);

				if (connect(sock, reinterpret_cast<sockaddr*>(&target), sizeof(target)) == 0)
				{
					sockaddr_in local{};
					int len = sizeof(local);
					getsockname(sock, reinterpret_cast<sockaddr*>(&local), &len);
					char buf[INET_ADDRSTRLEN]{};
					inet_ntop(AF_INET, &local.sin_addr, buf, sizeof(buf));
					local_ip = buf;
				}
				closesocket(sock);
			}

			if (local_ip.empty()) return "";
			return utils::string::va("%s:%u", local_ip.c_str(), static_cast<unsigned>(local_port));
		}
	}

	void add_friend(uint64_t steam_id, const std::string& fname)
	{
		if (steam_id == 0) return;

		friends_data.access([&](friend_state& state)
		{
			for (auto& e : state.list)
			{
				if (e.steam_id == steam_id)
				{
					if (!fname.empty() && fname != "Unknown") e.name = fname;
					return;
				}
			}
			if (static_cast<int>(state.list.size()) >= MAX_FRIENDS) return;

			friend_entry entry{};
			entry.steam_id = steam_id;
			entry.name = fname.empty() ? "Unknown" : fname;
			entry.state = status::offline;
			state.list.push_back(std::move(entry));
		});
		save_friends();
	}

	void remove_friend(uint64_t steam_id)
	{
		friends_data.access([&](friend_state& state)
		{
			std::erase_if(state.list, [steam_id](const friend_entry& e)
			{
				return e.steam_id == steam_id;
			});
		});
		save_friends();
	}

	bool is_friend(uint64_t steam_id)
	{
		bool found = false;
		friends_data.access([&](const friend_state& state)
		{
			for (const auto& e : state.list)
				if (e.steam_id == steam_id) { found = true; break; }
		});
		return found;
	}

	int get_friend_count()
	{
		int count = 0;
		friends_data.access([&](const friend_state& state)
		{
			count = static_cast<int>(state.list.size());
		});
		return count;
	}

	friend_entry get_friend_by_index(int index)
	{
		friend_entry result{};
		friends_data.access([&](const friend_state& state)
		{
			if (index >= 0 && index < static_cast<int>(state.list.size()))
				result = state.list[index];
		});
		return result;
	}

	std::vector<friend_entry> get_friends()
	{
		std::vector<friend_entry> result;
		friends_data.access([&](const friend_state& state) { result = state.list; });
		return result;
	}

	bool invite_to_game(uint64_t steam_id)
	{
		auto connect_str = get_own_connect_address();
		if (connect_str.empty()) return false;

		std::string mapname = game::get_dvar_string("mapname");
		std::string gametype = game::get_dvar_string("g_gametype");
		int playmode = game::Com_SessionMode_GetMode();
		std::string mod_id = workshop::get_mod_publisher_id();
		uint64_t own_steam_id = steam_proxy::get_own_steam_id();
		std::string own_name = name::get_player_name();
		if (own_name.empty()) own_name = steam_proxy::get_player_name();
		if (own_name.empty()) own_name = "Player";

		// enriched format: addr|map|gametype|mode|mod|sender_id|sender_name
		auto enriched = utils::string::va("%s|%s|%s|%d|%s|%llu|%s",
			connect_str.c_str(), mapname.c_str(), gametype.c_str(), playmode, mod_id.c_str(),
			own_steam_id, own_name.c_str());

		steam_proxy::set_rich_presence("connect", connect_str);
		steam_proxy::set_rich_presence("boiii_game_info", enriched);

		if (!is_friend(steam_id))
		{
			auto target_name = steam_proxy::get_steam_friend_name(steam_id);
			if (target_name.empty()) target_name = "Friend";
			add_friend(steam_id, target_name);
		}
		else
		{
			// Update name if we have a better one from Steam
			auto target_name = steam_proxy::get_steam_friend_name(steam_id);
			if (!target_name.empty())
				add_friend(steam_id, target_name);
		}

		try
		{
			steam_proxy::invite_friend(steam_id, enriched);
			return true;
		}
		catch (...) {}
		return false;
	}

	std::string get_presence_server(uint64_t steam_id)
	{
		std::string result;
		friends_data.access([&](const friend_state& state)
		{
			for (const auto& e : state.list)
			{
				if (e.steam_id == steam_id)
				{
					result = e.server_address;
					break;
				}
			}
		});
		return result;
	}

	bool has_pending_invite()
	{
		std::lock_guard lock(invite_mutex);
		return !pending_invite_connect.empty();
	}

	std::string consume_pending_invite()
	{
		std::lock_guard lock(invite_mutex);
		auto result = std::move(pending_invite_connect);
		pending_invite_connect.clear();
		return result;
	}

	std::vector<friend_server_info> get_friend_server_addresses()
	{
		std::vector<friend_server_info> result;
		std::unordered_set<uint64_t> seen_ids;

		std::vector<friend_entry> all_friends;
		friends_data.access([&](const friend_state& state)
		{
			all_friends = state.list;
		});

		for (const auto& entry : all_friends)
		{
			if (entry.steam_id == 0 || seen_ids.count(entry.steam_id)) continue;
			seen_ids.insert(entry.steam_id);

			steam_proxy::request_friend_rich_presence(entry.steam_id);
			auto addr = steam_proxy::get_friend_rich_presence(entry.steam_id, "connect");

			if (addr.empty() && !entry.server_address.empty())
				addr = entry.server_address;

			// green online and red offline
			std::string color_prefix = addr.empty() ? "^1" : "^3";
			result.push_back({entry.steam_id, addr, color_prefix + entry.name});
		}

		return result;
	}

	std::string get_friend_game_info_by_address(const std::string& address)
	{
		if (address.empty()) return "";

		auto target = network::address_from_string(address);

		std::vector<friend_entry> all_friends;
		friends_data.access([&](const friend_state& state)
		{
			all_friends = state.list;
		});

		for (const auto& entry : all_friends)
		{
			if (entry.steam_id == 0) continue;

			steam_proxy::request_friend_rich_presence(entry.steam_id);
			auto game_info = steam_proxy::get_friend_rich_presence(entry.steam_id, "boiii_game_info");
			if (game_info.empty()) continue;

			auto parts = utils::string::split(game_info, '|');
			if (parts.empty()) continue;

			// Check if the address in the RP data matches the requested address
			if (parts[0] == address) return game_info;

			// Also try matching resolved addresses
			if (target.type != game::NA_BAD)
			{
				auto friend_addr = network::address_from_string(parts[0]);
				if (friend_addr.type != game::NA_BAD && network::are_addresses_equal(friend_addr, target))
					return game_info;
			}
		}

		return "";
	}

	bool connect_to_friend(uint64_t steam_id)
	{
		if (steam_id == 0) return false;

		// Check if friend is in our list and has a server address
		std::string addr_str;
		friends_data.access([&](const friend_state& state)
		{
			for (const auto& e : state.list)
			{
				if (e.steam_id == steam_id && !e.server_address.empty())
				{
					addr_str = e.server_address;
					break;
				}
			}
		});

		// Also try rich presence
		if (addr_str.empty())
		{
			steam_proxy::request_friend_rich_presence(steam_id);
			addr_str = steam_proxy::get_friend_rich_presence(steam_id, "connect");
		}

		if (addr_str.empty())
		{
			// Friend is not in-game / not reachable
			scheduler::once([]
			{
				game::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI,
					"Friend is not online or not in a joinable game.");
			}, scheduler::main);
			return false;
		}

		// Try enriched game info for proper mode/map connection
		auto game_info = steam_proxy::get_friend_rich_presence(steam_id, "boiii_game_info");
		if (!game_info.empty())
		{
			auto parts = utils::string::split(game_info, '|');
			if (parts.size() >= 4)
			{
				auto connect_addr = parts[0];
				auto mapname = parts[1];
				auto gametype = parts[2];
				auto mode = static_cast<game::eModes>(std::atoi(parts[3].c_str()));
				std::string mod_id = parts.size() >= 5 ? parts[4] : "";

				auto target = network::address_from_string(connect_addr);
				if (target.type != game::NA_BAD && !mapname.empty() && !gametype.empty())
				{
					game::Com_SessionMode_SetGameMode(game::MODE_GAME_MATCHMAKING_PLAYLIST);
					auto usermap_id = workshop::get_usermap_publisher_id(mapname);
					party::connect_to_lobby_with_mode(target, mode, mapname, gametype, usermap_id, mod_id);
					return true;
				}
			}
		}

		// Fallback: raw connect
		game::Cbuf_AddText(0, utils::string::va("connect %s\n", addr_str.c_str()));
		return true;
	}

	struct component final : client_component
	{
		void post_unpack() override
		{
			load_friends();

			scheduler::once([] { fetch_public_ip(); }, scheduler::async, 2000ms);

			// Poll for incoming Steam invites via callback 337
			scheduler::loop([]
			{
				try
				{
					uint64_t friend_id = 0;
					auto invite = steam_proxy::get_pending_game_invite(&friend_id);
					if (invite.empty()) return;

					// Try to get enriched game info from sender's RP
					if (friend_id != 0)
					{
						steam_proxy::request_friend_rich_presence(friend_id);
						auto game_info = steam_proxy::get_friend_rich_presence(friend_id, "boiii_game_info");
						if (!game_info.empty())
							invite = game_info;
					}

					// Auto-add sender as friend
					uint64_t sender_steam_id = friend_id;
					std::string sender_name;

					auto parts = utils::string::split(invite, '|');
					if (parts.size() >= 7)
					{
						auto parsed_id = std::strtoull(parts[5].c_str(), nullptr, 10);
						if (parsed_id != 0) sender_steam_id = parsed_id;
						sender_name = parts[6];
					}

					if (sender_steam_id != 0)
					{
						if (sender_name.empty())
							sender_name = steam_proxy::get_steam_friend_name(sender_steam_id);
						if (sender_name.empty()) sender_name = "Friend";
						add_friend(sender_steam_id, sender_name);
					}

					std::lock_guard lock(invite_mutex);
					pending_invite_connect = invite;
				}
				catch (...) {}
			}, scheduler::async, 2000ms);

			// Process accepted invites and joins connect to the host's server
			scheduler::loop([]
			{
				if (!has_pending_invite()) return;
				auto invite_data = consume_pending_invite();
				if (invite_data.empty()) return;

				steam_proxy::clear_invite_presence();

				auto parts = utils::string::split(invite_data, '|');
				if (parts.size() >= 4)
				{
					auto addr_str = parts[0];
					auto mapname = parts[1];
					auto gametype = parts[2];
					auto mode = static_cast<game::eModes>(std::atoi(parts[3].c_str()));
					std::string mod_id = parts.size() >= 5 ? parts[4] : "";

					auto target = network::address_from_string(addr_str);
					if (target.type != game::NA_BAD && !mapname.empty() && !gametype.empty())
					{
						game::Com_SessionMode_SetGameMode(game::MODE_GAME_MATCHMAKING_PLAYLIST);
						auto usermap_id = workshop::get_usermap_publisher_id(mapname);
						party::connect_to_lobby_with_mode(target, mode, mapname, gametype, usermap_id, mod_id);
						return;
					}
				}

				game::Cbuf_AddText(0, utils::string::va("connect %s\n", invite_data.c_str()));
			}, scheduler::main, 1000ms);

			// Publish our rich presence so friends can see us and join
			scheduler::loop([]
			{
				try
				{
					auto addr = get_own_connect_address();
					steam_proxy::set_rich_presence("connect", addr);

					if (!addr.empty() && game::Com_IsInGame())
					{
						std::string mapname = game::get_dvar_string("mapname");
						std::string gametype = game::get_dvar_string("g_gametype");
						int playmode = game::Com_SessionMode_GetMode();
						std::string mod_id = workshop::get_mod_publisher_id();
						uint64_t own_steam_id = steam_proxy::get_own_steam_id();
						std::string own_name = name::get_player_name();
						if (own_name.empty()) own_name = steam_proxy::get_player_name();
						if (own_name.empty()) own_name = "Player";

						auto enriched = utils::string::va("%s|%s|%s|%d|%s|%llu|%s",
							addr.c_str(), mapname.c_str(), gametype.c_str(), playmode, mod_id.c_str(),
							own_steam_id, own_name.c_str());
						steam_proxy::set_rich_presence("boiii_game_info", enriched);
					}
					else
					{
						steam_proxy::set_rich_presence("boiii_game_info", "");
					}

					steam_proxy::set_rich_presence("status",
						game::Com_IsInGame() ? "In Game" : "Online");
				}
				catch (...) {}
			}, scheduler::main, 10000ms);

			// Refresh friend online status via their rich presence
			scheduler::loop([]
			{
				try
				{
					friends_data.access([](friend_state& state)
					{
						for (auto& f : state.list)
						{
							steam_proxy::request_friend_rich_presence(f.steam_id);
							auto connect_rp = steam_proxy::get_friend_rich_presence(f.steam_id, "connect");
							auto status_rp = steam_proxy::get_friend_rich_presence(f.steam_id, "status");

							// Try to update friend's name from their enriched RP
							auto game_info_rp = steam_proxy::get_friend_rich_presence(f.steam_id, "boiii_game_info");
							if (!game_info_rp.empty())
							{
								auto rp_parts = utils::string::split(game_info_rp, '|');
								if (rp_parts.size() >= 7 && !rp_parts[6].empty())
								{
									f.name = rp_parts[6];
								}
							}

							if (!connect_rp.empty())
							{
								f.state = status::in_game;
								f.server_address = connect_rp;
							}
							else if (!status_rp.empty())
							{
								f.state = status::online;
								f.server_address.clear();
							}
							else
							{
								f.state = status::offline;
								f.server_address.clear();
							}
						}
					});
					save_friends();
				}
				catch (...) {}
			}, scheduler::async, 15000ms);
		}
	};
}

REGISTER_COMPONENT(friends::component)
