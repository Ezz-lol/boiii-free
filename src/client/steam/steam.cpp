#include <std_include.hpp>
#include "steam.hpp"

#include <utils/nt.hpp>

namespace steam
{
	uint64_t callbacks::call_id_ = 0;
	std::recursive_mutex callbacks::mutex_;
	std::map<uint64_t, bool> callbacks::calls_;
	std::map<uint64_t, callbacks::base*> callbacks::result_handlers_;
	std::vector<callbacks::result> callbacks::results_;
	std::vector<callbacks::base*> callbacks::callback_list_;

	uint64_t callbacks::register_call()
	{
		std::lock_guard<std::recursive_mutex> _(mutex_);
		calls_[++call_id_] = false;
		return call_id_;
	}

	void callbacks::register_callback(base* handler, const int callback)
	{
		std::lock_guard<std::recursive_mutex> _(mutex_);
		handler->set_i_callback(callback);
		callback_list_.push_back(handler);
	}

	void callbacks::unregister_callback(base* handler)
	{
		std::lock_guard<std::recursive_mutex> _(mutex_);
		for (auto i = callback_list_.begin(); i != callback_list_.end();)
		{
			if (*i == handler)
			{
				i = callback_list_.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	void callbacks::register_call_result(const uint64_t call, base* result)
	{
		std::lock_guard<std::recursive_mutex> _(mutex_);
		result_handlers_[call] = result;
	}

	void callbacks::unregister_call_result(const uint64_t call, base* /*result*/)
	{
		std::lock_guard<std::recursive_mutex> _(mutex_);
		const auto i = result_handlers_.find(call);
		if (i != result_handlers_.end())
		{
			result_handlers_.erase(i);
		}
	}

	void callbacks::return_call(void* data, const int size, const int type, const uint64_t call)
	{
		std::lock_guard<std::recursive_mutex> _(mutex_);

		result result{};
		result.call = call;
		result.data = data;
		result.size = size;
		result.type = type;

		calls_[call] = true;

		results_.emplace_back(result);
	}

	void callbacks::run_callbacks()
	{
		std::lock_guard<std::recursive_mutex> _(mutex_);

		for (const auto& result : results_)
		{
			if (result_handlers_.find(result.call) != result_handlers_.end())
			{
				result_handlers_[result.call]->run(result.data, false, result.call);
			}

			for (const auto& callback : callback_list_)
			{
				if (callback && callback->get_i_callback() == result.type)
				{
					callback->run(result.data, false, 0);
				}
			}

			if (result.data)
			{
				free(result.data);
			}
		}

		results_.clear();
	}

	extern "C" {

	bool SteamAPI_RestartAppIfNecessary()
	{
	  OutputDebugStringA(__FUNCTION__);
		return false;
	}

	bool SteamAPI_Init()
	{
	  OutputDebugStringA(__FUNCTION__);
		const std::filesystem::path steam_path = steam::SteamAPI_GetSteamInstallPath();
		if (steam_path.empty()) return true;

		::utils::nt::library::load(steam_path / "tier0_s64.dll");
		::utils::nt::library::load(steam_path / "vstdlib_s64.dll");
		::utils::nt::library::load(steam_path / "gameoverlayrenderer64.dll");
		::utils::nt::library::load(steam_path / "steamclient64.dll");
		return true;
	}

	void SteamAPI_RegisterCallResult(callbacks::base* result, const uint64_t call)
	{
	  OutputDebugStringA(__FUNCTION__);
		callbacks::register_call_result(call, result);
	}

	void SteamAPI_RegisterCallback(callbacks::base* handler, const int callback)
	{
	  OutputDebugStringA(__FUNCTION__);
		callbacks::register_callback(handler, callback);
	}

	void SteamAPI_RunCallbacks()
	{
	  OutputDebugStringA(__FUNCTION__);
		callbacks::run_callbacks();
	}

	void SteamAPI_Shutdown()
	{
	  OutputDebugStringA(__FUNCTION__);
	}

	void SteamAPI_UnregisterCallResult(callbacks::base* result, const uint64_t call)
	{
	  OutputDebugStringA(__FUNCTION__);
		callbacks::unregister_call_result(call, result);
	}

	void SteamAPI_UnregisterCallback(callbacks::base* handler)
	{
	  OutputDebugStringA(__FUNCTION__);
		callbacks::unregister_callback(handler);
	}

	const char* SteamAPI_GetSteamInstallPath()
	{
	  OutputDebugStringA(__FUNCTION__);
		static std::string install_path{};
		if (!install_path.empty())
		{
			return install_path.data();
		}

		HKEY reg_key;
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\Valve\\Steam", 0, KEY_QUERY_VALUE,
		                  &reg_key) ==
			ERROR_SUCCESS)
		{
			char path[MAX_PATH] = {0};
			DWORD length = sizeof(path);
			RegQueryValueExA(reg_key, "InstallPath", nullptr, nullptr, reinterpret_cast<BYTE*>(path),
			                 &length);
			RegCloseKey(reg_key);

			install_path = path;
		}

		return install_path.data();
	}
	  void* SteamGameServer_GetHSteamPipe()
	{
	    OutputDebugStringA(__FUNCTION__);
	    return (void*)1;
	}

void* SteamGameServer_GetHSteamUser()
	{
  OutputDebugStringA(__FUNCTION__);
	    return (void*)1;
	}

void* SteamAPI_GetHSteamUser()
{
  OutputDebugStringA(__FUNCTION__);
  return (void*)1;
}

void* SteamAPI_GetHSteamPipe()
{
  OutputDebugStringA(__FUNCTION__);
  return (void*)1;
}

void* SteamInternal_CreateInterface(const char* interfacename)
	{
  OutputDebugStringA(__FUNCTION__);
  OutputDebugStringA(interfacename);
	  if(std::string(interfacename) == "SteamClient017") {
	    static client c;
	    return &c;
	  }


	  MessageBoxA(0, interfacename, __FUNCTION__, 0);
	  return nullptr;//::utils::nt::library("steam_api64.dll").invoke<void*>("SteamInternal_CreateInterface", interfacename);
	}

bool SteamInternal_GameServer_Init()
	{
  OutputDebugStringA(__FUNCTION__);
  return true;
	}

	bool SteamGameServer_Init()
	{
	  OutputDebugStringA(__FUNCTION__);
		return true;
	}

	void SteamGameServer_RunCallbacks()
	{
	  OutputDebugStringA(__FUNCTION__);
	}

	void SteamGameServer_Shutdown()
	{
	  OutputDebugStringA(__FUNCTION__);
	}


	friends* SteamFriends()
	{
	  OutputDebugStringA(__FUNCTION__);
		static friends friends;
		return &friends;
	}

	matchmaking* SteamMatchmaking()
	{
	  OutputDebugStringA(__FUNCTION__);
		static matchmaking matchmaking;
		return &matchmaking;
	}

	game_server* SteamGameServer()
	{
	  OutputDebugStringA(__FUNCTION__);
		static game_server game_server;
		return &game_server;
	}

	networking* SteamNetworking()
	{
	  OutputDebugStringA(__FUNCTION__);
		static networking networking;
		return &networking;
	}

	remote_storage* SteamRemoteStorage()
	{
	  OutputDebugStringA(__FUNCTION__);
		static remote_storage remote_storage;
		return &remote_storage;
	}

	user* SteamUser()
	{
	  OutputDebugStringA(__FUNCTION__);
		static user user;
		return &user;
	}

	utils* SteamUtils()
	{
	  OutputDebugStringA(__FUNCTION__);
		static utils utils;
		return &utils;
	}

	apps* SteamApps()
	{
	  OutputDebugStringA(__FUNCTION__);
		static apps apps;
		return &apps;
	}

	user_stats* SteamUserStats()
	{
	  OutputDebugStringA(__FUNCTION__);
		static user_stats user_stats;
		return &user_stats;
	}

	}
}
