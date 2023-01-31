#include <std_include.hpp>
#include "html_frame.hpp"
#include "utils/nt.hpp"
#include "utils/io.hpp"
#include "utils/hook.hpp"


namespace
{
	void* original_func{};
	GUID browser_emulation_guid{0xac969931, 0x3566, 0x4b50, {0xae, 0x48, 0x71, 0xb9, 0x6a, 0x75, 0xc8, 0x79}};

	int WINAPI co_internet_feature_value_internal_stub(const GUID* guid, uint32_t* result)
	{
		const auto res = static_cast<decltype(co_internet_feature_value_internal_stub)*>(original_func)(guid, result);

		if (IsEqualGUID(*guid, browser_emulation_guid))
		{
			*result = 11000;
			return 0;
		}

		return res;
	}

	void patch_cached_browser_emulator(const utils::nt::library& urlmon)
	{
		std::string data{};
		if(!utils::io::read_file(urlmon.get_path().generic_string(), &data))
		{
			return;
		}

		const utils::nt::library file_lib(reinterpret_cast<HMODULE>(data.data()));

		auto translate_file_offset_to_rva = [&](const size_t file_offset) -> size_t
		{
			const auto sections = file_lib.get_section_headers();
			for(const auto* section :  sections)
			{
				if(section->PointerToRawData <= file_offset && section->PointerToRawData + section->SizeOfRawData > file_offset)
				{
					const auto section_va = file_offset - section->PointerToRawData;
					return section_va + section->VirtualAddress;
				}
			}

			return 0;
		};

		const auto guid_pos = data.find(std::string(reinterpret_cast<const char*>(&browser_emulation_guid), sizeof(browser_emulation_guid)));
		if(guid_pos == std::string::npos)
		{
			return;
		}

		const auto guid_rva = translate_file_offset_to_rva(guid_pos);
		const auto guid_va = reinterpret_cast<GUID*>(urlmon.get_ptr() + guid_rva);

		if (!IsEqualGUID(*guid_va, browser_emulation_guid))
		{
			return;
		}

		const size_t unrelocated_guid_va = file_lib.get_optional_header()->ImageBase + guid_rva;
		const auto guid_ptr_pos = data.find(std::string(reinterpret_cast<const char*>(&unrelocated_guid_va), sizeof(unrelocated_guid_va)));
		if (guid_ptr_pos == std::string::npos)
		{
			return;
		}

		const auto guid_ptr_rva = translate_file_offset_to_rva(guid_ptr_pos);
		*reinterpret_cast<GUID**>(urlmon.get_ptr() + guid_ptr_rva) = guid_va;
	}

	void setup_ie_hooks()
	{
		static const auto _ = []
		{
			const auto urlmon = utils::nt::library::load("urlmon.dll"s);
			const auto target = urlmon.get_iat_entry("iertutil.dll", MAKEINTRESOURCEA(700));

			original_func = *target;
			utils::hook::set(target, co_internet_feature_value_internal_stub);

			patch_cached_browser_emulator(urlmon);

			return 0;
		}();
		(void)_;
	}

	void setup_ole()
	{
		static struct ole_initialzer
		{
			ole_initialzer()
			{
				if (OleInitialize(nullptr) != S_OK)
				{
					throw std::runtime_error("Unable to initialize the OLE library");
				}
			}

			~ole_initialzer()
			{
				OleUninitialize();
			}
		} init;
		(void)init;
	}
}

html_frame::html_frame()
{
	setup_ie_hooks();
	setup_ole();
}

HRESULT html_frame::GetHostInfo(DOCHOSTUIINFO* pInfo)
{
	pInfo->cbSize = sizeof(DOCHOSTUIINFO);
	pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DPI_AWARE | DOCHOSTUIFLAG_SCROLL_NO;
	pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

	return S_OK;
}

HRESULT html_frame::GetWindow(HWND* lphwnd)
{
	*lphwnd = this->window_;
	return S_OK;
}

