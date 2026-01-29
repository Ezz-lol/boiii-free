#pragma once

class internet_security_manager : public IInternetSecurityManager
{
public:
	HRESULT STDMETHODCALLTYPE SetSecuritySite(
		IInternetSecurityMgrSite*) override
	{
		return INET_E_DEFAULT_ACTION;
	}

	HRESULT STDMETHODCALLTYPE GetSecuritySite(
		IInternetSecurityMgrSite**) override
	{
		return INET_E_DEFAULT_ACTION;
	}

	HRESULT STDMETHODCALLTYPE MapUrlToZone(
		LPCWSTR,
		DWORD* pdwZone,
		DWORD) override
	{
		*pdwZone = URLZONE_TRUSTED;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetSecurityId(
		LPCWSTR,
		BYTE*,
		DWORD*,
		DWORD_PTR) override
	{
		return INET_E_DEFAULT_ACTION;
	}

	HRESULT STDMETHODCALLTYPE ProcessUrlAction(
		LPCWSTR,
		DWORD,
		BYTE*,
		DWORD,
		BYTE*,
		DWORD,
		DWORD,
		DWORD) override
	{
		return INET_E_DEFAULT_ACTION;
	}

	HRESULT STDMETHODCALLTYPE QueryCustomPolicy(
		LPCWSTR,
		REFGUID,
		BYTE**,
		DWORD*,
		BYTE*,
		DWORD,
		DWORD) override
	{
		return INET_E_DEFAULT_ACTION;
	}

	HRESULT STDMETHODCALLTYPE SetZoneMapping(
		DWORD,
		LPCWSTR,
		DWORD) override
	{
		return INET_E_DEFAULT_ACTION;
	}

	HRESULT STDMETHODCALLTYPE GetZoneMappings(
		DWORD,
		IEnumString**,
		DWORD) override
	{
		return INET_E_DEFAULT_ACTION;
	}
};
