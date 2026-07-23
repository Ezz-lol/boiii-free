#include <std_include.hpp>
#include "html_frame.hpp"
#include <wrl.h>

namespace {
void setup_com() {
  static struct com_initializer {
    com_initializer() {
      const auto result = OleInitialize(nullptr);
      if (FAILED(result)) {
        throw std::runtime_error("Unable to initialize COM");
      }
    }

    ~com_initializer() { OleUninitialize(); }
  } initializer;
  (void)initializer;
}

std::wstring to_wide(const std::string &value) {
  if (value.empty()) {
    return {};
  }

  const auto size = MultiByteToWideChar(
      CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
  std::wstring result(size, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                      result.data(), size);
  return result;
}

std::string to_narrow(const wchar_t *value) {
  if (!value || !*value) {
    return {};
  }

  const auto length = static_cast<int>(wcslen(value));
  const auto size = WideCharToMultiByte(CP_UTF8, 0, value, length, nullptr, 0,
                                        nullptr, nullptr);
  std::string result(size, '\0');
  WideCharToMultiByte(CP_UTF8, 0, value, length, result.data(), size, nullptr,
                      nullptr);
  return result;
}

std::wstring get_webview_data_path() {
  wchar_t local_app_data[MAX_PATH]{};
  if (FAILED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr,
                              SHGFP_TYPE_CURRENT, local_app_data))) {
    return {};
  }

  auto path = std::filesystem::path(local_app_data) / "EZZ BOIII" / "WebView2";
  std::error_code error;
  std::filesystem::create_directories(path, error);
  return error ? std::wstring{} : path.wstring();
}

class webview_dispatch final
    : public Microsoft::WRL::RuntimeClass<
          Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
          IDispatch> {
public:
  explicit webview_dispatch(html_frame *frame) : frame_(frame) {}

  HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *count) override {
    if (!count) {
      return E_POINTER;
    }
    *count = 0;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT, LCID, ITypeInfo **) override {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID, LPOLESTR *names, UINT count,
                                          LCID, DISPID *ids) override {
    if (!names || !ids) {
      return E_POINTER;
    }

    for (UINT i = 0; i < count; ++i) {
      const auto name = to_narrow(names[i]);
      if (name == "__request") {
        ids[i] = request_id;
        continue;
      }
      if (name == "__response") {
        ids[i] = response_id;
        continue;
      }
      const auto id = this->frame_->get_callback_id(name);
      if (id < 0) {
        ids[i] = DISPID_UNKNOWN;
        return DISP_E_UNKNOWNNAME;
      }
      ids[i] = id;
    }
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE Invoke(DISPID id, REFIID, LCID, WORD,
                                   DISPPARAMS *parameters, VARIANT *result,
                                   EXCEPINFO *, UINT *) override {
    if (id == request_id) {
      this->response_.Clear();
      if (!parameters || parameters->cArgs < 1) {
        return DISP_E_BADPARAMCOUNT;
      }

      html_argument request_argument(&parameters->rgvarg[0]);
      if (!request_argument.is_string()) {
        return DISP_E_TYPEMISMATCH;
      }

      rapidjson::Document request;
      request.Parse(request_argument.get_string().c_str());
      if (!request.IsObject() || !request.HasMember("name") ||
          !request["name"].IsString()) {
        return E_INVALIDARG;
      }

      const auto callback_id =
          this->frame_->get_callback_id(request["name"].GetString());
      if (callback_id < 0) {
        return DISP_E_MEMBERNOTFOUND;
      }

      std::vector<html_argument> arguments;
      if (request.HasMember("args") && request["args"].IsArray()) {
        for (const auto &argument : request["args"].GetArray()) {
          if (argument.IsString()) {
            arguments.emplace_back(
                CComVariant(to_wide(argument.GetString()).c_str()));
          } else if (argument.IsBool()) {
            arguments.emplace_back(CComVariant(argument.GetBool()));
          } else if (argument.IsInt()) {
            arguments.emplace_back(CComVariant(argument.GetInt()));
          } else if (argument.IsNumber()) {
            arguments.emplace_back(CComVariant(argument.GetDouble()));
          } else {
            arguments.emplace_back(CComVariant());
          }
        }
      }

      auto value = this->frame_->invoke_callback(callback_id, arguments);
      value.copy_to(this->response_);
      return S_OK;
    }

    if (id == response_id) {
      if (result) {
        return VariantCopy(result, &this->response_);
      }
      return S_OK;
    }

    if (id < 0) {
      return DISP_E_MEMBERNOTFOUND;
    }

    std::vector<html_argument> arguments;
    if (parameters) {
      arguments.reserve(parameters->cArgs);
      for (auto i = parameters->cArgs; i > 0; --i) {
        arguments.emplace_back(parameters->rgvarg[i - 1]);
      }
    }

    auto value = this->frame_->invoke_callback(id, arguments);
    value.move_to(result);
    return S_OK;
  }

private:
  static constexpr DISPID request_id = 0x60000000;
  static constexpr DISPID response_id = 0x60000001;
  html_frame *frame_;
  CComVariant response_;
};
} // namespace

