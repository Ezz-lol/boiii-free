#include <std_include.hpp>
#include "../steam.hpp"

namespace steam
{
	HTTPRequestHandle http::http::CreateHTTPRequest(uint32_t eHTTPRequestMethod, const char* pchAbsoluteURL)
	{
		return 0;
	}

	bool http::SetHTTPRequestContextValue(HTTPRequestHandle hRequest, uint64_t ulContextValue)
	{
		return false;
	}

	bool http::SetHTTPRequestNetworkActivityTimeout(HTTPRequestHandle hRequest, uint32_t unTimeoutSeconds)
	{
		return false;
	}

	bool http::SetHTTPRequestHeaderValue(HTTPRequestHandle hRequest, const char* pchHeaderName,
	                                     const char* pchHeaderValue)
	{
		return false;
	}

	bool http::SetHTTPRequestGetOrPostParameter(HTTPRequestHandle hRequest, const char* pchParamName,
	                                            const char* pchParamValue)
	{
		return false;
	}

	bool http::SendHTTPRequest(HTTPRequestHandle hRequest, uint64_t* pCallHandle)
	{
		return false;
	}

	bool http::SendHTTPRequestAndStreamResponse(HTTPRequestHandle hRequest, uint64_t* pCallHandle)
	{
		return false;
	}

	bool http::DeferHTTPRequest(HTTPRequestHandle hRequest)
	{
		return false;
	}

	bool http::PrioritizeHTTPRequest(HTTPRequestHandle hRequest)
	{
		return false;
	}

	bool http::GetHTTPResponseHeaderSize(HTTPRequestHandle hRequest, const char* pchHeaderName,
	                                     uint32_t* unResponseHeaderSize)
	{
		return false;
	}

	bool http::GetHTTPResponseHeaderValue(HTTPRequestHandle hRequest, const char* pchHeaderName,
	                                      uint8_t* pHeaderValueBuffer, uint32_t unBufferSize)
	{
		return false;
	}

	bool http::GetHTTPResponseBodySize(HTTPRequestHandle hRequest, uint32_t* unBodySize)
	{
		return false;
	}

	bool http::GetHTTPResponseBodyData(HTTPRequestHandle hRequest, uint8_t* pBodyDataBuffer, uint32_t unBufferSize)
	{
		return false;
	}

	bool http::GetHTTPStreamingResponseBodyData(HTTPRequestHandle hRequest, uint32_t cOffset, uint8_t* pBodyDataBuffer,
	                                            uint32_t unBufferSize)
	{
		return false;
	}

	bool http::ReleaseHTTPRequest(HTTPRequestHandle hRequest)
	{
		return false;
	}

	bool http::GetHTTPDownloadProgressPct(HTTPRequestHandle hRequest, float* pflPercentOut)
	{
		return false;
	}

	bool http::SetHTTPRequestRawPostBody(HTTPRequestHandle hRequest, const char* pchContentType, uint8_t* pubBody,
	                                     uint32_t unBodyLen)
	{
		return false;
	}

	HTTPCookieContainerHandle http::CreateCookieContainer(bool bAllowResponsesToModify)
	{
		return 0;
	}

	bool http::ReleaseCookieContainer(HTTPCookieContainerHandle hCookieContainer)
	{
		return false;
	}

	bool http::SetCookie(HTTPCookieContainerHandle hCookieContainer, const char* pchHost, const char* pchUrl,
	                     const char* pchCookie)
	{
		return false;
	}

	bool http::SetHTTPRequestCookieContainer(HTTPRequestHandle hRequest, HTTPCookieContainerHandle hCookieContainer)
	{
		return false;
	}

	bool http::SetHTTPRequestUserAgentInfo(HTTPRequestHandle hRequest, const char* pchUserAgentInfo)
	{
		return false;
	}

	bool http::SetHTTPRequestRequiresVerifiedCertificate(HTTPRequestHandle hRequest, bool bRequireVerifiedCertificate)
	{
		return false;
	}

	bool http::SetHTTPRequestAbsoluteTimeoutMS(HTTPRequestHandle hRequest, uint32_t unMilliseconds)
	{
		return false;
	}

	bool http::GetHTTPRequestWasTimedOut(HTTPRequestHandle hRequest, bool* pbWasTimedOut)
	{
		return false;
	}
}
