#pragma once
#include <std_include.hpp>
#include "dispatch.hpp"
#include "doc_host_ui_handler.hpp"
#include "html_argument.hpp"
#include "internet_security_manager.hpp"
#include "ole_client_site.hpp"
#include "ole_in_place_frame.hpp"
#include "ole_in_place_site.hpp"
#include "service_provider.hpp"
#include <WebView2.h>

class html_frame final : doc_host_ui_handler,
                         service_provider,
                         internet_security_manager,
                         ole_client_site,
                         ole_in_place_frame,
                         ole_in_place_site,
                         dispatch {
public:
  html_frame();
  html_frame(const html_frame &) = delete;
  html_frame &operator=(const html_frame &) = delete;
  html_frame(html_frame &&) = delete;
  html_frame &operator=(html_frame &&) = delete;
  ~html_frame();

  void initialize(HWND window);
  void resize(DWORD width, DWORD height) const;
  bool load_url(const std::string &url);
  bool load_html(const std::string &html);

  int get_callback_id(const std::string &name) const;
  html_argument invoke_callback(int id,
                                const std::vector<html_argument> &params) const;
  void register_callback(
      const std::string &name,
      const std::function<CComVariant(const std::vector<html_argument> &)>
          &callback);

private:
  HWND window_ = nullptr;
  bool use_legacy_browser_ = false;
  CComPtr<ICoreWebView2Controller> webview_controller_;
  CComPtr<ICoreWebView2> webview_;
  CComPtr<IDispatch> host_object_;
  CComPtr<IOleObject> browser_object_;
  std::string pending_url_;
  std::string pending_html_;

  std::vector<std::pair<std::string, std::function<html_argument(
                                         const std::vector<html_argument> &)>>>
      callbacks_;

  void configure_webview2();
  void handle_webview_error(HRESULT result);
  void show_webview_error(HRESULT result) const;
  void initialize_legacy_browser();
  CComPtr<IWebBrowser2> get_legacy_browser() const;
  CComPtr<IDispatch> get_legacy_dispatch() const;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID interface_id,
                                           void **object) override;
  ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
  ULONG STDMETHODCALLTYPE Release() override { return 1; }
  HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO *info) override;
  HRESULT STDMETHODCALLTYPE GetWindow(HWND *window) override;
  HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID, LPOLESTR *names, UINT count,
                                          LCID, DISPID *ids) override;
  HRESULT STDMETHODCALLTYPE Invoke(DISPID id, REFIID, LCID, WORD,
                                   DISPPARAMS *parameters, VARIANT *result,
                                   EXCEPINFO *, UINT *) override;
  HRESULT STDMETHODCALLTYPE GetExternal(IDispatch **dispatch) override;
};
