#include <std_include.hpp>
#include "../steam.hpp"

#include "game/game.hpp"

#include "component/party.hpp"
#include "component/network.hpp"
#include "component/server_list.hpp"
#include "component/friends.hpp"
#include "component/steam_proxy.hpp"

#include <utils/string.hpp>
#include <utils/concurrency.hpp>

namespace steam
{
	namespace
	{
		struct server
		{
			bool handled{false};
			game::netadr_t address{};
			gameserveritem_t server_item{};
		};

		auto* const internet_request = reinterpret_cast<void*>(1);
		auto* const favorites_request = reinterpret_cast<void*>(4);
		auto* const history_request = reinterpret_cast<void*>(5);
		auto* const friends_request = reinterpret_cast<void*>(3);

		using servers = std::vector<server>;

		::utils::concurrency::container<servers> internet_servers{};
		::utils::concurrency::container<servers> favorites_servers{};
		::utils::concurrency::container<servers> history_servers{};
		::utils::concurrency::container<servers> friends_servers{};
		std::atomic<matchmaking_server_list_response*> internet_response{};
		std::atomic<matchmaking_server_list_response*> favorites_response{};
		std::atomic<matchmaking_server_list_response*> history_response{};
		std::atomic<matchmaking_server_list_response*> friends_response{};

		template <typename T>
		void copy_safe(T& dest, const char* in)
		{
			::utils::string::copy(dest, in);
			::utils::string::strip_material(dest, dest, std::extent_v<T>);
		}

		gameserveritem_t create_server_item(const game::netadr_t& address, const ::utils::info_string& info,
		                                    const uint32_t ping, const bool success)
		{
			const auto sub_protocol = atoi(info.get("sub_protocol").data());

			gameserveritem_t server{};
			server.m_NetAdr.m_usConnectionPort = address.port;
			server.m_NetAdr.m_usQueryPort = address.port;
			server.m_NetAdr.m_unIP = ntohl(address.addr);
			server.m_nPing = static_cast<int>(ping);
			server.m_bHadSuccessfulResponse = success;
			server.m_bDoNotRefresh = false;

			copy_safe(server.m_szGameDir, "");
			copy_safe(server.m_szMap, info.get("mapname").data());
			copy_safe(server.m_szGameDescription, info.get("description").data());

			server.m_nAppID = (sub_protocol == SUB_PROTOCOL || sub_protocol == (SUB_PROTOCOL - 1)) ? 311210 : 0;
			server.m_nPlayers = atoi(info.get("clients").data());
			server.m_nMaxPlayers = atoi(info.get("sv_maxclients").data());
			server.m_nBotPlayers = atoi(info.get("bots").data());
			server.m_bPassword = info.get("isPrivate") == "1";
			server.m_bSecure = true;
			server.m_ulTimeLastPlayed = 0;
			server.m_nServerVersion = 1000;

			copy_safe(server.m_szServerName, info.get("hostname").data());

			const auto playmode = info.get("playmode");
			const auto mode = static_cast<game::eModes>(std::atoi(playmode.data()));

			const auto* tags = ::utils::string::va(
				R"(\gametype\%s\dedicated\%s\ranked\false\hardcore\%s\zombies\%s\playerCount\%d\bots\%d\modName\%s\)",
				info.get("gametype").data(),
				info.get("dedicated") == "1" ? "true" : "false",
				info.get("hc") == "1" ? "true" : "false",
				mode == game::MODE_ZOMBIES ? "true" : "false",
				server.m_nPlayers,
				atoi(info.get("bots").data()),
				info.get("modName").data());

			copy_safe(server.m_szGameTags, tags);

			server.m_steamID.bits = strtoull(info.get("xuid").data(), nullptr, 16);

			return server;
		}

		void handle_server_respone(const bool success, const game::netadr_t& host, const ::utils::info_string& info,
		                           const uint32_t ping, ::utils::concurrency::container<servers>& server_list,
		                           std::atomic<matchmaking_server_list_response*>& response, void* request)
		{
			bool all_handled = false;
			std::optional<int> index{};
			server_list.access([&](servers& srvs)
			{
				size_t i = 0;
				for (; i < srvs.size(); ++i)
				{
					if (srvs[i].address == host)
					{
						break;
					}
				}

				if (i >= srvs.size())
				{
					return;
				}

				index = static_cast<int>(i);

				auto& srv = srvs[i];
				srv.handled = true;
				srv.server_item = create_server_item(host, info, ping, success);


				for (const auto& entry : srvs)
				{
					if (!entry.handled)
					{
						return;
					}
				}

				all_handled = true;
			});

			const auto res = response.load();
			if (!index || !res)
			{
				return;
			}

			if (success)
			{
				res->ServerResponded(request, *index);
			}
			else
			{
				res->ServerFailedToRespond(request, *index);
			}

			if (all_handled)
			{
				res->RefreshComplete(request, eServerResponded);
			}
		}

