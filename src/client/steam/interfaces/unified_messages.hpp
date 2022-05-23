#pragma once

namespace steam
{
	using ClientUnifiedMessageHandle = uint64_t;

	class unified_messages
	{
	public:
		~unified_messages() = default;

		virtual ClientUnifiedMessageHandle SendMethod(const char* pchServiceMethod, const void* pRequestBuffer,
		                                              uint32_t unRequestBufferSize, uint64_t unContext);
		virtual bool GetMethodResponseInfo(ClientUnifiedMessageHandle hHandle, uint32_t* punResponseSize,
		                                   uint32_t* peResult);
		virtual bool GetMethodResponseData(ClientUnifiedMessageHandle hHandle, void* pResponseBuffer,
		                                   uint32_t unResponseBufferSize, bool bAutoRelease);
		virtual bool ReleaseMethod(ClientUnifiedMessageHandle hHandle);
		virtual bool SendNotification(const char* pchServiceNotification, const void* pNotificationBuffer,
		                              uint32_t unNotificationBufferSize);
	};
}
