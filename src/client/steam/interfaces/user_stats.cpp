#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	namespace
	{
		struct user_stats_received
		{
			enum { callback_id = 1101 };

			uint64_t m_n_game_id;
			int m_e_result;
			steam_id m_steam_id_user;
		};
	}

	bool user_stats::RequestCurrentStats()
	{

		static uint32_t ticket = 0;

		game_id game{};
		game.raw.mod_id = 0;
		game.raw.type = 0;
		game.raw.app_id = SteamUtils()->GetAppID();

		const auto result = callbacks::register_call();
		auto* response = static_cast<user_stats_received*>(calloc(
			1, sizeof(user_stats_received)));
		response->m_steam_id_user = SteamUser()->GetSteamID();
		response->m_e_result = 1; // k_EResultOK;
		response->m_n_game_id = game.bits;

		callbacks::return_call(response, sizeof(user_stats_received),
		                       user_stats_received::callback_id, result);
		return true;
	}

	bool user_stats::GetStat(const char* pchName, int* pData)
	{
		return false;
	}

	bool user_stats::GetStat(const char* pchName, float* pData)
	{
		return false;
	}

	bool user_stats::SetStat(const char* pchName, int nData)
	{
		return true;
	}

	bool user_stats::SetStat(const char* pchName, float fData)
	{
		return true;
	}

	bool user_stats::UpdateAvgRateStat(const char* pchName, float flCountThisSession, double dSessionLength)
	{
		return false;
	}

	bool user_stats::GetAchievement(const char* pchName, bool* pbAchieved)
	{
		return true;
	}

	bool user_stats::SetAchievement(const char* pchName)
	{
		return true;
	}

	bool user_stats::ClearAchievement(const char* pchName)
	{
		return true;
	}

	bool user_stats::GetAchievementAndUnlockTime(const char* pchName, bool* pbAchieved, unsigned int* punUnlockTime)
	{
		return true;
	}

	bool user_stats::StoreStats()
	{
		return true;
	}

	int user_stats::GetAchievementIcon(const char* pchName)
	{
		return 0;
	}

	const char* user_stats::GetAchievementDisplayAttribute(const char* pchName, const char* pchKey)
	{
		return "";
	}

	bool user_stats::IndicateAchievementProgress(const char* pchName, unsigned int nCurProgress,
	                                             unsigned int nMaxProgress)
	{
		return true;
	}

	unsigned int user_stats::GetNumAchievements()
	{
		return 0;
	}

	const char* user_stats::GetAchievementName(unsigned int iAchievement)
	{
		return "";
	}

	unsigned long long user_stats::RequestUserStats(steam_id steamIDUser)
	{
		return 0;
	}

	bool user_stats::GetUserStat(steam_id steamIDUser, const char* pchName, int* pData)
	{
		return false;
	}

	bool user_stats::GetUserStat(steam_id steamIDUser, const char* pchName, float* pData)
	{
		return false;
	}

	bool user_stats::GetUserAchievement(steam_id steamIDUser, const char* pchName, bool* pbAchieved)
	{
		return true;
	}

	bool user_stats::GetUserAchievementAndUnlockTime(steam_id steamIDUser, const char* pchName, bool* pbAchieved,
	                                                 unsigned int* punUnlockTime)
	{
		return true;
	}

	bool user_stats::ResetAllStats(bool bAchievementsToo)
	{
		return false;
	}

	unsigned long long user_stats::FindOrCreateLeaderboard(const char* pchLeaderboardName, int eLeaderboardSortMethod,
	                                                       int eLeaderboardDisplayType)
	{
		return 0;
	}

	unsigned long long user_stats::FindLeaderboard(const char* pchLeaderboardName)
	{
		return 0;
	}

	const char* user_stats::GetLeaderboardName(unsigned long long hSteamLeaderboard)
	{
		return "";
	}

	int user_stats::GetLeaderboardEntryCount(unsigned long long hSteamLeaderboard)
	{
		return 0;
	}

	int user_stats::GetLeaderboardSortMethod(unsigned long long hSteamLeaderboard)
	{
		return 0;
	}

	int user_stats::GetLeaderboardDisplayType(unsigned long long hSteamLeaderboard)
	{
		return 0;
	}

	unsigned long long user_stats::DownloadLeaderboardEntries(unsigned long long hSteamLeaderboard,
	                                                          int eLeaderboardDataRequest, int nRangeStart,
	                                                          int nRangeEnd)
	{
		return 0;
	}

	unsigned long long user_stats::DownloadLeaderboardEntriesForUsers(unsigned long long hSteamLeaderboard,
	                                                                  steam_id* prgUsers, int cUsers)
	{
		return 0;
	}

	bool user_stats::GetDownloadedLeaderboardEntry(unsigned long long hSteamLeaderboardEntries, int index,
	                                               int* pLeaderboardEntry, int* pDetails, int cDetailsMax)
	{
		return false;
	}

	unsigned long long user_stats::UploadLeaderboardScore(unsigned long long hSteamLeaderboard,
	                                                      int eLeaderboardUploadScoreMethod, int nScore,
	                                                      const int* pScoreDetails, int cScoreDetailsCount)
	{
		return 0;
	}

	unsigned long long user_stats::AttachLeaderboardUGC(unsigned long long hSteamLeaderboard, unsigned long long hUGC)
	{
		return 0;
	}

	unsigned long long user_stats::GetNumberOfCurrentPlayers()
	{
		return 0;
	}

	unsigned long long user_stats::RequestGlobalAchievementPercentages()
	{
		return 0;
	}

	int user_stats::GetMostAchievedAchievementInfo(char* pchName, unsigned int unNameBufLen, float* pflPercent,
	                                               bool* pbAchieved)
	{
		return 0;
	}

	int user_stats::GetNextMostAchievedAchievementInfo(int iIteratorPrevious, char* pchName, unsigned int unNameBufLen,
	                                                   float* pflPercent, bool* pbAchieved)
	{
		return 0;
	}

	bool user_stats::GetAchievementAchievedPercent(const char* pchName, float* pflPercent)
	{
		return true;
	}

	unsigned long long user_stats::RequestGlobalStats(int nHistoryDays)
	{
		return 0;
	}

	bool user_stats::GetGlobalStat(const char* pchStatName, long long* pData)
	{
		return false;
	}

	bool user_stats::GetGlobalStat(const char* pchStatName, double* pData)
	{
		return false;
	}

	int user_stats::GetGlobalStatHistory(const char* pchStatName, long long* pData, unsigned int cubData)
	{
		return 0;
	}

	int user_stats::GetGlobalStatHistory(const char* pchStatName, double* pData, unsigned int cubData)
	{
		return 0;
	}
}