html_frame::html_frame() { setup_com(); }

html_frame::~html_frame() {
  this->host_object_.Release();
  this->webview_.Release();
  if (this->webview_controller_) {
    this->webview_controller_->Close();
    this->webview_controller_.Release();
  }
}

void html_frame::initialize(const HWND window) {
  if (this->window_) {
    return;
  }
  this->window_ = window;

  using Microsoft::WRL::Callback;
  const auto user_data_path = get_webview_data_path();
  const auto environment_handler = Callback<
      ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
      [this](const HRESULT result,
             ICoreWebView2Environment *environment) -> HRESULT {
        if (FAILED(result) || !environment) {
          this->show_webview_error(result);
          return S_OK;
        }

        const auto controller_handler =
            Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [this](const HRESULT controller_result,
                       ICoreWebView2Controller *controller) -> HRESULT {
                  if (FAILED(controller_result) || !controller) {
                    this->show_webview_error(controller_result);
                    return S_OK;
                  }

                  this->webview_controller_ = controller;
                  const auto view_result =
                      controller->get_CoreWebView2(&this->webview_);
                  if (FAILED(view_result) || !this->webview_) {
                    this->show_webview_error(view_result);
                    return S_OK;
                  }

                  this->configure_webview2();
                  return S_OK;
                });

        const auto controller_result =
            environment->CreateCoreWebView2Controller(this->window_,
                                                      controller_handler.Get());
        if (FAILED(controller_result)) {
          this->show_webview_error(controller_result);
        }
        return S_OK;
      });

  const auto result = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, user_data_path.empty() ? nullptr : user_data_path.c_str(),
      nullptr, environment_handler.Get());
  if (FAILED(result)) {
    this->show_webview_error(result);
  }
}

void html_frame::configure_webview2() {
  RECT bounds{};
  GetClientRect(this->window_, &bounds);
  this->webview_controller_->put_Bounds(bounds);

  CComPtr<ICoreWebView2Settings> settings;
  if (SUCCEEDED(this->webview_->get_Settings(&settings)) && settings) {
    settings->put_AreDevToolsEnabled(FALSE);
    settings->put_AreDefaultContextMenusEnabled(FALSE);
    settings->put_IsStatusBarEnabled(FALSE);
    settings->put_IsZoomControlEnabled(FALSE);
  }

  const auto bridge = Microsoft::WRL::Make<webview_dispatch>(this);
  this->host_object_ = bridge.Get();

  VARIANT host{};
  host.vt = VT_DISPATCH;
  host.pdispVal = this->host_object_;
  const auto host_result =
      this->webview_->AddHostObjectToScript(L"external", &host);
  if (FAILED(host_result)) {
    this->show_webview_error(host_result);
    return;
  }

  if (!this->pending_html_.empty()) {
    const auto html = to_wide(this->pending_html_);
    this->webview_->NavigateToString(html.c_str());
  } else if (!this->pending_url_.empty()) {
    const auto url = to_wide(this->pending_url_);
    this->webview_->Navigate(url.c_str());
  }
}

void html_frame::show_webview_error(const HRESULT result) const {
  const auto message = std::format(
      L"The Microsoft Edge WebView2 launcher could not start.\n\nError: "
      L"0x{:08X}",
      static_cast<unsigned long>(result));
  MessageBoxW(this->window_, message.c_str(), L"EZZ BOIII",
              MB_OK | MB_ICONERROR);
  PostMessageW(this->window_, WM_CLOSE, 0, 0);
}

void html_frame::resize(const DWORD width, const DWORD height) const {
  if (!this->webview_controller_) {
    return;
  }
  const RECT bounds{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
  this->webview_controller_->put_Bounds(bounds);
}

bool html_frame::load_url(const std::string &url) {
  this->pending_html_.clear();
  this->pending_url_ = url;
  if (!this->webview_) {
    return true;
  }
  const auto value = to_wide(url);
  return SUCCEEDED(this->webview_->Navigate(value.c_str()));
}

bool html_frame::load_html(const std::string &html) {
  this->pending_url_.clear();
  this->pending_html_ = html;
  if (!this->webview_) {
    return true;
  }
  const auto value = to_wide(html);
  return SUCCEEDED(this->webview_->NavigateToString(value.c_str()));
}

int html_frame::get_callback_id(const std::string &name) const {
  for (auto i = 0u; i < this->callbacks_.size(); ++i) {
    if (this->callbacks_[i].first == name) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

html_argument
html_frame::invoke_callback(const int id,
                            const std::vector<html_argument> &params) const {
  if (id >= 0 && static_cast<size_t>(id) < this->callbacks_.size()) {
    return this->callbacks_[id].second(params);
  }
  return {};
}

void html_frame::register_callback(
    const std::string &name,
    const std::function<CComVariant(const std::vector<html_argument> &)>
        &callback) {
  this->callbacks_.emplace_back(name, callback);
}
