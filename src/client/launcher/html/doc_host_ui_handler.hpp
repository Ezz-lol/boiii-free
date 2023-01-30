#pragma once

class doc_host_ui_handler /*_boilerplate*/ : public IDocHostUIHandler
{
public:
	virtual ~doc_host_ui_handler() = default;

	HRESULT STDMETHODCALLTYPE ShowContextMenu(
		DWORD,
		POINT*,
		IUnknown*,
		IDispatch*) override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE ShowUI(
		DWORD,
		IOleInPlaceActiveObject*,
		IOleCommandTarget*,
		IOleInPlaceFrame*,
		IOleInPlaceUIWindow*) override
	{
		return S_OK;
	}
	
	HRESULT STDMETHODCALLTYPE HideUI() override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE UpdateUI() override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE EnableModeless(BOOL) override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnDocWindowActivate(BOOL) override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(BOOL) override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE ResizeBorder(
		LPCRECT,
		IOleInPlaceUIWindow*,
		BOOL) override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE TranslateAccelerator(
		LPMSG,
		const GUID*,
		DWORD) override
	{
		return S_FALSE;
	}

	HRESULT STDMETHODCALLTYPE GetOptionKeyPath(LPOLESTR __RPC_FAR* pchKey, DWORD dw) override
	{
		return S_FALSE;
	}

	HRESULT STDMETHODCALLTYPE GetDropTarget(
		IDropTarget*,
		IDropTarget**) override
	{
		return S_FALSE;
	}


	HRESULT STDMETHODCALLTYPE TranslateUrl(DWORD, OLECHAR*, OLECHAR** ppchURLOut) override
	{
		*ppchURLOut = nullptr;
		return S_FALSE;
	}

	HRESULT STDMETHODCALLTYPE FilterDataObject(IDataObject*, IDataObject** ppDORet) override
	{
		*ppDORet = nullptr;
		return S_FALSE;
	}
};
