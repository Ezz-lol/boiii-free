#pragma once

namespace steam
{
	class user_stats
	{
	public:
		~user_stats() = default;

		virtual bool RequestCurrentStats();
		virtual bool GetStat(const char* pchName, int* pData);
		virtual bool GetStat(const char* pchName, float* pData);
		virtual bool SetStat(const char* pchName, int nData);
		virtual bool SetStat(const char* pchName, float fData);
		virtual bool UpdateAvgRateStat(const char* pchName, float flCountThisSession, double dSessionLength);
		virtual bool GetAchievement(const char* pchName, bool* pbAchieved);
		virtual bool SetAchievement(const char* pchName);
		virtual bool ClearAchievement(const char* pchName);
		virtual bool GetAchievementAndUnlockTime(const char* pchName, bool* pbAchieved, unsigned int* punUnlockTime);
		virtual bool StoreStats();
		virtual int GetAchievementIcon(const char* pchName);
		virtual const char* GetAchievementDisplayAttribute(const char* pchName, const char* pchKey);
		virtual bool IndicateAchievementProgress(const char* pchName, unsigned int nCurProgress,
		                                         unsigned int nMaxProgress);
		virtual unsigned int GetNumAchievements();
		virtual const char* GetAchievementName(unsigned int iAchievement);
		virtual unsigned long long RequestUserStats(steam_id steamIDUser);
		virtual bool GetUserStat(steam_id steamIDUser, const char* pchName, int* pData);
		virtual bool GetUserStat(steam_id steamIDUser, const char* pchName, float* pData);
		virtual bool GetUserAchievement(steam_id steamIDUser, const char* pchName, bool* pbAchieved);
		virtual bool GetUserAchievementAndUnlockTime(steam_id steamIDUser, const char* pchName, bool* pbAchieved,
		                                             unsigned int* punUnlockTime);
		virtual bool ResetAllStats(bool bAchievementsToo);
		virtual unsigned long long FindOrCreateLeaderboard(const char* pchLeaderboardName, int eLeaderboardSortMethod,
		                                                   int eLeaderboardDisplayType);
		virtual unsigned long long FindLeaderboard(const char* pchLeaderboardName);
		virtual const char* GetLeaderboardName(unsigned long long hSteamLeaderboard);
		virtual int GetLeaderboardEntryCount(unsigned long long hSteamLeaderboard);
		virtual int GetLeaderboardSortMethod(unsigned long long hSteamLeaderboard);
		virtual int GetLeaderboardDisplayType(unsigned long long hSteamLeaderboard);
		virtual unsigned long long DownloadLeaderboardEntries(unsigned long long hSteamLeaderboard,
		                                                      int eLeaderboardDataRequest, int nRangeStart,
		                                                      int nRangeEnd);
		virtual unsigned long long DownloadLeaderboardEntriesForUsers(unsigned long long hSteamLeaderboard,
		                                                              steam_id* prgUsers, int cUsers);
		virtual bool GetDownloadedLeaderboardEntry(unsigned long long hSteamLeaderboardEntries, int index,
		                                           int* pLeaderboardEntry, int* pDetails, int cDetailsMax);
		virtual unsigned long long UploadLeaderboardScore(unsigned long long hSteamLeaderboard,
		                                                  int eLeaderboardUploadScoreMethod, int nScore,
		                                                  const int* pScoreDetails, int cScoreDetailsCount);
		virtual unsigned long long AttachLeaderboardUGC(unsigned long long hSteamLeaderboard, unsigned long long hUGC);
		virtual unsigned long long GetNumberOfCurrentPlayers();
		virtual unsigned long long RequestGlobalAchievementPercentages();
		virtual int GetMostAchievedAchievementInfo(char* pchName, unsigned int unNameBufLen, float* pflPercent,
		                                           bool* pbAchieved);
		virtual int GetNextMostAchievedAchievementInfo(int iIteratorPrevious, char* pchName, unsigned int unNameBufLen,
		                                               float* pflPercent, bool* pbAchieved);
		virtual bool GetAchievementAchievedPercent(const char* pchName, float* pflPercent);
		virtual unsigned long long RequestGlobalStats(int nHistoryDays);
		virtual bool GetGlobalStat(const char* pchStatName, long long* pData);
		virtual bool GetGlobalStat(const char* pchStatName, double* pData);
		virtual int GetGlobalStatHistory(const char* pchStatName, long long* pData, unsigned int cubData);
		virtual int GetGlobalStatHistory(const char* pchStatName, double* pData, unsigned int cubData);
	};
}
