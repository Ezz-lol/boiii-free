#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "resource.hpp"

#include "game/game.hpp"

#include <utils/thread.hpp>
#include <utils/hook.hpp>

#define CONSOLE_BUFFER_SIZE		16384

namespace console
{
	namespace
	{
		volatile bool g_started = false;
		HANDLE logo;

		void print_message(const char* message)
		{
			if (g_started)
			{
				game::Com_Printf(0, 0, "%s", message);
			}
		}

		void execute_command(const char* command)
		{
			game::Cbuf_AddText(0, command);
		}

		void print_stub(const char* fmt, ...)
		{
			va_list ap;
			va_start(ap, fmt);

			char buffer[1024]{0};
			const int res = vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, ap);
			(void)res;
			print_message(buffer);

			va_end(ap);
		}

		LRESULT con_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
			case WM_CTLCOLORSTATIC:
				SetBkColor((HDC)wParam, RGB(50, 50, 50));
				SetTextColor((HDC)wParam, RGB(232, 230, 227));
				return (INT_PTR)CreateSolidBrush(RGB(50, 50, 50));
			case WM_CTLCOLOREDIT:
				SetBkColor((HDC)wParam, RGB(50, 50, 50));
				SetTextColor((HDC)wParam, RGB(232, 230, 227));
				return (INT_PTR)CreateSolidBrush(RGB(50, 50, 50));
			default:
				return utils::hook::invoke<LRESULT>(0x142333520_g, hWnd, uMsg, wParam, lParam);
			}
		}

		LRESULT input_line_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			return utils::hook::invoke<LRESULT>(0x142333820_g, hWnd, uMsg, wParam, lParam);
		}

		// cod naming, functionality stolen from https://github.com/id-Software/Quake-III-Arena/blob/master/code/win32/win_syscon.c#L520
		int conbuf_clean_text(const char* source, char* target)
		{
			char* b = target;
			int i = 0;
			while (source[i] && ((b - target) < sizeof(target) - 1))
			{
				if (source[i] == '\n' && source[i + 1] == '\r')
				{
					b[0] = '\r';
					b[1] = '\n';
					b += 2;
					i++;
				}
				else if (source[i] == '\r' || source[i] == '\n')
				{
					b[0] = '\r';
					b[1] = '\n';
					b += 2;
				}
				else if (source && source[0] == '^' && source[1] && source[1] != '^' && source[1] >= 48 && source[1] <= 64) // Q_IsColorString
				{
					i++;
				}
				else
				{
					*b = source[i];
					b++;
				}
				i++;
			}

			*b = 0;
			return static_cast<int>(b - target);
		}

		void sys_create_console_stub(HINSTANCE hInstance)
		{
			char text[CONSOLE_BUFFER_SIZE];
			char cleanConsoleBuffer[CONSOLE_BUFFER_SIZE];

			const auto* class_name = "BOIII WinConsole";
			const auto* window_name = "BOIII Console";

			WNDCLASSA WndClass;
			WndClass.style = 0;
			WndClass.lpfnWndProc = con_wnd_proc;
			WndClass.cbClsExtra = 0;
			WndClass.cbWndExtra = 0;
			WndClass.hInstance = hInstance;
			WndClass.hIcon = LoadIconA(hInstance, (LPCSTR)1);
			WndClass.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
			WndClass.hbrBackground = CreateSolidBrush(RGB(50, 50, 50));
			WndClass.lpszMenuName = 0;
			WndClass.lpszClassName = class_name;

			if (!RegisterClassA(&WndClass))
			{
				return;
			}

			tagRECT Rect;
			Rect.left = 0;
			Rect.right = 620;
			Rect.top = 0;
			Rect.bottom = 450;
			AdjustWindowRect(&Rect, 0x80CA0000, 0);

			auto hDC = GetDC(GetDesktopWindow());
			auto swidth = GetDeviceCaps(hDC, 8);
			auto sheight = GetDeviceCaps(hDC, 10);
			ReleaseDC(GetDesktopWindow(), hDC);

			utils::hook::set<int>(game::s_wcd::windowWidth, Rect.right - Rect.left + 1);
			utils::hook::set<int>(game::s_wcd::windowHeight, Rect.bottom - Rect.top + 1);

			utils::hook::set<HWND>(game::s_wcd::hWnd, CreateWindowExA(
				0, class_name, window_name, 0x80CA0000, (swidth - 600) / 2, (sheight - 450) / 2,
				Rect.right - Rect.left + 1, Rect.bottom - Rect.top + 1, 0, 0, hInstance, 0));

			if (!*game::s_wcd::hWnd)
			{
				return;
			}

			// create fonts
			hDC = GetDC(*game::s_wcd::hWnd);
			auto nHeight = MulDiv(8, GetDeviceCaps(hDC, 90), 72);

			utils::hook::set<HFONT>(game::s_wcd::hfBufferFont, CreateFontA(
				-nHeight, 0, 0, 0, 300, 0, 0, 0, 1u, 0, 0, 0, 0x31u, "Courier New"));

			ReleaseDC(*game::s_wcd::hWnd, hDC);

			if (logo)
			{
				utils::hook::set<HWND>(game::s_wcd::codLogo, CreateWindowExA(
					0, "Static", 0, 0x5000000Eu, 5, 5, 0, 0, *game::s_wcd::hWnd, (HMENU)1, hInstance, 0));
				SendMessageA(*game::s_wcd::codLogo, 0x172u, 0, (LPARAM)logo);
			}

			// create the input line
			utils::hook::set<HWND>(game::s_wcd::hwndInputLine, CreateWindowExA(
				0, "edit", 0, 0x50800080u, 6, 400, 608, 20, *game::s_wcd::hWnd, (HMENU)0x65, hInstance, 0));
			utils::hook::set<HWND>(game::s_wcd::hwndBuffer, CreateWindowExA(
				0, "edit", 0, 0x50A00844u, 6, 70, 606, 324, *game::s_wcd::hWnd, (HMENU)0x64, hInstance, 0));
			SendMessageA(*game::s_wcd::hwndBuffer, WM_SETFONT, (WPARAM)*game::s_wcd::hfBufferFont, 0);

			utils::hook::set<WNDPROC>(game::s_wcd::SysInputLineWndProc, (WNDPROC)SetWindowLongPtrA(
				*game::s_wcd::hwndInputLine, -4, (LONG_PTR)input_line_wnd_proc));
			SendMessageA(*game::s_wcd::hwndInputLine, WM_SETFONT, (WPARAM)*game::s_wcd::hfBufferFont, 0);

			SetFocus(*game::s_wcd::hwndInputLine);
			game::Con_GetTextCopy(text, 0x4000);
			conbuf_clean_text(text, cleanConsoleBuffer); // Conbuf_CleanText is inlined
			SetWindowTextA(*game::s_wcd::hwndBuffer, cleanConsoleBuffer);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			const utils::nt::library self;
			logo = LoadImageA(self.get_handle(), MAKEINTRESOURCEA(IMAGE_LOGO), 0, 0, 0, LR_COPYFROMRESOURCE);

			utils::hook::jump(printf, print_stub);

			this->terminate_runner_ = false;

			this->console_runner_ = utils::thread::create_named_thread("Console IO", [this]
			{
				{
					utils::hook::detour sys_create_console_hook;
					sys_create_console_hook.create(0x1423339C0_g, sys_create_console_stub);

					game::Sys_ShowConsole();
					g_started = true;
				}

				MSG msg{};
				while (!this->terminate_runner_)
				{
					if (PeekMessageW(&msg, nullptr, NULL, NULL, PM_REMOVE))
					{
						TranslateMessage(&msg);
						DispatchMessageW(&msg);

						if (msg.message == WM_QUIT)
						{
							execute_command("quit\n");
						}
					}
					else
					{
						std::this_thread::sleep_for(1ms);
					}
				}
			});
		}

		void pre_destroy() override
		{
			this->terminate_runner_ = true;

			if (this->console_runner_.joinable())
			{
				this->console_runner_.join();
			}
		}

	private:
		std::atomic_bool terminate_runner_{false};
		std::thread console_runner_;
	};
}

REGISTER_COMPONENT(console::component)
