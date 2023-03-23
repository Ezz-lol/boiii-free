#include <std_include.hpp>
#include "console.hpp"
#include "loader/component_loader.hpp"
#include "resource.hpp"

#include "game/game.hpp"
#include "scheduler.hpp"

#include <utils/thread.hpp>
#include <utils/hook.hpp>
#include <utils/flags.hpp>
#include <utils/concurrency.hpp>
#include <utils/image.hpp>

#define CONSOLE_BUFFER_SIZE 16384
#define WINDOW_WIDTH 608

namespace console
{
	namespace
	{
		utils::image::object logo;
		std::atomic_bool started{false};
		std::atomic_bool terminate_runner{false};
		utils::concurrency::container<std::function<void(const std::string& message)>> interceptor{};
		utils::concurrency::container<std::queue<std::string>> message_queue{};

		void print_message(const char* message)
		{
#ifndef NDEBUG
			OutputDebugStringA(message);
#endif

			if (started && !terminate_runner)
			{
				game::Com_Printf(0, 0, "%s", message);
			}
		}

		void queue_message(const char* message)
		{
			interceptor.access([message](const std::function<void(const std::string&)>& callback)
			{
				if (callback)
				{
					callback(message);
				}
			});

			message_queue.access([message](std::queue<std::string>& queue)
			{
				queue.push(message);
			});
		}

		bool is_headless()
		{
			static const auto headless = utils::flags::has_flag("headless");
			return headless;
		}

		void print_message_to_console(const char* message)
		{
			if (is_headless())
			{
				fputs(message, stdout);
				return;
			}

			static auto print_func = utils::hook::assemble([](utils::hook::assembler& a)
			{
				a.push(rbx);
				a.mov(eax, 0x8030);
				a.jmp(game::select(0x142332AA7, 0x140597527));
			});

			static_cast<void(*)(const char*)>(print_func)(message);
		}

		std::queue<std::string> empty_message_queue()
		{
			std::queue<std::string> current_queue{};

			message_queue.access([&](std::queue<std::string>& queue)
			{
				current_queue = std::move(queue);
				queue = {};
			});

			return current_queue;
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

		INT_PTR get_gray_brush()
		{
			static utils::image::object b(CreateSolidBrush(RGB(50, 50, 50)));
			return reinterpret_cast<INT_PTR>(b.get());
		}

		LRESULT con_wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wparam, const LPARAM lparam)
		{
			switch (msg)
			{
			case WM_CTLCOLOREDIT:
			case WM_CTLCOLORSTATIC:
				SetBkColor(reinterpret_cast<HDC>(wparam), RGB(50, 50, 50));
				SetTextColor(reinterpret_cast<HDC>(wparam), RGB(232, 230, 227));
				return get_gray_brush();
			case WM_CLOSE:
				game::Cbuf_AddText(0, "quit\n");
				[[fallthrough]];
			default:
				return utils::hook::invoke<LRESULT>(game::select(0x142332960, 0x1405973E0), hwnd, msg, wparam, lparam);
			}
		}

		LRESULT input_line_wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wparam, const LPARAM lparam)
		{
			return utils::hook::invoke<LRESULT>(game::select(0x142332C60, 0x1405976E0), hwnd, msg, wparam, lparam);
		}

