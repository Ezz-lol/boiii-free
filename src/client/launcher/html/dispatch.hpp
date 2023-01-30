#pragma once


class dispatch : public IDispatch
{
public:
	virtual ~dispatch() = default;

	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctinfo) override
	{
		return S_FALSE;
	}

	HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override
	{
		return S_FALSE;
	}

};
