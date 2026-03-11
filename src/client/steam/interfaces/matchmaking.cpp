#include <std_include.hpp>
#include "../steam.hpp"

#include "component/network.hpp"
#include "component/server_list.hpp"
#include "game/game.hpp"

namespace steam
{
	namespace
	{
		constexpr unsigned int k_unFavoriteFlag = 1u;
		constexpr unsigned int k_unHistoryFlag = 2u;
		constexpr unsigned int BO3_APP_ID = 311210;

		// combined list of favorites + recents for GetFavoriteGame enumeration
		struct favorite_entry
		{
			game::netadr_t addr;
			unsigned int flags;
		};

		std::vector<favorite_entry> build_favorites_list()
		{
			std::vector<favorite_entry> result;

			server_list::get_favorite_servers().access([&](const server_list::server_list& favs)
			{
				for (const auto& addr : favs)
				{
					result.push_back({addr, k_unFavoriteFlag});
				}
			});

			server_list::get_recent_servers().access([&](const server_list::recent_list& recents)
			{
				for (const auto& addr : recents)
				{
					result.push_back({addr, k_unHistoryFlag});
				}
			});

			return result;
		}
	}

	int matchmaking::GetFavoriteGameCount()
	{
		auto list = build_favorites_list();
		return static_cast<int>(list.size());
	}

	bool matchmaking::GetFavoriteGame(int iGame, unsigned int* pnAppID, unsigned int* pnIP, unsigned short* pnConnPort,
	                                  unsigned short* pnQueryPort, unsigned int* punFlags,
	                                  unsigned int* pRTime32LastPlayedOnServer)
	{
		auto list = build_favorites_list();
		if (iGame < 0 || iGame >= static_cast<int>(list.size()))
			return false;

		const auto& entry = list[iGame];
		if (pnAppID) *pnAppID = BO3_APP_ID;

		// pack IP as network-order uint32
		if (pnIP)
		{
			uint32_t ip = 0;
			memcpy(&ip, &entry.addr.ipv4.a, 4);
			*pnIP = ntohl(ip);
		}

		if (pnConnPort) *pnConnPort = entry.addr.port;
		if (pnQueryPort) *pnQueryPort = entry.addr.port;
		if (punFlags) *punFlags = entry.flags;
		if (pRTime32LastPlayedOnServer) *pRTime32LastPlayedOnServer = static_cast<unsigned int>(std::time(nullptr));

		return true;
	}

	int matchmaking::AddFavoriteGame(unsigned int nAppID, unsigned int nIP, unsigned short nConnPort,
	                                 unsigned short nQueryPort, unsigned int unFlags,
	                                 unsigned int rTime32LastPlayedOnServer)
	{
		auto addr = network::address_from_ip(htonl(nIP), nConnPort);
		if ((unFlags & k_unFavoriteFlag) != 0u)
		{
			server_list::add_favorite_server(addr);
		}
		else if ((unFlags & k_unHistoryFlag) != 0u || unFlags == 0u)
		{
			server_list::add_recent_server(addr);
		}
		return 0;
	}

	bool matchmaking::RemoveFavoriteGame(unsigned int nAppID, unsigned int nIP, unsigned short nConnPort,
	                                     unsigned short nQueryPort, unsigned int unFlags)
	{
		auto addr = network::address_from_ip(htonl(nIP), nConnPort);
		if ((unFlags & k_unFavoriteFlag) != 0u)
		{
			server_list::remove_favorite_server(addr);
		}
		else if ((unFlags & k_unHistoryFlag) != 0u || unFlags == 0u)
		{
			server_list::remove_recent_server(addr);
		}
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
