#include <std_include.hpp>
#include "../steam.hpp"
#include "component/party.hpp"
#include "component/network.hpp"
#include "utils/string.hpp"

namespace steam
{
	namespace
	{
		gameserveritem_t* get_server_item()
		{
			static gameserveritem_t server{};
			server.m_NetAdr.m_usConnectionPort = 28960;
			server.m_NetAdr.m_usQueryPort = 28960;

			uint32_t address{};
			inet_pton(AF_INET, "192.168.178.34", &address);
			server.m_NetAdr.m_unIP = ntohl(address);

			server.m_nPing = 10;
			server.m_bHadSuccessfulResponse = true;
			server.m_bDoNotRefresh = false;
			strcpy_s(server.m_szGameDir, "usermaps");
			strcpy_s(server.m_szMap, "mp_nuketown_x");
			strcpy_s(server.m_szGameDescription, "Example BO^3I^5I^6I ^7Server");
			server.m_nAppID = 311210;
			server.m_nPlayers = 0;
			server.m_nMaxPlayers = 18;
			server.m_nBotPlayers = 0;
			server.m_bPassword = false;
			server.m_bSecure = true;
			server.m_ulTimeLastPlayed = 0;
			server.m_nServerVersion = 1000;
			strcpy_s(server.m_szServerName, "BO^3I^5I^6I ^7Server");
			strcpy_s(server.m_szGameTags,
			         R"(\gametype\gun\dedicated\true\ranked\true\hardcore\false\zombies\false\modName\\playerCount\0)");
			server.m_steamID = steam_id();

			return &server;
		}

		gameserveritem_t create_server_item(const game::netadr_t& address, const ::utils::info_string& info)
		{
			gameserveritem_t server{};
			server.m_NetAdr.m_usConnectionPort = address.port;
			server.m_NetAdr.m_usQueryPort = address.port;
			server.m_NetAdr.m_unIP = address.addr;
			server.m_nPing = 10;
			server.m_bHadSuccessfulResponse = true;
			server.m_bDoNotRefresh = false;
			strcpy_s(server.m_szGameDir, "");
			strcpy_s(server.m_szMap, info.get("mapname").data());
			strcpy_s(server.m_szGameDescription, "Example BO^3I^5I^6I ^7Server");
			server.m_nAppID = 311210;
			server.m_nPlayers = 0;
			server.m_nMaxPlayers = 18;
			server.m_nBotPlayers = 0;
			server.m_bPassword = false;
			server.m_bSecure = true;
			server.m_ulTimeLastPlayed = 0;
			server.m_nServerVersion = 1000;
			strcpy_s(server.m_szServerName, info.get("sv_hostname").data());

			const auto playmode = info.get("playmode");
			const auto mode = game::eModes(std::atoi(playmode.data()));

			const auto* tags = ::utils::string::va(
				R"(\gametype\%s\dedicated\true\ranked\true\hardcore\false\zombies\%s\modName\\playerCount\0)",
				info.get("gametype").data(), mode == game::MODE_ZOMBIES ? "true" : "false");

			strcpy_s(server.m_szGameTags, tags);
			server.m_steamID.bits = strtoull(info.get("xuid").data(), nullptr, 16);

			return server;
		}
	}

	void* matchmaking_servers::RequestInternetServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                     matchmaking_server_list_response* pRequestServersResponse)
	{
		pRequestServersResponse->ServerResponded(reinterpret_cast<void*>(1), 0);
		pRequestServersResponse->RefreshComplete(reinterpret_cast<void*>(1), eServerResponded);
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
	}

	gameserveritem_t* matchmaking_servers::GetServerDetails(void* hRequest, int iServer)
	{
		return get_server_item();
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
		return (reinterpret_cast<void*>(1) == hRequest) ? 1 : 0;
	}

	void matchmaking_servers::RefreshServer(void* hRequest, int iServer)
	{
	}

	void* matchmaking_servers::PingServer(unsigned int unIP, unsigned short usPort,
	                                      matchmaking_ping_response* pRequestServersResponse)
	{
		auto response = pRequestServersResponse;
		const auto addr = network::address_from_ip(htonl(unIP), usPort);

		OutputDebugStringA(::utils::string::va("Sending: %u", (uint32_t)usPort));

		party::query_server(
			addr, [response](const bool success, const game::netadr_t& host, const ::utils::info_string& info)
			{
				OutputDebugStringA(::utils::string::va("Responded: %s", success ? "true" : "false"));
				if (success)
				{
					auto server_item = create_server_item(host, info);
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
