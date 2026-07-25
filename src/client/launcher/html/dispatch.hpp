#pragma once

class dispatch : public IDispatch {
public:
  virtual ~dispatch() = default;

  HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *) override {
    return S_FALSE;
  }
  HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT, LCID, ITypeInfo **) override {
    return S_FALSE;
  }
};
