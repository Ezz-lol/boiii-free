#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	ClientUnifiedMessageHandle unified_messages::SendMethod(const char* pchServiceMethod, const void* pRequestBuffer,
	                                                        uint32_t unRequestBufferSize, uint64_t unContext)
	{
		return 0;
	}

	bool unified_messages::GetMethodResponseInfo(ClientUnifiedMessageHandle hHandle, uint32_t* punResponseSize,
	                                             uint32_t* peResult)
	{
		return false;
	}

	bool unified_messages::GetMethodResponseData(ClientUnifiedMessageHandle hHandle, void* pResponseBuffer,
	                                             uint32_t unResponseBufferSize, bool bAutoRelease)
	{
		return false;
	}

	bool unified_messages::ReleaseMethod(ClientUnifiedMessageHandle hHandle)
	{
		return false;
	}

	bool unified_messages::SendNotification(const char* pchServiceNotification, const void* pNotificationBuffer,
	                                        uint32_t unNotificationBufferSize)
	{
		return false;
	}
}
