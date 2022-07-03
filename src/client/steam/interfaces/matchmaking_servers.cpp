#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	namespace
	{
		gameserveritem_t* get_server_item()
		{
			static gameserveritem_t server{};
			server.m_NetAdr.m_usConnectionPort = 27017;
			server.m_NetAdr.m_usQueryPort = 27017;
			server.m_NetAdr.m_unIP = 0x7F000001;
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
			         R"(\gametype\gun\dedicated\true\ranked\true\hardcore\false\zombies\false\modName\usermaps\playerCount\0)");
			server.m_steamID = steam_id();

			return &server;
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
		pRequestServersResponse->ServerResponded(*get_server_item());
		return reinterpret_cast<void*>(7);
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
