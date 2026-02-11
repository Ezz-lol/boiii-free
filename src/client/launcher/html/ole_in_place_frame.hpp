#pragma once

class ole_in_place_frame : public IOleInPlaceFrame
{
public:
	virtual ~ole_in_place_frame() = default;

	HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetBorder(LPRECT) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RequestBorderSpace(LPCBORDERWIDTHS) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE SetBorderSpace(LPCBORDERWIDTHS) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE SetActiveObject(IOleInPlaceActiveObject* pActiveObject, LPCOLESTR pszObjName) override
	{
		(void)pszObjName;
		this->active_object_ = pActiveObject;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE SetMenu(HMENU, HOLEMENU, HWND) override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RemoveMenus(HMENU hmenuShared) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR) override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE EnableModeless(BOOL) override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG lpmsg, WORD wID) override
	{
		(void)wID;
		if (this->active_object_)
		{
			return this->active_object_->TranslateAccelerator(lpmsg);
		}
		return E_NOTIMPL;
	}

	IOleInPlaceActiveObject* get_active_object() const
	{
		return this->active_object_;
	}

private:
	CComPtr<IOleInPlaceActiveObject> active_object_;
};
