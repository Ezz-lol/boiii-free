#pragma once

namespace steam
{
	class game_server
	{
	public:
		~game_server() = default;

		virtual bool InitGameServer(unsigned int unGameIP, unsigned short unGamePort, unsigned short usQueryPort,
		                            unsigned int unServerFlags, unsigned int nAppID, const char* pchVersion);
		virtual void SetProduct(const char* pchProductName);
		virtual void SetGameDescription(const char* pchGameDescription);
		virtual void SetModDir(const char* pchModDir);
		virtual void SetDedicatedServer(bool bDedicatedServer);
		virtual void LogOn(const char* pszAccountName, const char* pszPassword);
		virtual void LogOnAnonymous();
		virtual void LogOff();
		virtual bool BLoggedOn();
		virtual bool BSecure();
		virtual steam_id GetSteamID();
		virtual bool WasRestartRequested();
		virtual void SetMaxPlayerCount(int cPlayersMax);
		virtual void SetBotPlayerCount(int cBotPlayers);
		virtual void SetServerName(const char* pszServerName);
		virtual void SetMapName(const char* pszMapName);
		virtual void SetPasswordProtected(bool bPasswordProtected);
		virtual void SetSpectatorPort(unsigned short unSpectatorPort);
		virtual void SetSpectatorServerName(const char* pszSpectatorServerName);
		virtual void ClearAllKeyValues();
		virtual void SetKeyValue(const char* pKey, const char* pValue);
		virtual void SetGameTags(const char* pchGameTags);
		virtual void SetGameData(const char* pchGameData);
		virtual void SetRegion(const char* pchRegionName);
		virtual int SendUserConnectAndAuthenticate(unsigned int unIPClient, const void* pvAuthBlob,
		                                           unsigned int cubAuthBlobSize, steam_id* pSteamIDUser);
		virtual steam_id CreateUnauthenticatedUserConnection();
		virtual void SendUserDisconnect(steam_id steamIDUser);
		virtual bool BUpdateUserData(steam_id steamIDUser, const char* pchPlayerName, unsigned int uScore);
		virtual int GetAuthSessionTicket(void* pTicket, int cbMaxTicket, unsigned int* pcbTicket);
		virtual int BeginAuthSession(const void* pAuthTicket, int cbAuthTicket, steam_id steamID);
		virtual void EndAuthSession(steam_id steamID);
		virtual void CancelAuthTicket(int hAuthTicket);
		virtual int UserHasLicenseForApp(steam_id steamID, unsigned int appID);
		virtual bool RequestUserGroupStatus(steam_id steamIDUser, steam_id steamIDGroup);
		virtual void GetGameplayStats();
		virtual unsigned long long GetServerReputation();
		virtual unsigned int GetPublicIP();
		virtual bool HandleIncomingPacket(const void* pData, int cbData, unsigned int srcIP, unsigned short srcPort);
		virtual int GetNextOutgoingPacket(void* pOut, int cbMaxOut, unsigned int* pNetAdr, unsigned short* pPort);
		virtual void EnableHeartbeats(bool bActive);
		virtual void SetHeartbeatInterval(int iHeartbeatInterval);
		virtual void ForceHeartbeat();
		virtual unsigned long long AssociateWithClan(steam_id clanID);
		virtual unsigned long long ComputeNewPlayerCompatibility(steam_id steamID);
	};
}
