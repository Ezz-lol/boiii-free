#pragma once

class ole_in_place_site : public IOleInPlaceSite {
public:
  virtual ~ole_in_place_site() = default;
  HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE CanInPlaceActivate() override { return S_OK; }
  HRESULT STDMETHODCALLTYPE OnInPlaceActivate() override { return S_OK; }
  HRESULT STDMETHODCALLTYPE OnUIActivate() override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetWindowContext(
      LPOLEINPLACEFRAME *frame, LPOLEINPLACEUIWINDOW *document, LPRECT position,
      LPRECT clip, LPOLEINPLACEFRAMEINFO info) override {
    ZeroMemory(position, sizeof(*position));
    ZeroMemory(clip, sizeof(*clip));
    CComPtr<IOleInPlaceFrame> inplace_frame;
    if (FAILED(QueryInterface(IID_IOleInPlaceFrame,
                              reinterpret_cast<void **>(&inplace_frame)))) {
      *frame = nullptr;
      *document = nullptr;
      return E_FAIL;
    }
    *frame = inplace_frame.Detach();
    *document = nullptr;
    info->fMDIApp = FALSE;
    info->haccel = nullptr;
    info->cAccelEntries = 0;
    GetWindow(&info->hwndFrame);
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE Scroll(SIZE) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate() override { return S_OK; }
  HRESULT STDMETHODCALLTYPE DiscardUndoState() override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE DeactivateAndUndo() override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT position) override {
    CComPtr<IOleInPlaceObject> inplace;
    if (SUCCEEDED(QueryInterface(IID_IOleInPlaceObject,
                                 reinterpret_cast<void **>(&inplace))))
      inplace->SetObjectRects(position, position);
    return S_OK;
  }
};
