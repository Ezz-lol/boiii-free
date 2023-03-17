#include <std_include.hpp>
#include "../steam.hpp"
#include "../../component/steam_proxy.hpp"

namespace steam
{
	namespace
	{
		void* get_dummy()
		{
			static class blub
			{
			public:
				virtual uint64_t m0() { return 0; }
				virtual uint64_t m1() { return 0; }
				virtual uint64_t m2() { return 0; }
				virtual uint64_t m3() { return 0; }
				virtual uint64_t m4() { return 0; }
				virtual uint64_t m5() { return 0; }
				virtual uint64_t m6() { return 0; }
				virtual uint64_t m7() { return 0; }
				virtual uint64_t m8() { return 0; }
				virtual uint64_t m9() { return 0; }

				virtual uint64_t m10() { return 0; }
				virtual uint64_t m11() { return 0; }
				virtual uint64_t m12() { return 0; }
				virtual uint64_t m13() { return 0; }
				virtual uint64_t m14() { return 0; }
				virtual uint64_t m15() { return 0; }
				virtual uint64_t m16() { return 0; }
				virtual uint64_t m17() { return 0; }
				virtual uint64_t m18() { return 0; }
				virtual uint64_t m19() { return 0; }

				virtual uint64_t m20() { return 0; }
				virtual uint64_t m21() { return 0; }
				virtual uint64_t m22() { return 0; }
				virtual uint64_t m23() { return 0; }
				virtual uint64_t m24() { return 0; }
				virtual uint64_t m25() { return 0; }
				virtual uint64_t m26() { return 0; }
				virtual uint64_t m27() { return 0; }
				virtual uint64_t m28() { return 0; }
				virtual uint64_t m29() { return 0; }

				virtual uint64_t m30() { return 0; }
				virtual uint64_t m31() { return 0; }
				virtual uint64_t m32() { return 0; }
				virtual uint64_t m33() { return 0; }
				virtual uint64_t m34() { return 0; }
				virtual uint64_t m35() { return 0; }
				virtual uint64_t m36() { return 0; }
				virtual uint64_t m37() { return 0; }
				virtual uint64_t m38() { return 0; }
				virtual uint64_t m39() { return 0; }

				virtual uint64_t m40() { return 0; }
				virtual uint64_t m41() { return 0; }
				virtual uint64_t m42() { return 0; }
				virtual uint64_t m43() { return 0; }
				virtual uint64_t m44() { return 0; }
				virtual uint64_t m45() { return 0; }
				virtual uint64_t m46() { return 0; }
				virtual uint64_t m47() { return 0; }
				virtual uint64_t m48() { return 0; }
				virtual uint64_t m49() { return 0; }

				virtual uint64_t m50() { return 0; }
				virtual uint64_t m51() { return 0; }
				virtual uint64_t m52() { return 0; }
				virtual uint64_t m53() { return 0; }
				virtual uint64_t m54() { return 0; }
				virtual uint64_t m55() { return 0; }
				virtual uint64_t m56() { return 0; }
				virtual uint64_t m57() { return 0; }
				virtual uint64_t m58() { return 0; }
				virtual uint64_t m59() { return 0; }

				virtual uint64_t m60() { return 0; }
				virtual uint64_t m61() { return 0; }
				virtual uint64_t m62() { return 0; }
				virtual uint64_t m63() { return 0; }
				virtual uint64_t m64() { return 0; }
				virtual uint64_t m65() { return 0; }
				virtual uint64_t m66() { return 0; }
				virtual uint64_t m67() { return 0; }
				virtual uint64_t m68() { return 0; }
				virtual uint64_t m69() { return 0; }
			} x;
			return &x;
		}
	}

	HSteamPipe client::CreateSteamPipe()
	{
		return 1;
	}

	bool client::BReleaseSteamPipe(HSteamPipe hSteamPipe)
	{
		return true;
	}

	HSteamUser client::ConnectToGlobalUser(HSteamPipe hSteamPipe)
	{
		return 1;
	}

	HSteamUser client::CreateLocalUser(HSteamPipe* phSteamPipe, uint32_t eAccountType)
	{
		return 1;
	}

	void client::ReleaseUser(HSteamPipe hSteamPipe, HSteamUser hUser)
	{
	}

	void* client::GetISteamUser(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return SteamUser();
	}

	void* client::GetISteamGameServer(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return SteamGameServer();
	}

	void client::SetLocalIPBinding(uint32_t unIP, uint16_t usPort)
	{
	}

	void* client::GetISteamFriends(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return SteamFriends();
	}

	void* client::GetISteamUtils(HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return SteamUtils();
	}

	void* client::GetISteamMatchmaking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return SteamMatchmaking();
	}

	void* client::GetISteamMatchmakingServers(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		static matchmaking_servers u;
		return &u;
	}

	void* client::GetISteamGenericInterface(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return nullptr;
	}

	void* client::GetISteamUserStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return SteamUserStats();
	}

	void* client::GetISteamGameServerStats(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		static game_server_stats u;
		return &u;
	}

	void* client::GetISteamApps(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return SteamApps();
	}

	void* client::GetISteamNetworking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return SteamNetworking();
	}

	void* client::GetISteamRemoteStorage(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return SteamRemoteStorage();
	}

	void* client::GetISteamScreenshots(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		static screenshots s;
		return &s;
	}

	void client::RunFrame()
	{
	}

	uint32_t client::GetIPCCallCount()
	{
		return 0;
	}

	void client::SetWarningMessageHook(void* pFunction)
	{
	}

	bool client::BShutdownIfAllPipesClosed()
	{
		return true;
	}

	void* client::GetISteamHTTP(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		static http h;
		return &h;
	}

	void* client::GetISteamUnifiedMessages(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		static unified_messages u;
		return &u;
	}

	void* client::GetISteamController(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		static controller c;
		return &c;
	}

	void* client::GetISteamUGC(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		static auto x = []
		{
			steam_proxy::create_ugc();
			return 0;
		}();
		(void)x;

		static ugc u;
		return &u;
	}

	void* client::GetISteamAppList(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return get_dummy();
	}

	void* client::GetISteamMusic(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return get_dummy();
	}

	void* client::GetISteamMusicRemote(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return get_dummy();
	}

	void* client::GetISteamHTMLSurface(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return get_dummy();
	}

	void client::Set_SteamAPI_CPostAPIResultInProcess(void* func)
	{
	}

	void client::Remove_SteamAPI_CPostAPIResultInProcess(void* func)
	{
	}

	void client::Set_SteamAPI_CCheckCallbackRegisteredInProcess(void* func)
	{
	}

	void* client::GetISteamInventory(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return get_dummy();
	}

	void* client::GetISteamVideo(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return get_dummy();
	}

	void* client::GetISteamParentalSettings(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return get_dummy();
	}
}
