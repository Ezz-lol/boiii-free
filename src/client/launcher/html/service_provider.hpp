#pragma once

class service_provider : public IServiceProvider
{
public:
	virtual ~service_provider() = default;

	HRESULT STDMETHODCALLTYPE QueryService(
		REFGUID guidService,
		REFIID riid,
		void** ppvObject) override
	{
		if (IsEqualGUID(riid, IID_IInternetSecurityManager))
		{
			return QueryInterface(riid, ppvObject);
		}

		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}
};

