#include <std_include.hpp>
#include "../steam.hpp"
#include "../../component/dedicated.hpp"

namespace steam
{
	bool game_server::InitGameServer(unsigned int unGameIP, unsigned short unGamePort, unsigned short usQueryPort,
	                                 unsigned int unServerFlags, unsigned int nAppID, const char* pchVersion)
	{
		return true;
	}

	void game_server::SetProduct(const char* pchProductName)
	{
	}

	void game_server::SetGameDescription(const char* pchGameDescription)
	{
	}

	void game_server::SetModDir(const char* pchModDir)
	{
	}

	void game_server::SetDedicatedServer(bool bDedicatedServer)
	{
	}

	void game_server::LogOn(const char* pszAccountName, const char* pszPassword)
	{
	}

	void game_server::LogOnAnonymous()
	{
		auto* const retvals = calloc(1, 1);
		const auto result = callbacks::register_call();
		callbacks::return_call(retvals, 0, 101, result);
	}

	void game_server::LogOff()
	{
	}

	bool game_server::BLoggedOn()
	{
		return true;
	}

	bool game_server::BSecure()
	{
		return false;
	}

	steam_id game_server::GetSteamID()
	{
		return SteamUser()->GetSteamID();
	}

	bool game_server::WasRestartRequested()
	{
		return false;
	}

	void game_server::SetMaxPlayerCount(int cPlayersMax)
	{
	}

	void game_server::SetBotPlayerCount(int cBotPlayers)
	{
	}

	void game_server::SetServerName(const char* pszServerName)
	{
	}

	void game_server::SetMapName(const char* pszMapName)
	{
	}

	void game_server::SetPasswordProtected(bool bPasswordProtected)
	{
	}

	void game_server::SetSpectatorPort(unsigned short unSpectatorPort)
	{
	}

	void game_server::SetSpectatorServerName(const char* pszSpectatorServerName)
	{
	}

	void game_server::ClearAllKeyValues()
	{
	}

	void game_server::SetKeyValue(const char* pKey, const char* pValue)
	{
	}

	void game_server::SetGameTags(const char* pchGameTags)
	{
	}

	void game_server::SetGameData(const char* pchGameData)
	{
	}

	void game_server::SetRegion(const char* pchRegionName)
	{
	}

	int game_server::SendUserConnectAndAuthenticate(unsigned int unIPClient, const void* pvAuthBlob,
	                                                unsigned int cubAuthBlobSize, steam_id* pSteamIDUser)
	{
		return 0;
	}

	steam_id game_server::CreateUnauthenticatedUserConnection()
	{
		return SteamUser()->GetSteamID();
	}

	void game_server::SendUserDisconnect(steam_id steamIDUser)
	{
	}

	bool game_server::BUpdateUserData(steam_id steamIDUser, const char* pchPlayerName, unsigned int uScore)
	{
		return false;
	}

	int game_server::GetAuthSessionTicket(void* pTicket, int cbMaxTicket, unsigned int* pcbTicket)
	{
		return 0;
	}

	int game_server::BeginAuthSession(const void* pAuthTicket, int cbAuthTicket, steam_id steamID)
	{
		return 0;
	}

	void game_server::EndAuthSession(steam_id steamID)
	{
	}

	void game_server::CancelAuthTicket(int hAuthTicket)
	{
	}

	int game_server::UserHasLicenseForApp(steam_id steamID, unsigned int appID)
	{
		return 0;
	}

	bool game_server::RequestUserGroupStatus(steam_id steamIDUser, steam_id steamIDGroup)
	{
		return false;
	}

	void game_server::GetGameplayStats()
	{
	}

	unsigned long long game_server::GetServerReputation()
	{
		return 0;
	}

	unsigned int game_server::GetPublicIP()
	{
		return 0;
	}

	bool game_server::HandleIncomingPacket(const void* pData, int cbData, unsigned int srcIP, unsigned short srcPort)
	{
		return false;
	}

	int game_server::GetNextOutgoingPacket(void* pOut, int cbMaxOut, unsigned int* pNetAdr, unsigned short* pPort)
	{
		return 0;
	}

	void game_server::EnableHeartbeats(bool bActive)
	{
		dedicated::send_heartbeat();
	}

	void game_server::SetHeartbeatInterval(int iHeartbeatInterval)
	{
	}

	void game_server::ForceHeartbeat()
	{
	}

	unsigned long long game_server::AssociateWithClan(steam_id clanID)
	{
		return 0;
	}

	unsigned long long game_server::ComputeNewPlayerCompatibility(steam_id steamID)
	{
		return 0;
	}
}
