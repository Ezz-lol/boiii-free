#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	bool apps::BIsSubscribed()
	{
		return true;
	}

	bool apps::BIsLowViolence()
	{
		return false;
	}

	bool apps::BIsCybercafe()
	{
		return false;
	}

	bool apps::BIsVACBanned()
	{
		return false;
	}

	const char* apps::GetCurrentGameLanguage()
	{
		return "english";
	}

	const char* apps::GetAvailableGameLanguages()
	{
		return "english";
	}

	// This is used for checking DLC too.
	bool apps::BIsSubscribedApp(unsigned int appID)
	{
		return appID == 366842 ? std::filesystem::exists("zone/zm_common.xpak")	//zombies
			: appID == 366841 ? std::filesystem::exists("zone/mp_common.xpak")	//multiplayer
			: appID == 366840 ? std::filesystem::exists("zone/cp_common.xpak")	//campaign
			: true;
	}

	bool apps::BIsDlcInstalled(unsigned int appID)
	{
		return BIsSubscribedApp(appID);	//to not repeat the same code here
	}

	unsigned int apps::GetEarliestPurchaseUnixTime(unsigned int nAppID)
	{
		return 0;
	}

	bool apps::BIsSubscribedFromFreeWeekend()
	{
		return false;
	}

	int apps::GetDLCCount()
	{
		return 0;
	}

	bool apps::BGetDLCDataByIndex(int iDLC, unsigned int* pAppID, bool* pbAvailable, char* pchName,
	                              int cchNameBufferSize)
	{
		return false;
	}

	void apps::InstallDLC(unsigned int nAppID)
	{
	}

	void apps::UninstallDLC(unsigned int nAppID)
	{
	}

	void apps::RequestAppProofOfPurchaseKey(unsigned int nAppID)
	{
	}

	bool apps::GetCurrentBetaName(char* pchName, int cchNameBufferSize)
	{
		strncpy_s(pchName, cchNameBufferSize, "public", cchNameBufferSize);
		return true;
	}

	bool apps::MarkContentCorrupt(bool bMissingFilesOnly)
	{
		return false;
	}

	unsigned int apps::GetInstalledDepots(int* pvecDepots, unsigned int cMaxDepots)
	{
		return 0;
	}

	unsigned int apps::GetAppInstallDir(unsigned int appID, char* pchFolder, unsigned int cchFolderBufferSize)
	{
		return 0;
	}

	bool apps::BIsAppInstalled(unsigned int appID)
	{
		return false;
	}

	steam_id apps::GetAppOwner()
	{
		return SteamUser()->GetSteamID();
	}

	const char* apps::GetLaunchQueryParam(const char* pchKey)
	{
		return "";
	}

	bool apps::GetDlcDownloadProgress(uint32_t nAppID, uint64_t* punBytesDownloaded, uint64_t* punBytesTotal)
	{
		*punBytesTotal = 0;
		*punBytesDownloaded = 0;
		return false;
	}

	int apps::GetAppBuildId()
	{
		return 0;
	}

	void apps::RequestAllProofOfPurchaseKeys()
	{
	}

	uint64_t apps::GetFileDetails(const char* pszFileName)
	{
		return 0;
	}

	int apps::GetLaunchCommandLine(char* pszCommandLine, int cubCommandLine)
	{
		return 0;
	}

	bool apps::BIsSubscribedFromFamilySharing()
	{
		return false;
	}
}
