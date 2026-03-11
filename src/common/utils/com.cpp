#include "com.hpp"
#include "nt.hpp"
#include "string.hpp"
#include "finally.hpp"

#include <stdexcept>

#include <ShlObj.h>


namespace utils::com
{
	namespace
	{
		void initialize_com()
		{
			thread_local struct x
			{
				x()
				{
					if (FAILED(CoInitialize(nullptr)))
					{
						throw std::runtime_error("Failed to initialize the component object model");
					}
				}

				~x()
				{
					CoUninitialize();
				}
			} xx;
		}
	}

	bool select_folder(std::string& out_folder, const std::string& title, const std::string& selected_folder)
	{
		initialize_com();

		const std::wstring wide_title(title.begin(), title.end());

		IFileOpenDialog* pfd = nullptr;
		if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))) || !pfd)
			return false;

		DWORD opts = 0;
		pfd->GetOptions(&opts);
		pfd->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
		pfd->SetTitle(wide_title.c_str());

		if (!selected_folder.empty())
		{
			std::wstring wide_sel(selected_folder.begin(), selected_folder.end());
			for (auto& c : wide_sel) if (c == L'/') c = L'\\';
			IShellItem* psi_default = nullptr;
			if (SUCCEEDED(SHCreateItemFromParsingName(wide_sel.c_str(), nullptr, IID_PPV_ARGS(&psi_default))) && psi_default)
			{
				pfd->SetFolder(psi_default);
				psi_default->Release();
			}
		}

		HRESULT hr = pfd->Show(nullptr);
		if (FAILED(hr)) { pfd->Release(); return false; }

		IShellItem* psi = nullptr;
		if (FAILED(pfd->GetResult(&psi)) || !psi) { pfd->Release(); return false; }

		LPWSTR path_buf = nullptr;
		bool ok = SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &path_buf)) && path_buf;
		if (ok)
		{
			out_folder = string::convert(std::wstring(path_buf));
			CoTaskMemFree(path_buf);
		}
		psi->Release();
		pfd->Release();
		return ok;
	}

	CComPtr<IProgressDialog> create_progress_dialog()
	{
		initialize_com();

		CComPtr<IProgressDialog> progress_dialog{};
		if (FAILED(
			CoCreateInstance(CLSID_ProgressDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&progress_dialog))))
		{
			throw std::runtime_error("Failed to create co instance");
		}

		return progress_dialog;
	}
}