		void sys_create_console_stub(const HINSTANCE h_instance)
		{
			if (is_headless())
			{
				return;
			}

			char text[CONSOLE_BUFFER_SIZE]{0};

			const auto* class_name = "BOIII WinConsole";
			const auto* window_name = game::is_server() ? "BOIII Server" : "BOIII Console";

			WNDCLASSA wnd_class{};
			wnd_class.style = 0;
			wnd_class.lpfnWndProc = con_wnd_proc;
			wnd_class.cbClsExtra = 0;
			wnd_class.cbWndExtra = 0;
			wnd_class.hInstance = h_instance;
			wnd_class.hIcon = LoadIconA(h_instance, reinterpret_cast<LPCSTR>(1));
			wnd_class.hCursor = LoadCursorA(nullptr, reinterpret_cast<LPCSTR>(0x7F00));
			wnd_class.hbrBackground = CreateSolidBrush(RGB(50, 50, 50));
			wnd_class.lpszMenuName = nullptr;
			wnd_class.lpszClassName = class_name;

			if (!RegisterClassA(&wnd_class))
			{
				return;
			}

			RECT rect{};
			rect.left = 0;
			rect.right = 620;
			rect.top = 0;
			rect.bottom = 450;
			AdjustWindowRect(&rect, 0x80CA0000, 0);

			auto dc = GetDC(GetDesktopWindow());
			const auto swidth = GetDeviceCaps(dc, 8);
			const auto sheight = GetDeviceCaps(dc, 10);
			ReleaseDC(GetDesktopWindow(), dc);

			utils::hook::set<int>(game::s_wcd::windowWidth, (rect.right - rect.left + 1));
			utils::hook::set<int>(game::s_wcd::windowHeight, (rect.bottom - rect.top + 1));

			utils::hook::set<HWND>(game::s_wcd::hWnd, CreateWindowExA(
				                       0, class_name, window_name, 0x80CA0000, (swidth - 600) / 2, (sheight - 450) / 2,
				                       rect.right - rect.left + 1, rect.bottom - rect.top + 1, nullptr, nullptr,
				                       h_instance, nullptr));

			if (!*game::s_wcd::hWnd)
			{
				return;
			}

			// create fonts
			dc = GetDC(*game::s_wcd::hWnd);
			const auto n_height = MulDiv(8, GetDeviceCaps(dc, 90), 72);

			utils::hook::set<HFONT>(game::s_wcd::hfBufferFont, CreateFontA(
				                        -n_height, 0, 0, 0, 300, 0, 0, 0, 1u, 0, 0, 0, 0x31u, "Courier New"));

			ReleaseDC(*game::s_wcd::hWnd, dc);

			if (logo)
			{
				utils::hook::set<HWND>(game::s_wcd::codLogo, CreateWindowExA(
					                       0, "Static", nullptr, 0x5000000Eu, 5, 5, 0, 0, *game::s_wcd::hWnd,
					                       reinterpret_cast<HMENU>(1), h_instance, nullptr));
				SendMessageA(*game::s_wcd::codLogo, STM_SETIMAGE, IMAGE_BITMAP, logo);
			}

			// create the input line
			utils::hook::set<HWND>(game::s_wcd::hwndInputLine, CreateWindowExA(
				                       0, "edit", nullptr, 0x50800080u, 6, 400, WINDOW_WIDTH, 20, *game::s_wcd::hWnd,
				                       reinterpret_cast<HMENU>(0x65), h_instance, nullptr));
			utils::hook::set<HWND>(game::s_wcd::hwndBuffer, CreateWindowExA(
				                       0, "edit", nullptr, 0x50A00844u, 6, 70, WINDOW_WIDTH, 324, *game::s_wcd::hWnd,
				                       reinterpret_cast<HMENU>(0x64), h_instance, nullptr));
			SendMessageA(*game::s_wcd::hwndBuffer, WM_SETFONT, reinterpret_cast<WPARAM>(*game::s_wcd::hfBufferFont), 0);

			utils::hook::set<WNDPROC>(game::s_wcd::SysInputLineWndProc, reinterpret_cast<WNDPROC>(SetWindowLongPtrA(
				                          *game::s_wcd::hwndInputLine, -4,
				                          reinterpret_cast<LONG_PTR>(input_line_wnd_proc))));
			SendMessageA(*game::s_wcd::hwndInputLine, WM_SETFONT, reinterpret_cast<WPARAM>(*game::s_wcd::hfBufferFont),
			             0);

			SetFocus(*game::s_wcd::hwndInputLine);
			game::Con_GetTextCopy(text, std::min(0x4000, static_cast<int>(sizeof(text))));
			SetWindowTextA(*game::s_wcd::hwndBuffer, text);
		}
	}

	void set_interceptor(std::function<void(const std::string& message)> callback)
	{
		interceptor.access([&callback](std::function<void(const std::string&)>& c)
		{
			c = std::move(callback);
		});
	}

	void remove_interceptor()
	{
		set_interceptor({});
	}

	void set_title(const std::string& title)
	{
		if (is_headless())
		{
			SetConsoleTitleA(title.data());
		}
		else
		{
			SetWindowTextA(*game::s_wcd::hWnd, title.data());
		}
	}

	struct component final : generic_component
	{
		component()
		{
			if (is_headless())
			{
				if (!AttachConsole(ATTACH_PARENT_PROCESS))
				{
					AllocConsole();
					AttachConsole(GetCurrentProcessId());
				}

				ShowWindow(GetConsoleWindow(), SW_SHOW);

				FILE* fp;
				freopen_s(&fp, "CONIN$", "r", stdin);
				freopen_s(&fp, "CONOUT$", "w", stdout);
				freopen_s(&fp, "CONOUT$", "w", stderr);
			}
		}

		void post_unpack() override
		{
			if (!game::is_server())
			{
				utils::hook::set<uint8_t>(0x14133D2FE_g, 0xEB); // Always enable ingame console

				if (utils::nt::is_wine() && !utils::flags::has_flag("console"))
				{
					return;
				}
			}

			utils::hook::jump(printf, print_stub);

			utils::hook::jump(game::select(0x142332C30, 0x1405976B0), queue_message);
			utils::hook::nop(game::select(0x142332C4A, 0x1405976CA), 2); // Print from every thread

			//const auto self = utils::nt::library::get_by_address(sys_create_console_stub);
			//logo = LoadImageA(self.get_handle(), MAKEINTRESOURCEA(IMAGE_LOGO), 0, 0, 0, LR_COPYFROMRESOURCE);

			const auto res = utils::nt::load_resource(IMAGE_LOGO);
			const auto img = utils::image::load_image(res);
			logo = utils::image::create_bitmap(img);

			terminate_runner = false;

			this->message_runner_ = utils::thread::create_named_thread("Console IO", []
			{
				while (!terminate_runner)
				{
					std::string message_buffer{};
					auto current_queue = empty_message_queue();

					while (!current_queue.empty())
					{
						const auto& msg = current_queue.front();
						message_buffer.append(msg);
						current_queue.pop();
					}

					if (!message_buffer.empty())
					{
						print_message_to_console(message_buffer.data());
					}

					std::this_thread::sleep_for(5ms);
				}
			});

			this->console_runner_ = utils::thread::create_named_thread("Console Window", [this]
			{
				{
					static utils::hook::detour sys_create_console_hook;
					sys_create_console_hook.create(game::select(0x142332E00, 0x140597880), sys_create_console_stub);

					game::Sys_ShowConsole();
					started = true;
				}

				MSG msg{};
				while (!terminate_runner)
				{
					if (PeekMessageW(&msg, nullptr, NULL, NULL, PM_REMOVE))
					{
						TranslateMessage(&msg);
						DispatchMessageW(&msg);
					}
					else
					{
						std::this_thread::sleep_for(5ms);
					}
				}
			});

			while (!started)
			{
				std::this_thread::sleep_for(10ms);
			}
		}

		void pre_destroy() override
		{
			terminate_runner = true;

			if (this->message_runner_.joinable())
			{
				this->message_runner_.join();
			}

			if (this->console_runner_.joinable())
			{
				this->console_runner_.join();
			}
		}

	private:
		std::thread console_runner_{};
		std::thread message_runner_{};
	};
}

REGISTER_COMPONENT(console::component)
