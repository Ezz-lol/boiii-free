#pragma once
#include <cstdint>

namespace steam
{
	using HSteamPipe = uint64_t;
	using HSteamUser = uint64_t;

	class client
	{
	public:
		~client() = default;

		virtual HSteamPipe CreateSteamPipe();
		virtual bool BReleaseSteamPipe(HSteamPipe hSteamPipe);
		virtual HSteamUser ConnectToGlobalUser(HSteamPipe hSteamPipe);
		virtual HSteamUser CreateLocalUser(HSteamPipe* phSteamPipe, uint32_t eAccountType);
		virtual void ReleaseUser(HSteamPipe hSteamPipe, HSteamUser hUser);
		virtual void* GetISteamUser(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamGameServer(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void SetLocalIPBinding(uint32_t unIP, uint16_t usPort);
		virtual void* GetISteamFriends(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamUtils(HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamMatchmaking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamMatchmakingServers(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamGenericInterface(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamUserStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamGameServerStats(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamApps(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamNetworking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamRemoteStorage(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamScreenshots(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void RunFrame();
		virtual uint32_t GetIPCCallCount();
		virtual void SetWarningMessageHook(void* pFunction);
		virtual bool BShutdownIfAllPipesClosed();
		virtual void* GetISteamHTTP(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamUnifiedMessages(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamController(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamUGC(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamAppList(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamMusic(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamMusicRemote(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamHTMLSurface(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void Set_SteamAPI_CPostAPIResultInProcess(void* func);
		virtual void Remove_SteamAPI_CPostAPIResultInProcess(void* func);
		virtual void Set_SteamAPI_CCheckCallbackRegisteredInProcess(void* func);
		virtual void* GetISteamInventory(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamVideo(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
		virtual void* GetISteamParentalSettings(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion);
	};
}
