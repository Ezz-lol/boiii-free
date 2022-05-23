#pragma once

namespace steam
{
	using HTTPRequestHandle = uint64_t;
	using HTTPCookieContainerHandle = uint64_t;

	class http
	{
	public:
		~http() = default;

		virtual HTTPRequestHandle CreateHTTPRequest(uint32_t eHTTPRequestMethod, const char* pchAbsoluteURL);
		virtual bool SetHTTPRequestContextValue(HTTPRequestHandle hRequest, uint64_t ulContextValue);
		virtual bool SetHTTPRequestNetworkActivityTimeout(HTTPRequestHandle hRequest, uint32_t unTimeoutSeconds);
		virtual bool SetHTTPRequestHeaderValue(HTTPRequestHandle hRequest, const char* pchHeaderName,
		                                       const char* pchHeaderValue);
		virtual bool SetHTTPRequestGetOrPostParameter(HTTPRequestHandle hRequest, const char* pchParamName,
		                                              const char* pchParamValue);
		virtual bool SendHTTPRequest(HTTPRequestHandle hRequest, uint64_t* pCallHandle);
		virtual bool SendHTTPRequestAndStreamResponse(HTTPRequestHandle hRequest, uint64_t* pCallHandle);
		virtual bool DeferHTTPRequest(HTTPRequestHandle hRequest);
		virtual bool PrioritizeHTTPRequest(HTTPRequestHandle hRequest);
		virtual bool GetHTTPResponseHeaderSize(HTTPRequestHandle hRequest, const char* pchHeaderName,
		                                       uint32_t* unResponseHeaderSize);
		virtual bool GetHTTPResponseHeaderValue(HTTPRequestHandle hRequest, const char* pchHeaderName,
		                                        uint8_t* pHeaderValueBuffer, uint32_t unBufferSize);
		virtual bool GetHTTPResponseBodySize(HTTPRequestHandle hRequest, uint32_t* unBodySize);
		virtual bool GetHTTPResponseBodyData(HTTPRequestHandle hRequest, uint8_t* pBodyDataBuffer,
		                                     uint32_t unBufferSize);
		virtual bool GetHTTPStreamingResponseBodyData(HTTPRequestHandle hRequest, uint32_t cOffset,
		                                              uint8_t* pBodyDataBuffer, uint32_t unBufferSize);
		virtual bool ReleaseHTTPRequest(HTTPRequestHandle hRequest);
		virtual bool GetHTTPDownloadProgressPct(HTTPRequestHandle hRequest, float* pflPercentOut);
		virtual bool SetHTTPRequestRawPostBody(HTTPRequestHandle hRequest, const char* pchContentType, uint8_t* pubBody,
		                                       uint32_t unBodyLen);
		virtual HTTPCookieContainerHandle CreateCookieContainer(bool bAllowResponsesToModify);
		virtual bool ReleaseCookieContainer(HTTPCookieContainerHandle hCookieContainer);
		virtual bool SetCookie(HTTPCookieContainerHandle hCookieContainer, const char* pchHost, const char* pchUrl,
		                       const char* pchCookie);
		virtual bool SetHTTPRequestCookieContainer(HTTPRequestHandle hRequest,
		                                           HTTPCookieContainerHandle hCookieContainer);
		virtual bool SetHTTPRequestUserAgentInfo(HTTPRequestHandle hRequest, const char* pchUserAgentInfo);
		virtual bool SetHTTPRequestRequiresVerifiedCertificate(HTTPRequestHandle hRequest,
		                                                       bool bRequireVerifiedCertificate);
		virtual bool SetHTTPRequestAbsoluteTimeoutMS(HTTPRequestHandle hRequest, uint32_t unMilliseconds);
		virtual bool GetHTTPRequestWasTimedOut(HTTPRequestHandle hRequest, bool* pbWasTimedOut);
	};
}
