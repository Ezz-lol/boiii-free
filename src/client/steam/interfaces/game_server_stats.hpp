#pragma once

namespace steam
{
	class game_server_stats
	{
	public:
		~game_server_stats() = default;

		virtual uint64_t RequestUserStats(steam_id steamIDUser);
		virtual bool GetUserStat(steam_id steamIDUser, const char* pchName, int32_t* pData);
		virtual bool GetUserStat(steam_id steamIDUser, const char* pchName, float* pData);
		virtual bool GetUserAchievement(steam_id steamIDUser, const char* pchName, bool* pbAchieved);
		virtual bool SetUserStat(steam_id steamIDUser, const char* pchName, int32_t nData);
		virtual bool SetUserStat(steam_id steamIDUser, const char* pchName, float fData);
		virtual bool UpdateUserAvgRateStat(steam_id steamIDUser, const char* pchName, float flCountThisSession,
		                                   double dSessionLength);
		virtual bool SetUserAchievement(steam_id steamIDUser, const char* pchName);
		virtual bool ClearUserAchievement(steam_id steamIDUser, const char* pchName);
		virtual uint64_t StoreUserStats(steam_id steamIDUser);
	};
}
