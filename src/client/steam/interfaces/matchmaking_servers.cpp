#include <std_include.hpp>
#include "../steam.hpp"

#include "game/game.hpp"

#include "component/party.hpp"
#include "component/network.hpp"
#include "component/server_list.hpp"

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

		using servers = std::vector<server>;

		::utils::concurrency::container<servers> queried_servers{};
		std::atomic<matchmaking_server_list_response*> current_response{};

		gameserveritem_t create_server_item(const game::netadr_t& address, const ::utils::info_string& info,
		                                    const uint32_t ping, const bool success)
		{
			gameserveritem_t server{};
			server.m_NetAdr.m_usConnectionPort = address.port;
			server.m_NetAdr.m_usQueryPort = address.port;
			server.m_NetAdr.m_unIP = address.addr;
			server.m_nPing = static_cast<int>(ping);
			server.m_bHadSuccessfulResponse = success;
			server.m_bDoNotRefresh = false;
			strcpy_s(server.m_szGameDir, "");
			strcpy_s(server.m_szMap, info.get("mapname").data());
			strcpy_s(server.m_szGameDescription, "Example BO^3I^5I^6I ^7Server");
			server.m_nAppID = 311210;
			server.m_nPlayers = atoi(info.get("clients").data());
			server.m_nMaxPlayers = atoi(info.get("sv_maxclients").data());
			server.m_nBotPlayers = atoi(info.get("bots").data());
			server.m_bPassword = false;
			server.m_bSecure = true;
			server.m_ulTimeLastPlayed = 0;
			server.m_nServerVersion = 1000;
			strcpy_s(server.m_szServerName, info.get("hostname").data());

			const auto playmode = info.get("playmode");
			const auto mode = game::eModes(std::atoi(playmode.data()));

			const auto* tags = ::utils::string::va(
				R"(\gametype\%s\dedicated\%s\ranked\false\hardcore\false\zombies\%s\modName\\playerCount\0)",
				info.get("gametype").data(),
				info.get("dedicated") == "1" ? "true" : "false",
				mode == game::MODE_ZOMBIES ? "true" : "false");

			strcpy_s(server.m_szGameTags, tags);
			server.m_steamID.bits = strtoull(info.get("xuid").data(), nullptr, 16);

			return server;
		}

		void handle_server_respone(const bool success, const game::netadr_t& host, const ::utils::info_string& info,
		                           const uint32_t ping)
		{
			bool all_handled = false;
			std::optional<int> index{};
			queried_servers.access([&](servers& srvs)
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

			const auto res = current_response.load();
			if (!index || !res)
			{
				return;
			}

			if (success)
			{
				res->ServerResponded(reinterpret_cast<void*>(1), *index);
			}
			else
			{
				res->ServerFailedToRespond(reinterpret_cast<void*>(1), *index);
			}

			if (all_handled)
			{
				res->RefreshComplete(reinterpret_cast<void*>(1), eServerResponded);
			}
		}

		void ping_server(const game::netadr_t& server)
		{
			party::query_server(server, handle_server_respone);
		}
	}

	void* matchmaking_servers::RequestInternetServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                     matchmaking_server_list_response* pRequestServersResponse)
	{
		current_response = pRequestServersResponse;

		server_list::request_servers([](const bool success, const std::unordered_set<game::netadr_t>& s)
		{
			const auto res = current_response.load();
			if (!res)
			{
				return;
			}

			if (!success)
			{
				res->RefreshComplete(reinterpret_cast<void*>(1), eServerFailedToRespond);
				return;
			}

			if (s.empty())
			{
				res->RefreshComplete(reinterpret_cast<void*>(1), eNoServersListedOnMasterServer);
				return;
			}

			queried_servers.access([&s](servers& srvs)
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
				ping_server(srv);
			}

		});

		return reinterpret_cast<void*>(1);
	}

	void* matchmaking_servers::RequestLANServerList(unsigned int iApp,
	                                                matchmaking_server_list_response* pRequestServersResponse)
	{
		return reinterpret_cast<void*>(2);
	}

	void* matchmaking_servers::RequestFriendsServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                    matchmaking_server_list_response* pRequestServersResponse)
	{
		return reinterpret_cast<void*>(3);
	}

	void* matchmaking_servers::RequestFavoritesServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                      matchmaking_server_list_response* pRequestServersResponse)
	{
		return reinterpret_cast<void*>(4);
	}

	void* matchmaking_servers::RequestHistoryServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                    matchmaking_server_list_response* pRequestServersResponse)
	{
		return reinterpret_cast<void*>(5);
	}

	void* matchmaking_servers::RequestSpectatorServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                      matchmaking_server_list_response* pRequestServersResponse)
	{
		return reinterpret_cast<void*>(6);
	}

	void matchmaking_servers::ReleaseRequest(void* hServerListRequest)
	{
		current_response = nullptr;
	}

	gameserveritem_t* matchmaking_servers::GetServerDetails(void* hRequest, int iServer)
	{
		if (reinterpret_cast<void*>(1) != hRequest)
		{
			return nullptr;
		}

		static thread_local gameserveritem_t server_item{};
		return queried_servers.access<gameserveritem_t*>([iServer](const servers& s) -> gameserveritem_t*
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
		if (reinterpret_cast<void*>(1) != hRequest)
		{
			return 0;
		}

		return queried_servers.access<int>([](const servers& s)
		{
			return static_cast<int>(s.size());
		});
	}

	void matchmaking_servers::RefreshServer(void* hRequest, int iServer)
	{
	}

	void* matchmaking_servers::PingServer(unsigned int unIP, unsigned short usPort,
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
