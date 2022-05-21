#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	int matchmaking::GetFavoriteGameCount()
	{
		return 0;
	}

	bool matchmaking::GetFavoriteGame(int iGame, unsigned int* pnAppID, unsigned int* pnIP, unsigned short* pnConnPort,
	                                  unsigned short* pnQueryPort, unsigned int* punFlags,
	                                  unsigned int* pRTime32LastPlayedOnServer)
	{
		return false;
	}

	int matchmaking::AddFavoriteGame(unsigned int nAppID, unsigned int nIP, unsigned short nConnPort,
	                                 unsigned short nQueryPort, unsigned int unFlags,
	                                 unsigned int rTime32LastPlayedOnServer)
	{
		return 0;
	}

	bool matchmaking::RemoveFavoriteGame(unsigned int nAppID, unsigned int nIP, unsigned short nConnPort,
	                                     unsigned short nQueryPort, unsigned int unFlags)
	{
		return false;
	}

	unsigned long long matchmaking::RequestLobbyList()
	{
		return 0;
	}

	void matchmaking::AddRequestLobbyListStringFilter(const char* pchKeyToMatch, const char* pchValueToMatch,
	                                                  int eComparisonType)
	{
	}

	void matchmaking::AddRequestLobbyListNumericalFilter(const char* pchKeyToMatch, int nValueToMatch,
	                                                     int eComparisonType)
	{
	}

	void matchmaking::AddRequestLobbyListNearValueFilter(const char* pchKeyToMatch, int nValueToBeCloseTo)
	{
	}

	void matchmaking::AddRequestLobbyListFilterSlotsAvailable(int nSlotsAvailable)
	{
	}

	void matchmaking::AddRequestLobbyListDistanceFilter(int eLobbyDistanceFilter)
	{
	}

	void matchmaking::AddRequestLobbyListResultCountFilter(int cMaxResults)
	{
	}

	void matchmaking::AddRequestLobbyListCompatibleMembersFilter(steam_id steamID)
	{
	}

	steam_id matchmaking::GetLobbyByIndex(int iLobby)
	{
		steam_id id;

		id.raw.account_id = SteamUser()->GetSteamID().raw.account_id;
		id.raw.universe = 1;
		id.raw.account_type = 8;
		id.raw.account_instance = 0x40000;

		return id;
	}

	unsigned long long matchmaking::CreateLobby(int eLobbyType, int cMaxMembers)
	{
		const auto result = callbacks::register_call();
		auto retvals = static_cast<lobby_created*>(calloc(1, sizeof(lobby_created)));
		//::Utils::Memory::AllocateArray<LobbyCreated>();
		steam_id id;

		id.raw.account_id = SteamUser()->GetSteamID().raw.account_id;
		id.raw.universe = 1;
		id.raw.account_type = 8;
		id.raw.account_instance = 0x40000;

		retvals->m_e_result = 1;
		retvals->m_ul_steam_id_lobby = id;

		callbacks::return_call(retvals, sizeof(lobby_created), lobby_created::callback_id, result);

		matchmaking::JoinLobby(id);

		return result;
	}

	unsigned long long matchmaking::JoinLobby(steam_id steamIDLobby)
	{
		const auto result = callbacks::register_call();
		auto* retvals = static_cast<lobby_enter*>(calloc(1, sizeof(lobby_enter)));
		//::Utils::Memory::AllocateArray<LobbyEnter>();
		retvals->m_b_locked = false;
		retvals->m_e_chat_room_enter_response = 1;
		retvals->m_rgf_chat_permissions = 0xFFFFFFFF;
		retvals->m_ul_steam_id_lobby = steamIDLobby;

		callbacks::return_call(retvals, sizeof(lobby_enter), lobby_enter::callback_id, result);

		return result;
	}

	void matchmaking::LeaveLobby(steam_id steamIDLobby)
	{
	}

	bool matchmaking::InviteUserToLobby(steam_id steamIDLobby, steam_id steamIDInvitee)
	{
		return true;
	}

	int matchmaking::GetNumLobbyMembers(steam_id steamIDLobby)
	{
		return 1;
	}

	steam_id matchmaking::GetLobbyMemberByIndex(steam_id steamIDLobby, int iMember)
	{
		return SteamUser()->GetSteamID();
	}

	const char* matchmaking::GetLobbyData(steam_id steamIDLobby, const char* pchKey)
	{
		return "";
	}

	bool matchmaking::SetLobbyData(steam_id steamIDLobby, const char* pchKey, const char* pchValue)
	{
		return true;
	}

	int matchmaking::GetLobbyDataCount(steam_id steamIDLobby)
	{
		return 0;
	}

	bool matchmaking::GetLobbyDataByIndex(steam_id steamIDLobby, int iLobbyData, char* pchKey, int cchKeyBufferSize,
	                                      char* pchValue, int cchValueBufferSize)
	{
		return true;
	}

	bool matchmaking::DeleteLobbyData(steam_id steamIDLobby, const char* pchKey)
	{
		return true;
	}

	const char* matchmaking::GetLobbyMemberData(steam_id steamIDLobby, steam_id steamIDUser, const char* pchKey)
	{
		return "";
	}

	void matchmaking::SetLobbyMemberData(steam_id steamIDLobby, const char* pchKey, const char* pchValue)
	{
	}

	bool matchmaking::SendLobbyChatMsg(steam_id steamIDLobby, const void* pvMsgBody, int cubMsgBody)
	{
		return true;
	}

	int matchmaking::GetLobbyChatEntry(steam_id steamIDLobby, int iChatID, steam_id* pSteamIDUser, void* pvData,
	                                   int cubData, int* peChatEntryType)
	{
		return 0;
	}

	bool matchmaking::RequestLobbyData(steam_id steamIDLobby)
	{
		return true;
	}

	void matchmaking::SetLobbyGameServer(steam_id steamIDLobby, unsigned int unGameServerIP,
	                                     unsigned short unGameServerPort, steam_id steamIDGameServer)
	{
	}

	bool matchmaking::GetLobbyGameServer(steam_id steamIDLobby, unsigned int* punGameServerIP,
	                                     unsigned short* punGameServerPort, steam_id* psteamIDGameServer)
	{
		return true;
	}

	bool matchmaking::SetLobbyMemberLimit(steam_id steamIDLobby, int cMaxMembers)
	{
		return true;
	}

	int matchmaking::GetLobbyMemberLimit(steam_id steamIDLobby)
	{
		return 0;
	}

	bool matchmaking::SetLobbyType(steam_id steamIDLobby, int eLobbyType)
	{
		return true;
	}

	bool matchmaking::SetLobbyJoinable(steam_id steamIDLobby, bool bLobbyJoinable)
	{
		return true;
	}

	steam_id matchmaking::GetLobbyOwner(steam_id steamIDLobby)
	{
		return SteamUser()->GetSteamID();
	}

	bool matchmaking::SetLobbyOwner(steam_id steamIDLobby, steam_id steamIDNewOwner)
	{
		return true;
	}

	bool matchmaking::SetLinkedLobby(steam_id steamIDLobby, steam_id steamIDLobby2)
	{
		return true;
	}
}