		void handle_internet_server_response(const bool success, const game::netadr_t& host,
		                                     const ::utils::info_string& info,
		                                     const uint32_t ping)
		{
			handle_server_respone(success, host, info, ping, internet_servers, internet_response, internet_request);
		}


		void handle_favorites_server_response(const bool success, const game::netadr_t& host,
		                                      const ::utils::info_string& info,
		                                      const uint32_t ping)
		{
			handle_server_respone(success, host, info, ping, favorites_servers, favorites_response, favorites_request);
		}

		void handle_history_server_response(const bool success, const game::netadr_t& host,
		                                    const ::utils::info_string& info,
		                                    const uint32_t ping)
		{
			handle_server_respone(success, host, info, ping, history_servers, history_response, history_request);
		}

		void handle_friends_server_response(const bool success, const game::netadr_t& host,
		                                    const ::utils::info_string& info,
		                                    const uint32_t ping)
		{
			handle_server_respone(success, host, info, ping, friends_servers, friends_response, friends_request);
		}

		void ping_server(const game::netadr_t& server, party::query_callback callback)
		{
			party::query_server(server, callback);
		}
	}

	void* matchmaking_servers::RequestInternetServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                     matchmaking_server_list_response* pRequestServersResponse)
	{
		internet_response = pRequestServersResponse;

		server_list::request_servers([](const bool success, const std::unordered_set<game::netadr_t>& s)
		{
			const auto res = internet_response.load();
			if (!res)
			{
				return;
			}

			if (!success)
			{
				res->RefreshComplete(internet_request, eServerFailedToRespond);
				return;
			}

			if (s.empty())
			{
				res->RefreshComplete(internet_request, eNoServersListedOnMasterServer);
				return;
			}

			internet_servers.access([&s](servers& srvs)
			{
				srvs = {};
				srvs.reserve(s.size());

				for (auto& address : s)
				{
					server new_server{};
					new_server.address = address;
					new_server.server_item = create_server_item(address, {}, 0, false);

					srvs.push_back(new_server);
				}
			});

			for (auto& srv : s)
			{
				ping_server(srv, handle_internet_server_response);
			}
		});

		return internet_request;
	}

	void* matchmaking_servers::RequestLANServerList(unsigned int iApp,
	                                                matchmaking_server_list_response* pRequestServersResponse)
	{
		return reinterpret_cast<void*>(2);
	}

	void* matchmaking_servers::RequestFriendsServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                    matchmaking_server_list_response* pRequestServersResponse)
	{
		friends_response = pRequestServersResponse;

		auto friend_infos = ::friends::get_friend_server_addresses();

		const auto res = friends_response.load();
		if (!res)
		{
			return friends_request;
		}

		if (friend_infos.empty())
		{
			res->RefreshComplete(friends_request, eNoServersListedOnMasterServer);
			return friends_request;
		}

		std::vector<std::pair<game::netadr_t, std::string>> addr_names;
		for (const auto& info : friend_infos)
		{
			auto addr = network::address_from_string(info.address);
			if (addr.type != game::NA_BAD)
			{
				addr_names.emplace_back(addr, info.player_name);
			}
		}

		if (addr_names.empty())
		{
			res->RefreshComplete(friends_request, eNoServersListedOnMasterServer);
			return friends_request;
		}

		friends_servers.access([&addr_names](servers& srvs)
		{
			srvs = {};
			srvs.reserve(addr_names.size());

			for (auto& [address, name] : addr_names)
			{
				server new_server{};
				new_server.address = address;
				new_server.server_item = create_server_item(address, {}, 0, false);

				copy_safe(new_server.server_item.m_szServerName, name.c_str());

				srvs.push_back(new_server);
			}
		});

		for (auto& [addr, name] : addr_names)
		{
			ping_server(addr, handle_friends_server_response);
		}

		return friends_request;
	}

	void* matchmaking_servers::RequestFavoritesServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                      matchmaking_server_list_response* pRequestServersResponse)
	{
		favorites_response = pRequestServersResponse;

		auto& srvs = server_list::get_favorite_servers();
		srvs.access([&](std::unordered_set<game::netadr_t> s)
		{
			const auto res = favorites_response.load();
			if (!res)
			{
				return;
			}

			if (s.empty())
			{
				res->RefreshComplete(favorites_request, eNoServersListedOnMasterServer);
				return;
			}

			favorites_servers.access([s](servers& srvs)
			{
				srvs = {};
				srvs.reserve(s.size());

				for (auto& address : s)
				{
					server new_server{};
					new_server.address = address;
					new_server.server_item = create_server_item(address, {}, 0, false);

					srvs.push_back(new_server);
				}
			});

			for (auto& srv : s)
			{
				ping_server(srv, handle_favorites_server_response);
			}
		});

		return favorites_request;
	}

	void* matchmaking_servers::RequestHistoryServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                    matchmaking_server_list_response* pRequestServersResponse)
	{
		history_response = pRequestServersResponse;

		auto& srvs = server_list::get_recent_servers();
		srvs.access([&](std::vector<game::netadr_t> s)
		{
			const auto res = history_response.load();
			if (!res)
			{
				return;
			}

			if (s.empty())
			{
				res->RefreshComplete(history_request, eNoServersListedOnMasterServer);
				return;
			}

			history_servers.access([&s](servers& srvs)
			{
				srvs = {};
				srvs.reserve(s.size());

				for (auto& address : s)
				{
					server new_server{};
					new_server.address = address;
					new_server.server_item = create_server_item(address, {}, 0, false);

					srvs.push_back(new_server);
				}
			});

			for (auto& srv : s)
			{
				ping_server(srv, handle_history_server_response);
			}
		});

		return history_request;
	}

	void* matchmaking_servers::RequestSpectatorServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                      matchmaking_server_list_response* pRequestServersResponse)
	{
		return reinterpret_cast<void*>(6);
	}

	void matchmaking_servers::ReleaseRequest(void* hServerListRequest)
	{
		if (internet_request == hServerListRequest)
		{
			internet_response = nullptr;
		}
		if (favorites_request == hServerListRequest)
		{
			favorites_response = nullptr;
		}
		if (history_request == hServerListRequest)
		{
			history_response = nullptr;
		}
		if (friends_request == hServerListRequest)
		{
			friends_response = nullptr;
		}
	}

	gameserveritem_t* matchmaking_servers::GetServerDetails(void* hRequest, int iServer)
	{
		if (internet_request != hRequest && favorites_request != hRequest && history_request != hRequest && friends_request != hRequest)
		{
			return nullptr;
		}

		auto& servers_list = hRequest == favorites_request ? favorites_servers
		                   : hRequest == history_request ? history_servers
		                   : hRequest == friends_request ? friends_servers
		                   : internet_servers;

		thread_local gameserveritem_t server_item{};
		return servers_list.access<gameserveritem_t*>([iServer](const servers& s) -> gameserveritem_t*
		{
			if (iServer < 0 || static_cast<size_t>(iServer) >= s.size())
			{
				return nullptr;
			}

			server_item = s[iServer].server_item;
			return &server_item;
		});
	}

	void matchmaking_servers::CancelQuery(void* hRequest)
	{
	}

	void matchmaking_servers::RefreshQuery(void* hRequest)
	{
	}

	bool matchmaking_servers::IsRefreshing(void* hRequest)
	{
		return false;
	}

	int matchmaking_servers::GetServerCount(void* hRequest)
	{
		if (internet_request != hRequest && favorites_request != hRequest && history_request != hRequest && friends_request != hRequest)
		{
			return 0;
		}

		auto& servers_list = hRequest == favorites_request ? favorites_servers
		                   : hRequest == history_request ? history_servers
		                   : hRequest == friends_request ? friends_servers
		                   : internet_servers;
		return servers_list.access<int>([](const servers& s)
		{
			return static_cast<int>(s.size());
		});
	}

	void matchmaking_servers::RefreshServer(void* hRequest, const int iServer)
	{
		if (internet_request != hRequest && favorites_request != hRequest && history_request != hRequest && friends_request != hRequest)
		{
			return;
		}

		std::optional<game::netadr_t> address{};
		auto& servers_list = hRequest == favorites_request ? favorites_servers
		                   : hRequest == history_request ? history_servers
		                   : hRequest == friends_request ? friends_servers
		                   : internet_servers;
		servers_list.access([&](const servers& s)
		{
			if (iServer < 0 || static_cast<size_t>(iServer) >= s.size())
			{
				return;
			}

			address = s[iServer].address;
		});

		if (address)
		{
			auto callback = hRequest == favorites_request ? handle_favorites_server_response
			              : hRequest == history_request ? handle_history_server_response
			              : hRequest == friends_request ? handle_friends_server_response
			              : handle_internet_server_response;
			ping_server(*address, callback);
		}
	}

	void* matchmaking_servers::PingServer(const unsigned int unIP, const unsigned short usPort,
	                                      matchmaking_ping_response* pRequestServersResponse)
	{
		auto response = pRequestServersResponse;
		const auto addr = network::address_from_ip(htonl(unIP), usPort);

		party::query_server(
			addr, [response](const bool success, const game::netadr_t& host, const ::utils::info_string& info,
			                 const uint32_t ping)
			{
				if (success)
				{
					auto server_item = create_server_item(host, info, ping, success);
					response->ServerResponded(server_item);
				}
				else
				{
					response->ServerFailedToRespond();
				}
			});

		return reinterpret_cast<void*>(static_cast<uint64_t>(7 + rand()));
	}

	int matchmaking_servers::PlayerDetails(unsigned int unIP, unsigned short usPort, void* pRequestServersResponse)
	{
		return 0;
	}

	int matchmaking_servers::ServerRules(unsigned int unIP, unsigned short usPort, void* pRequestServersResponse)
	{
		return 0;
	}

	void matchmaking_servers::CancelServerQuery(int hServerQuery)
	{
	}
}
