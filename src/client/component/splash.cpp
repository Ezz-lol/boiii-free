#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "resource.hpp"

#include <utils/nt.hpp>

namespace splash
{
	namespace
	{
		HANDLE load_splash_image()
		{
			const auto self = utils::nt::library::get_by_address(load_splash_image);
			return LoadImageA(self, MAKEINTRESOURCE(IMAGE_SPLASH), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
		}

		void enable_dpi_awareness()
		{
			const utils::nt::library user32{ "user32.dll" };
			const auto set_dpi = user32
				? user32.get_proc<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>(
					"SetProcessDpiAwarenessContext")
				: nullptr;
			if (set_dpi)
			{
				set_dpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
			}
		}
	}

	class component final : public component_interface
	{
	public:
		component()
		{
			image_ = load_splash_image();

			enable_dpi_awareness();
			this->show();
		}

		void pre_destroy() override
		{
			this->destroy();

			MSG msg;
			while (this->window_ && IsWindow(this->window_))
			{
				if (PeekMessageA(&msg, nullptr, NULL, NULL, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					std::this_thread::sleep_for(1ms);
				}
			}

			this->window_ = nullptr;
		}

		void post_unpack() override
		{
			this->destroy();
		}

	private:
		HWND window_{};
		HANDLE image_{};

		void destroy() const
		{
			if (this->window_ && IsWindow(this->window_))
			{
				ShowWindow(this->window_, SW_HIDE);
				DestroyWindow(this->window_);
				UnregisterClassA("Black Ops III Splash Screen", utils::nt::library{});
			}
		}

		void show()
		{
			WNDCLASSA wnd_class;

			const utils::nt::library host{};

			wnd_class.style = CS_DROPSHADOW;
			wnd_class.cbClsExtra = 0;
			wnd_class.cbWndExtra = 0;
			wnd_class.lpszMenuName = nullptr;
			wnd_class.lpfnWndProc = DefWindowProcA;
			wnd_class.hInstance = host;
			wnd_class.hIcon = LoadIconA(host, reinterpret_cast<LPCSTR>(102));
			wnd_class.hCursor = LoadCursorA(nullptr, IDC_APPSTARTING);
			wnd_class.hbrBackground = reinterpret_cast<HBRUSH>(6);
			wnd_class.lpszClassName = "Black Ops III Splash Screen";

			if (RegisterClassA(&wnd_class))
			{
				const auto x_pixels = GetSystemMetrics(SM_CXFULLSCREEN);
				const auto y_pixels = GetSystemMetrics(SM_CYFULLSCREEN);

				if (image_)
				{
					this->window_ = CreateWindowExA(WS_EX_APPWINDOW, "Black Ops III Splash Screen", "BOIII",
					                                WS_POPUP | WS_SYSMENU,
					                                (x_pixels - 320) / 2, (y_pixels - 100) / 2, 320, 100, nullptr,
					                                nullptr,
					                                host, nullptr);

					if (this->window_)
					{
						auto* const image_window = CreateWindowExA(0, "Static", nullptr, WS_CHILD | WS_VISIBLE | 0xEu,
						                                           0, 0,
						                                           320, 100, this->window_, nullptr, host, nullptr);
						if (image_window)
						{
							RECT rect;
							SendMessageA(image_window, 0x172u, 0, reinterpret_cast<LPARAM>(image_));
							GetWindowRect(image_window, &rect);

							const int width = rect.right - rect.left;
							rect.left = (x_pixels - width) / 2;

							const int height = rect.bottom - rect.top;
							rect.top = (y_pixels - height) / 2;

							rect.right = rect.left + width;
							rect.bottom = rect.top + height;
							AdjustWindowRect(&rect, WS_CHILD | WS_VISIBLE | 0xEu, 0);
							SetWindowPos(this->window_, nullptr, rect.left, rect.top, rect.right - rect.left,
							             rect.bottom - rect.top, SWP_NOZORDER);

							ShowWindow(this->window_, SW_SHOW);
							UpdateWindow(this->window_);
						}
					}
				}
			}
		}
	};
}

REGISTER_COMPONENT(splash::component)
