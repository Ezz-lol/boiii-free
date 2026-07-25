#pragma once

class service_provider : public IServiceProvider {
public:
  virtual ~service_provider() = default;
  HRESULT STDMETHODCALLTYPE QueryService(REFGUID, REFIID interface_id,
                                         void **object) override {
    if (IsEqualGUID(interface_id, IID_IInternetSecurityManager))
      return QueryInterface(interface_id, object);
    *object = nullptr;
    return E_NOINTERFACE;
  }
};
