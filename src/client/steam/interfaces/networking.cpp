#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	bool networking::SendP2PPacket(steam_id steamIDRemote, const void* pubData, unsigned int cubData, int eP2PSendType)
	{
		return false;
	}

	bool networking::IsP2PPacketAvailable(unsigned int* pcubMsgSize, int idk)
	{
		return false;
	}

	bool networking::ReadP2PPacket(void* pubDest, unsigned int cubDest, unsigned int* pcubMsgSize,
	                               steam_id* psteamIDRemote)
	{
		return false;
	}

	bool networking::AcceptP2PSessionWithUser(steam_id steamIDRemote)
	{
		return false;
	}

	bool networking::CloseP2PSessionWithUser(steam_id steamIDRemote)
	{
		return false;
	}

	bool networking::CloseP2PChannelWithUser(steam_id steamIDRemote, int iVirtualPort)
	{
		return false;
	}

	bool networking::GetP2PSessionState(steam_id steamIDRemote, void* pConnectionState)
	{
		return false;
	}

	bool networking::AllowP2PPacketRelay(bool bAllow)
	{
		return false;
	}

	unsigned int networking::CreateListenSocket(int nVirtualP2PPort, unsigned int nIP, unsigned short nPort,
	                                            bool bAllowUseOfPacketRelay)
	{
		return NULL;
	}

	unsigned int networking::CreateP2PConnectionSocket(steam_id steamIDTarget, int nVirtualPort, int nTimeoutSec,
	                                                   bool bAllowUseOfPacketRelay)
	{
		return NULL;
	}

	unsigned int networking::CreateConnectionSocket(unsigned int nIP, unsigned short nPort, int nTimeoutSec)
	{
		return NULL;
	}

	bool networking::DestroySocket(unsigned int hSocket, bool bNotifyRemoteEnd)
	{
		return false;
	}

	bool networking::DestroyListenSocket(unsigned int hSocket, bool bNotifyRemoteEnd)
	{
		return false;
	}

	bool networking::SendDataOnSocket(unsigned int hSocket, void* pubData, unsigned int cubData, bool bReliable)
	{
		return false;
	}

	bool networking::IsDataAvailableOnSocket(unsigned int hSocket, unsigned int* pcubMsgSize)
	{
		return false;
	}

	bool networking::RetrieveDataFromSocket(unsigned int hSocket, void* pubDest, unsigned int cubDest,
	                                        unsigned int* pcubMsgSize)
	{
		return false;
	}

	bool networking::IsDataAvailable(unsigned int hListenSocket, unsigned int* pcubMsgSize, unsigned int* phSocket)
	{
		return false;
	}

	bool networking::RetrieveData(unsigned int hListenSocket, void* pubDest, unsigned int cubDest,
	                              unsigned int* pcubMsgSize, unsigned int* phSocket)
	{
		return false;
	}

	bool networking::GetSocketInfo(unsigned int hSocket, steam_id* pSteamIDRemote, int* peSocketStatus,
	                               unsigned int* punIPRemote, unsigned short* punPortRemote)
	{
		return false;
	}

	bool networking::GetListenSocketInfo(unsigned int hListenSocket, unsigned int* pnIP, unsigned short* pnPort)
	{
		return false;
	}

	int networking::GetSocketConnectionType(unsigned int hSocket)
	{
		return 0;
	}

	int networking::GetMaxPacketSize(unsigned int hSocket)
	{
		return 0;
	}
}