HRESULT html_frame::QueryInterface(REFIID riid, void** ppvObject)
{
	if (IsEqualGUID(riid, IID_IDispatch))
	{
		*ppvObject = static_cast<IDispatch*>(this);
		return S_OK;
	}

	if (IsEqualGUID(riid, IID_IDispatch))
	{
		const auto d = get_dispatch();
		if (!d)
		{
			return E_NOINTERFACE;
		}

		(*d).AddRef();
		*ppvObject = &*d;
		return S_OK;
	}

	if (IsEqualGUID(riid, IID_IServiceProvider))
	{
		*ppvObject = static_cast<IServiceProvider*>(this);
		return S_OK;
	}

	if (IsEqualGUID(riid, IID_IInternetSecurityManager))
	{
		*ppvObject = static_cast<IInternetSecurityManager*>(this);
		 return S_OK;
	}

	if (IsEqualGUID(riid, IID_IUnknown))
	{
		*ppvObject = static_cast<IUnknown*>(static_cast<IOleClientSite*>(this));
		return S_OK;
	}

	if (IsEqualGUID(riid, IID_IOleClientSite))
	{
		*ppvObject = static_cast<IOleClientSite*>(this);
		return S_OK;
	}

	if (IsEqualGUID(riid, IID_IOleInPlaceSite))
	{
		*ppvObject = static_cast<IOleInPlaceSite*>(this);
		return S_OK;
	}

	if (IsEqualGUID(riid, IID_IOleInPlaceFrame))
	{
		*ppvObject = static_cast<IOleInPlaceFrame*>(this);
		return S_OK;
	}

	if (IsEqualGUID(riid, IID_IDocHostUIHandler))
	{
		*ppvObject = static_cast<IDocHostUIHandler*>(this);
		return S_OK;
	}

	if (IsEqualGUID(riid, IID_IOleInPlaceObject) && this->browser_object_)
	{
		return this->browser_object_->QueryInterface(riid, ppvObject);
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

HWND html_frame::get_window() const
{
	return this->window_;
}

CComPtr<IOleObject> html_frame::get_browser_object() const
{
	return this->browser_object_;
}

CComPtr<IWebBrowser2> html_frame::get_web_browser() const
{
	CComPtr<IWebBrowser2> web_browser{};
	if (!this->browser_object_ || FAILED(this->browser_object_.QueryInterface(&web_browser)))
	{
		return {};
	}

	return web_browser;
}

CComPtr<IDispatch> html_frame::get_dispatch() const
{
	const auto web_browser = this->get_web_browser();

	CComPtr<IDispatch> dispatch{};
	if (!web_browser || FAILED(web_browser->get_Document(&dispatch)))
	{
		return {};
	}

	return dispatch;
}

CComPtr<IHTMLDocument2> html_frame::get_document() const
{
	const auto dispatch = this->get_dispatch();

	CComPtr<IHTMLDocument2> document{};
	if (!dispatch || FAILED(dispatch.QueryInterface(&document)))
	{
		return {};
	}

	return document;
}

void html_frame::initialize(const HWND window)
{
	if (this->window_) return;
	this->window_ = window;

	this->create_browser();
	this->initialize_browser();
}

void html_frame::create_browser()
{
	CComPtr<IClassFactory> class_factory{};
	if (FAILED(
		CoGetClassObject(CLSID_WebBrowser, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, nullptr, IID_IClassFactory,
			reinterpret_cast<void **>(&class_factory))))
	{
		throw std::runtime_error("Unable to get the class factory");
	}

	class_factory->CreateInstance(nullptr, IID_IOleObject, reinterpret_cast<void**>(&this->browser_object_));

	if (!this->browser_object_)
	{
		throw std::runtime_error("Unable to create browser object");
	}
}

void html_frame::initialize_browser()
{
	this->browser_object_->SetClientSite(this);
	this->browser_object_->SetHostNames(L"Hostname", nullptr);

	RECT rect;
	GetClientRect(this->get_window(), &rect);
	OleSetContainedObject(this->browser_object_, TRUE);

	this->browser_object_->DoVerb(OLEIVERB_SHOW, nullptr, this, -1, this->get_window(), &rect);
	this->resize(rect.right, rect.bottom);
}

void html_frame::resize(const DWORD width, const DWORD height) const
{
	const auto web_browser = this->get_web_browser();
	if (web_browser)
	{
		web_browser->put_Left(0);
		web_browser->put_Top(0);
		web_browser->put_Width(width);
		web_browser->put_Height(height);
	}
}

bool html_frame::load_url(const std::string& url) const
{
	auto web_browser = this->get_web_browser();
	if (!web_browser) return false;

	CComVariant my_url(url.data());
	return SUCCEEDED(web_browser->Navigate2(&my_url, nullptr, nullptr, nullptr, nullptr));
}

bool html_frame::load_html(const std::string& html) const
{
	if (!this->load_url("about:blank")) return false;

	const auto document = this->get_document();
	if (!document) return false;

	CComSafeArrayBound bound{};
	bound.SetCount(1);
	bound.SetLowerBound(0);

	CComSafeArray<VARIANT> array(&bound, 1);
	array[0] = CComVariant(html.data());

	document->write(array);
	document->close();

	return true;
}

html_argument html_frame::evaluate(const std::string& javascript) const
{
	auto dispDoc = this->get_dispatch();
	
	CComPtr<IHTMLDocument2> htmlDoc;
	dispDoc->QueryInterface(&htmlDoc);

	CComPtr<IHTMLWindow2> htmlWindow;
	htmlDoc->get_parentWindow(&htmlWindow);

	CComDispatchDriver dispWindow;
	htmlWindow->QueryInterface(&dispWindow);

	CComPtr<IDispatchEx> dispexWindow;
	htmlWindow->QueryInterface(&dispexWindow);

	DISPID dispidEval = -1;
	dispexWindow->GetDispID(CComBSTR("eval"), fdexNameCaseSensitive, &dispidEval);

	CComVariant result{};
	CComVariant code(javascript.data());
	(void)dispWindow.Invoke1(dispidEval, &code, &result);

	return result;
}

int html_frame::get_callback_id(const std::string& name) const
{
	for (auto i = 0u; i < this->callbacks_.size(); ++i)
	{
		if (this->callbacks_[i].first == name)
		{
			return i;
		}
	}

	return -1;
}

html_argument html_frame::invoke_callback(const int id, const std::vector<html_argument>& params) const
{
	if (id >= 0 && static_cast<unsigned int>(id) < this->callbacks_.size())
	{
		return this->callbacks_[id].second(params);
	}

	return {};
}

void html_frame::register_callback(const std::string& name, const std::function<CComVariant(const std::vector<html_argument>&)>& callback)
{
	this->callbacks_.emplace_back(name, callback);
}

HRESULT html_frame::GetIDsOfNames(const IID& /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID /*lcid*/,
                                  DISPID* rgDispId)
{
	for (unsigned int i = 0; i < cNames; ++i)
	{
		std::wstring wide_name(rgszNames[i]);
		std::string name(wide_name.begin(), wide_name.end());

		rgDispId[i] = this->get_callback_id(name);
	}

	return S_OK;
}

HRESULT html_frame::Invoke(const DISPID dispIdMember, const IID& /*riid*/, LCID /*lcid*/, WORD /*wFlags*/,
                           DISPPARAMS* pDispParams,
                           VARIANT* pVarResult, EXCEPINFO* /*pExcepInfo*/, UINT* /*puArgErr*/)
{
	std::vector<html_argument> params{};
	for (auto i = pDispParams->cArgs; i > 0; --i)
	{
		auto& param = pDispParams->rgvarg[i - 1];
		params.emplace_back(param);
	}

	auto res = this->invoke_callback(dispIdMember, params);
	res.move_to(pVarResult);

	return S_OK;
}

HRESULT html_frame::GetExternal(IDispatch** ppDispatch)
{
	*ppDispatch = this;
	return *ppDispatch ? S_OK : S_FALSE;
}
