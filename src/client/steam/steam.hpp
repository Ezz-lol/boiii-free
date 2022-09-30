#pragma once

//#define STEAM_EXPORT extern "C" __declspec(dllexport)
#define STEAM_EXPORT

struct raw_steam_id final
{
	unsigned int account_id : 32;
	unsigned int account_instance : 20;
	unsigned int account_type : 4;
	int universe : 8;
};

typedef union
{
	raw_steam_id raw;
	unsigned long long bits;
} steam_id;

#pragma pack( push, 1 )
struct raw_game_id final
{
	unsigned int app_id : 24;
	unsigned int type : 8;
	unsigned int mod_id : 32;
};

typedef union
{
	raw_game_id raw;
	unsigned long long bits;
} game_id;
#pragma pack( pop )

#include "interfaces/apps.hpp"
#include "interfaces/user.hpp"
#include "interfaces/utils.hpp"
#include "interfaces/friends.hpp"
#include "interfaces/user_stats.hpp"
#include "interfaces/game_server.hpp"
#include "interfaces/networking.hpp"
#include "interfaces/matchmaking.hpp"
#include "interfaces/matchmaking_servers.hpp"
#include "interfaces/remote_storage.hpp"
#include "interfaces/screenshots.hpp"
#include "interfaces/game_server_stats.hpp"
#include "interfaces/http.hpp"
#include "interfaces/unified_messages.hpp"
#include "interfaces/controller.hpp"
#include "interfaces/client.hpp"
#include "interfaces/ugc.hpp"

namespace steam
{
	class callbacks
	{
	public:
		class base
		{
		public:
			base() : flags_(0), callback_(0)
			{
			}

			virtual void run(void* pv_param) = 0;
			virtual void run(void* pv_param, bool failure, uint64_t handle) = 0;
			virtual int get_callback_size_bytes() = 0;

			int get_i_callback() const { return callback_; }
			void set_i_callback(const int i_callback) { callback_ = i_callback; }

		protected:
			~base() = default;

			unsigned char flags_;
			int callback_;
		};

		struct result final
		{
			void* data{};
			int size{};
			int type{};
			uint64_t call{};
		};

		static uint64_t register_call();

		static void register_callback(base* handler, int callback);
		static void unregister_callback(base* handler);

		static void register_call_result(uint64_t call, base* result);
		static void unregister_call_result(uint64_t call, base* result);

		static void return_call(void* data, int size, int type, uint64_t call);
		static void run_callbacks();

	private:
		static uint64_t call_id_;
		static std::recursive_mutex mutex_;
		static std::map<uint64_t, bool> calls_;
		static std::map<uint64_t, base*> result_handlers_;
		static std::vector<result> results_;
		static std::vector<base*> callback_list_;
	};

	STEAM_EXPORT bool SteamAPI_RestartAppIfNecessary();
	STEAM_EXPORT bool SteamAPI_Init();
	STEAM_EXPORT void SteamAPI_RegisterCallResult(callbacks::base* result, uint64_t call);
	STEAM_EXPORT void SteamAPI_RegisterCallback(callbacks::base* handler, int callback);
	STEAM_EXPORT void SteamAPI_RunCallbacks();
	STEAM_EXPORT void SteamAPI_Shutdown();
	STEAM_EXPORT void SteamAPI_UnregisterCallResult(callbacks::base* result, const uint64_t call);
	STEAM_EXPORT void SteamAPI_UnregisterCallback(callbacks::base* handler);
	STEAM_EXPORT const char* SteamAPI_GetSteamInstallPath();

	STEAM_EXPORT void* SteamGameServer_GetHSteamPipe();
	STEAM_EXPORT void* SteamGameServer_GetHSteamUser();

	STEAM_EXPORT void* SteamAPI_GetHSteamUser();
	STEAM_EXPORT void* SteamAPI_GetHSteamPipe();

	STEAM_EXPORT void* SteamInternal_CreateInterface(const char* interfacename);

	STEAM_EXPORT bool SteamInternal_GameServer_Init();
	STEAM_EXPORT bool SteamGameServer_Init();
	STEAM_EXPORT void SteamGameServer_RunCallbacks();
	STEAM_EXPORT void SteamGameServer_Shutdown();

	STEAM_EXPORT friends* SteamFriends();
	STEAM_EXPORT matchmaking* SteamMatchmaking();
	STEAM_EXPORT game_server* SteamGameServer();
	STEAM_EXPORT networking* SteamNetworking();
	STEAM_EXPORT remote_storage* SteamRemoteStorage();
	STEAM_EXPORT user* SteamUser();
	STEAM_EXPORT utils* SteamUtils();
	STEAM_EXPORT apps* SteamApps();
	STEAM_EXPORT user_stats* SteamUserStats();
}
