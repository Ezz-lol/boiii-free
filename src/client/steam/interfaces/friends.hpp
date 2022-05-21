#pragma once

namespace steam
{
	class friends
	{
	public:
		~friends() = default;

		virtual const char* GetPersonaName();
		virtual unsigned long long SetPersonaName(const char* pchPersonaName);
		virtual int GetPersonaState();
		virtual int GetFriendCount(int eFriendFlags);
		virtual steam_id GetFriendByIndex(int iFriend, int iFriendFlags);
		virtual int GetFriendRelationship(steam_id steamIDFriend);
		virtual int GetFriendPersonaState(steam_id steamIDFriend);
		virtual const char* GetFriendPersonaName(steam_id steamIDFriend);
		virtual bool GetFriendGamePlayed(steam_id steamIDFriend, void* pFriendGameInfo);
		virtual const char* GetFriendPersonaNameHistory(steam_id steamIDFriend, int iPersonaName);
		virtual bool HasFriend(steam_id steamIDFriend, int eFriendFlags);
		virtual int GetClanCount();
		virtual steam_id GetClanByIndex(int iClan);
		virtual const char* GetClanName(steam_id steamIDClan);
		virtual const char* GetClanTag(steam_id steamIDClan);
		virtual bool GetClanActivityCounts(steam_id steamID, int* pnOnline, int* pnInGame, int* pnChatting);
		virtual unsigned long long DownloadClanActivityCounts(steam_id groupIDs[], int nIds);
		virtual int GetFriendCountFromSource(steam_id steamIDSource);
		virtual steam_id GetFriendFromSourceByIndex(steam_id steamIDSource, int iFriend);
		virtual bool IsUserInSource(steam_id steamIDUser, steam_id steamIDSource);
		virtual void SetInGameVoiceSpeaking(steam_id steamIDUser, bool bSpeaking);
		virtual void ActivateGameOverlay(const char* pchDialog);
		virtual void ActivateGameOverlayToUser(const char* pchDialog, steam_id steamID);
		virtual void ActivateGameOverlayToWebPage(const char* pchURL);
		virtual void ActivateGameOverlayToStore(unsigned int nAppID, unsigned int eFlag);
		virtual void SetPlayedWith(steam_id steamIDUserPlayedWith);
		virtual void ActivateGameOverlayInviteDialog(steam_id steamIDLobby);
		virtual int GetSmallFriendAvatar(steam_id steamIDFriend);
		virtual int GetMediumFriendAvatar(steam_id steamIDFriend);
		virtual int GetLargeFriendAvatar(steam_id steamIDFriend);
		virtual bool RequestUserInformation(steam_id steamIDUser, bool bRequireNameOnly);
		virtual unsigned long long RequestClanOfficerList(steam_id steamIDClan);
		virtual steam_id GetClanOwner(steam_id steamIDClan);
		virtual int GetClanOfficerCount(steam_id steamIDClan);
		virtual steam_id GetClanOfficerByIndex(steam_id steamIDClan, int iOfficer);
		virtual int GetUserRestrictions();
		virtual bool SetRichPresence(const char* pchKey, const char* pchValue);
		virtual void ClearRichPresence();
		virtual const char* GetFriendRichPresence(steam_id steamIDFriend, const char* pchKey);
		virtual int GetFriendRichPresenceKeyCount(steam_id steamIDFriend);
		virtual const char* GetFriendRichPresenceKeyByIndex(steam_id steamIDFriend, int iKey);
		virtual void RequestFriendRichPresence(steam_id steamIDFriend);
		virtual bool InviteUserToGame(steam_id steamIDFriend, const char* pchConnectString);
		virtual int GetCoplayFriendCount();
		virtual steam_id GetCoplayFriend(int iCoplayFriend);
		virtual int GetFriendCoplayTime(steam_id steamIDFriend);
		virtual unsigned int GetFriendCoplayGame(steam_id steamIDFriend);
		virtual unsigned long long JoinClanChatRoom(steam_id steamIDClan);
		virtual bool LeaveClanChatRoom(steam_id steamIDClan);
		virtual int GetClanChatMemberCount(steam_id steamIDClan);
		virtual steam_id GetChatMemberByIndex(steam_id steamIDClan, int iUser);
		virtual bool SendClanChatMessage(steam_id steamIDClanChat, const char* pchText);
		virtual int GetClanChatMessage(steam_id steamIDClanChat, int iMessage, void* prgchText, int cchTextMax,
		                               unsigned int* peChatEntryType, steam_id* pSteamIDChatter);
		virtual bool IsClanChatAdmin(steam_id steamIDClanChat, steam_id steamIDUser);
		virtual bool IsClanChatWindowOpenInSteam(steam_id steamIDClanChat);
		virtual bool OpenClanChatWindowInSteam(steam_id steamIDClanChat);
		virtual bool CloseClanChatWindowInSteam(steam_id steamIDClanChat);
		virtual bool SetListenForFriendsMessages(bool bInterceptEnabled);
		virtual bool ReplyToFriendMessage(steam_id steamIDFriend, const char* pchMsgToSend);
		virtual int GetFriendMessage(steam_id steamIDFriend, int iMessageID, void* pvData, int cubData,
		                             unsigned int* peChatEntryType);
		virtual unsigned long long GetFollowerCount(steam_id steamID);
		virtual unsigned long long IsFollowing(steam_id steamID);
		virtual unsigned long long EnumerateFollowingList(unsigned int unStartIndex);
	};
}
