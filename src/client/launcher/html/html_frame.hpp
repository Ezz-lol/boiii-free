#pragma once

#include "html_argument.hpp"
#include <WebView2.h>

class html_frame final {
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
  CComPtr<ICoreWebView2Controller> webview_controller_;
  CComPtr<ICoreWebView2> webview_;
  CComPtr<IDispatch> host_object_;
  std::string pending_url_;
  std::string pending_html_;

  std::vector<std::pair<std::string, std::function<html_argument(
                                         const std::vector<html_argument> &)>>>
      callbacks_;

  void configure_webview2();
  void show_webview_error(HRESULT result) const;
};
