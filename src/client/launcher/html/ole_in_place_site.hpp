#pragma once

class ole_in_place_site : public IOleInPlaceSite
{
public:
	virtual ~ole_in_place_site() = default;

	HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE CanInPlaceActivate() override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnInPlaceActivate() override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnUIActivate() override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetWindowContext(LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc,
	                                           const LPRECT lprcPosRect, const LPRECT lprcClipRect,
	                                           const LPOLEINPLACEFRAMEINFO lpFrameInfo) override
	{
		ZeroMemory(lprcPosRect, sizeof(*lprcPosRect));
		ZeroMemory(lprcClipRect, sizeof(*lprcClipRect));

		CComPtr<IOleInPlaceFrame> ole_in_place_frame{};
		if (FAILED(QueryInterface(IID_IOleInPlaceFrame, reinterpret_cast<void**>(&ole_in_place_frame))))
		{
			*lplpFrame = nullptr;
			*lplpDoc = nullptr;
			return E_FAIL;
		}

		*lplpFrame = ole_in_place_frame;
		*lplpDoc = nullptr;

		lpFrameInfo->fMDIApp = FALSE;
		lpFrameInfo->haccel = nullptr;
		lpFrameInfo->cAccelEntries = 0;
		GetWindow(&lpFrameInfo->hwndFrame);

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Scroll(SIZE) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL) override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate() override
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE DiscardUndoState() override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE DeactivateAndUndo() override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE OnPosRectChange(const LPCRECT lprcPosRect) override
	{
		CComPtr<IOleInPlaceObject> in_place{};
		if (SUCCEEDED(QueryInterface(IID_IOleInPlaceObject, reinterpret_cast<void**>(&in_place))))
		{
			in_place->SetObjectRects(lprcPosRect, lprcPosRect);
		}

		return S_OK;
	}
};
