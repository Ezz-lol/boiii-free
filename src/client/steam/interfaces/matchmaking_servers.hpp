#pragma once

namespace steam
{
	class matchmaking_servers
	{
	public:
		~matchmaking_servers() = default;

		virtual void* RequestInternetServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
		                                        void* pRequestServersResponse);
		virtual void* RequestLANServerList(unsigned int iApp, void* pRequestServersResponse);
		virtual void* RequestFriendsServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
		                                       void* pRequestServersResponse);
		virtual void* RequestFavoritesServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
		                                         void* pRequestServersResponse);
		virtual void* RequestHistoryServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
		                                       void* pRequestServersResponse);
		virtual void* RequestSpectatorServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
		                                         void* pRequestServersResponse);
		virtual void ReleaseRequest(void* hServerListRequest);
		virtual void* GetServerDetails(void* hRequest, int iServer);
		virtual void CancelQuery(void* hRequest);
		virtual void RefreshQuery(void* hRequest);
		virtual bool IsRefreshing(void* hRequest);
		virtual int GetServerCount(void* hRequest);
		virtual void RefreshServer(void* hRequest, int iServer);
		virtual int PingServer(unsigned int unIP, unsigned short usPort, void* pRequestServersResponse);
		virtual int PlayerDetails(unsigned int unIP, unsigned short usPort, void* pRequestServersResponse);
		virtual int ServerRules(unsigned int unIP, unsigned short usPort, void* pRequestServersResponse);
		virtual void CancelServerQuery(int hServerQuery);
	};
}
