#pragma once

namespace steam
{
	typedef enum EMatchMakingServerResponse
	{
		eServerResponded = 0,
		eServerFailedToRespond,
		eNoServersListedOnMasterServer
	} matchmaking_server_response;

	class servernetadr_t
	{
	public:
		uint16_t m_usConnectionPort;
		uint16_t m_usQueryPort;
		uint32_t m_unIP;
	};

	class gameserveritem_t
	{
	public:
		servernetadr_t m_NetAdr;
		int m_nPing;
		bool m_bHadSuccessfulResponse;
		bool m_bDoNotRefresh;
		char m_szGameDir[32];
		char m_szMap[32];
		char m_szGameDescription[64];
		uint32_t m_nAppID;
		int m_nPlayers;
		int m_nMaxPlayers;
		int m_nBotPlayers;
		bool m_bPassword;
		bool m_bSecure;
		uint32_t m_ulTimeLastPlayed;
		int m_nServerVersion;
		char m_szServerName[64];
		char m_szGameTags[128];
		steam_id m_steamID;
	};

	class matchmaking_ping_response
	{
	public:
		virtual void ServerResponded(gameserveritem_t& server) = 0;
		virtual void ServerFailedToRespond() = 0;
	};

	class matchmaking_server_list_response
	{
	public:
		virtual void ServerResponded(void* hRequest, int iServer) = 0;
		virtual void ServerFailedToRespond(void* hRequest, int iServer) = 0;
		virtual void RefreshComplete(void* hRequest, matchmaking_server_response response) = 0;
	};

	class matchmaking_servers
	{
	public:
		~matchmaking_servers() = default;

		virtual void* RequestInternetServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
		                                        matchmaking_server_list_response* pRequestServersResponse);
		virtual void* RequestLANServerList(unsigned int iApp,
		                                   matchmaking_server_list_response* pRequestServersResponse);
		virtual void* RequestFriendsServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
		                                       matchmaking_server_list_response* pRequestServersResponse);
		virtual void* RequestFavoritesServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
		                                         matchmaking_server_list_response* pRequestServersResponse);
		virtual void* RequestHistoryServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
		                                       matchmaking_server_list_response* pRequestServersResponse);
		virtual void* RequestSpectatorServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
		                                         matchmaking_server_list_response* pRequestServersResponse);
		virtual void ReleaseRequest(void* hServerListRequest);
		virtual gameserveritem_t* GetServerDetails(void* hRequest, int iServer);
		virtual void CancelQuery(void* hRequest);
		virtual void RefreshQuery(void* hRequest);
		virtual bool IsRefreshing(void* hRequest);
		virtual int GetServerCount(void* hRequest);
		virtual void RefreshServer(void* hRequest, int iServer);
		virtual void* PingServer(unsigned int unIP, unsigned short usPort, matchmaking_ping_response* pRequestServersResponse);
		virtual int PlayerDetails(unsigned int unIP, unsigned short usPort, void* pRequestServersResponse);
		virtual int ServerRules(unsigned int unIP, unsigned short usPort, void* pRequestServersResponse);
		virtual void CancelServerQuery(int hServerQuery);
	};
}
