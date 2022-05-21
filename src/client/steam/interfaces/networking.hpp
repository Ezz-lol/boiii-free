#pragma once

namespace steam
{
	class networking
	{
	public:
		~networking() = default;

		virtual bool SendP2PPacket(steam_id steamIDRemote, const void* pubData, unsigned int cubData, int eP2PSendType);
		virtual bool IsP2PPacketAvailable(unsigned int* pcubMsgSize, int idk);
		virtual bool ReadP2PPacket(void* pubDest, unsigned int cubDest, unsigned int* pcubMsgSize,
		                           steam_id* psteamIDRemote);
		virtual bool AcceptP2PSessionWithUser(steam_id steamIDRemote);
		virtual bool CloseP2PSessionWithUser(steam_id steamIDRemote);
		virtual bool CloseP2PChannelWithUser(steam_id steamIDRemote, int iVirtualPort);
		virtual bool GetP2PSessionState(steam_id steamIDRemote, void* pConnectionState);
		virtual bool AllowP2PPacketRelay(bool bAllow);
		virtual unsigned int CreateListenSocket(int nVirtualP2PPort, unsigned int nIP, unsigned short nPort,
		                                        bool bAllowUseOfPacketRelay);
		virtual unsigned int CreateP2PConnectionSocket(steam_id steamIDTarget, int nVirtualPort, int nTimeoutSec,
		                                               bool bAllowUseOfPacketRelay);
		virtual unsigned int CreateConnectionSocket(unsigned int nIP, unsigned short nPort, int nTimeoutSec);
		virtual bool DestroySocket(unsigned int hSocket, bool bNotifyRemoteEnd);
		virtual bool DestroyListenSocket(unsigned int hSocket, bool bNotifyRemoteEnd);
		virtual bool SendDataOnSocket(unsigned int hSocket, void* pubData, unsigned int cubData, bool bReliable);
		virtual bool IsDataAvailableOnSocket(unsigned int hSocket, unsigned int* pcubMsgSize);
		virtual bool RetrieveDataFromSocket(unsigned int hSocket, void* pubDest, unsigned int cubDest,
		                                    unsigned int* pcubMsgSize);
		virtual bool IsDataAvailable(unsigned int hListenSocket, unsigned int* pcubMsgSize, unsigned int* phSocket);
		virtual bool RetrieveData(unsigned int hListenSocket, void* pubDest, unsigned int cubDest,
		                          unsigned int* pcubMsgSize, unsigned int* phSocket);
		virtual bool GetSocketInfo(unsigned int hSocket, steam_id* pSteamIDRemote, int* peSocketStatus,
		                           unsigned int* punIPRemote, unsigned short* punPortRemote);
		virtual bool GetListenSocketInfo(unsigned int hListenSocket, unsigned int* pnIP, unsigned short* pnPort);
		virtual int GetSocketConnectionType(unsigned int hSocket);
		virtual int GetMaxPacketSize(unsigned int hSocket);
	};
}
