#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	uint64_t game_server_stats::RequestUserStats(steam_id steamIDUser)
	{
		return 0;
	}

	bool game_server_stats::GetUserStat(steam_id steamIDUser, const char* pchName, int32_t* pData)
	{
		return false;
	}

	bool game_server_stats::GetUserStat(steam_id steamIDUser, const char* pchName, float* pData)
	{
		return false;
	}

	bool game_server_stats::GetUserAchievement(steam_id steamIDUser, const char* pchName, bool* pbAchieved)
	{
		return false;
	}

	bool game_server_stats::SetUserStat(steam_id steamIDUser, const char* pchName, int32_t nData)
	{
		return false;
	}

	bool game_server_stats::SetUserStat(steam_id steamIDUser, const char* pchName, float fData)
	{
		return false;
	}

	bool game_server_stats::UpdateUserAvgRateStat(steam_id steamIDUser, const char* pchName, float flCountThisSession,
	                                              double dSessionLength)
	{
		return false;
	}

	bool game_server_stats::SetUserAchievement(steam_id steamIDUser, const char* pchName)
	{
		return false;
	}

	bool game_server_stats::ClearUserAchievement(steam_id steamIDUser, const char* pchName)
	{
		return false;
	}

	uint64_t game_server_stats::StoreUserStats(steam_id steamIDUser)
	{
		return 0;
	}
}
