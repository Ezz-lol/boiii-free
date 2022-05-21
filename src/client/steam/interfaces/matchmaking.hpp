#pragma once

namespace steam
{
	struct lobby_created final
	{
		enum { callback_id = 513 };

		int m_e_result;
		int m_pad;
		steam_id m_ul_steam_id_lobby;
	};

	struct lobby_enter final
	{
		enum { callback_id = 504 };

		steam_id m_ul_steam_id_lobby;
		int m_rgf_chat_permissions;
		bool m_b_locked;
		int m_e_chat_room_enter_response;
	};

	class matchmaking
	{
	public:
		~matchmaking() = default;

		virtual int GetFavoriteGameCount();
		virtual bool GetFavoriteGame(int iGame, unsigned int* pnAppID, unsigned int* pnIP, unsigned short* pnConnPort,
		                             unsigned short* pnQueryPort, unsigned int* punFlags,
		                             unsigned int* pRTime32LastPlayedOnServer);
		virtual int AddFavoriteGame(unsigned int nAppID, unsigned int nIP, unsigned short nConnPort,
		                            unsigned short nQueryPort, unsigned int unFlags,
		                            unsigned int rTime32LastPlayedOnServer);
		virtual bool RemoveFavoriteGame(unsigned int nAppID, unsigned int nIP, unsigned short nConnPort,
		                                unsigned short nQueryPort, unsigned int unFlags);
		virtual unsigned long long RequestLobbyList();
		virtual void AddRequestLobbyListStringFilter(const char* pchKeyToMatch, const char* pchValueToMatch,
		                                             int eComparisonType);
		virtual void AddRequestLobbyListNumericalFilter(const char* pchKeyToMatch, int nValueToMatch,
		                                                int eComparisonType);
		virtual void AddRequestLobbyListNearValueFilter(const char* pchKeyToMatch, int nValueToBeCloseTo);
		virtual void AddRequestLobbyListFilterSlotsAvailable(int nSlotsAvailable);
		virtual void AddRequestLobbyListDistanceFilter(int eLobbyDistanceFilter);
		virtual void AddRequestLobbyListResultCountFilter(int cMaxResults);
		virtual void AddRequestLobbyListCompatibleMembersFilter(steam_id steamID);
		virtual steam_id GetLobbyByIndex(int iLobby);
		virtual unsigned long long CreateLobby(int eLobbyType, int cMaxMembers);
		virtual unsigned long long JoinLobby(steam_id steamIDLobby);
		virtual void LeaveLobby(steam_id steamIDLobby);
		virtual bool InviteUserToLobby(steam_id steamIDLobby, steam_id steamIDInvitee);
		virtual int GetNumLobbyMembers(steam_id steamIDLobby);
		virtual steam_id GetLobbyMemberByIndex(steam_id steamIDLobby, int iMember);
		virtual const char* GetLobbyData(steam_id steamIDLobby, const char* pchKey);
		virtual bool SetLobbyData(steam_id steamIDLobby, const char* pchKey, const char* pchValue);
		virtual int GetLobbyDataCount(steam_id steamIDLobby);
		virtual bool GetLobbyDataByIndex(steam_id steamIDLobby, int iLobbyData, char* pchKey, int cchKeyBufferSize,
		                                 char* pchValue, int cchValueBufferSize);
		virtual bool DeleteLobbyData(steam_id steamIDLobby, const char* pchKey);
		virtual const char* GetLobbyMemberData(steam_id steamIDLobby, steam_id steamIDUser, const char* pchKey);
		virtual void SetLobbyMemberData(steam_id steamIDLobby, const char* pchKey, const char* pchValue);
		virtual bool SendLobbyChatMsg(steam_id steamIDLobby, const void* pvMsgBody, int cubMsgBody);
		virtual int GetLobbyChatEntry(steam_id steamIDLobby, int iChatID, steam_id* pSteamIDUser, void* pvData,
		                              int cubData, int* peChatEntryType);
		virtual bool RequestLobbyData(steam_id steamIDLobby);
		virtual void SetLobbyGameServer(steam_id steamIDLobby, unsigned int unGameServerIP,
		                                unsigned short unGameServerPort, steam_id steamIDGameServer);
		virtual bool GetLobbyGameServer(steam_id steamIDLobby, unsigned int* punGameServerIP,
		                                unsigned short* punGameServerPort, steam_id* psteamIDGameServer);
		virtual bool SetLobbyMemberLimit(steam_id steamIDLobby, int cMaxMembers);
		virtual int GetLobbyMemberLimit(steam_id steamIDLobby);
		virtual bool SetLobbyType(steam_id steamIDLobby, int eLobbyType);
		virtual bool SetLobbyJoinable(steam_id steamIDLobby, bool bLobbyJoinable);
		virtual steam_id GetLobbyOwner(steam_id steamIDLobby);
		virtual bool SetLobbyOwner(steam_id steamIDLobby, steam_id steamIDNewOwner);
		virtual bool SetLinkedLobby(steam_id steamIDLobby, steam_id steamIDLobby2);
	};
}
