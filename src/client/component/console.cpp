#include <std_include.hpp>
#include "console.hpp"
#include "loader/component_loader.hpp"
#include "resource.hpp"

#include "game/game.hpp"

#include <utils/thread.hpp>
#include <utils/hook.hpp>
#include <utils/flags.hpp>
#include <utils/concurrency.hpp>
#include <utils/image.hpp>

#include <utils/io.hpp>

#include <Richedit.h>

#include <algorithm>
#include <sstream>
#include <unordered_set>

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
		std::vector<std::string> dvar_name_list{};
		std::atomic_bool dvar_list_loaded{false};

		constexpr UINT WM_APPEND_CONSOLE_TEXT = WM_APP + 0x1337;
		constexpr size_t MAX_CONSOLE_CHARS = 1'000'000;

		COLORREF get_cod_color(const char code)
		{
			switch (code)
			{
			case '0': return RGB(0, 0, 0);
			case '1': return RGB(255, 0, 0);
			case '2': return RGB(0, 255, 0);
			case '3': return RGB(255, 255, 0);
			case '4': return RGB(0, 0, 255);
			case '5': return RGB(0, 255, 255);
			case '6': return RGB(255, 0, 255);
			case '7': return RGB(255, 255, 255);
			case '8': return RGB(255, 165, 0);
			case '9': return RGB(128, 128, 128);
			default:  return RGB(232, 230, 227);
			}
		}

		COLORREF get_error_color()
		{
			return RGB(220, 90, 90);
		}

		COLORREF get_warning_color()
		{
			return RGB(220, 190, 90);
		}

		COLORREF get_info_color()
		{
			return RGB(185, 185, 185);
		}

		COLORREF get_bracket_tag_color()
		{
			return RGB(120, 170, 220);
		}

		COLORREF get_default_console_color()
		{
			return RGB(232, 230, 227);
		}

		bool contains_case_insensitive(std::string_view haystack, std::string_view needle)
		{
			if (needle.empty() || haystack.size() < needle.size())
			{
				return false;
			}

			for (size_t i = 0; i + needle.size() <= haystack.size(); ++i)
			{
				bool match = true;
				for (size_t j = 0; j < needle.size(); ++j)
				{
					const auto a = static_cast<unsigned char>(haystack[i + j]);
					const auto b = static_cast<unsigned char>(needle[j]);
					const char ca = (a >= 'A' && a <= 'Z') ? static_cast<char>(a + 32) : static_cast<char>(a);
					const char cb = (b >= 'A' && b <= 'Z') ? static_cast<char>(b + 32) : static_cast<char>(b);
					if (ca != cb)
					{
						match = false;
						break;
					}
				}
				if (match)
				{
					return true;
				}
			}

			return false;
		}

		COLORREF get_line_base_color(const std::string_view line)
		{
			if (!line.empty() && line[0] == '[' && line.find('^') == std::string_view::npos)
			{
				return get_bracket_tag_color();
			}

			if (!line.empty() && line[0] == ']')
			{
				return RGB(245, 242, 240);
			}

			if (contains_case_insensitive(line, "com_error:") || contains_case_insensitive(line, "unrecoverable error") ||
				contains_case_insensitive(line, "script error"))
			{
				return get_error_color();
			}

			if (contains_case_insensitive(line, "ui error") || contains_case_insensitive(line, "unable to load module") ||
				contains_case_insensitive(line, "stack traceback") || contains_case_insensitive(line, "attempt to index a nil value") ||
				contains_case_insensitive(line, "function expected instead of nil"))
			{
				return get_error_color();
			}

			if (contains_case_insensitive(line, "error") || contains_case_insensitive(line, "could not find") ||
				contains_case_insensitive(line, "exec from disk failed") || contains_case_insensitive(line, "invalid line") ||
				contains_case_insensitive(line, "missing asset") || contains_case_insensitive(line, "failed"))
			{
				return get_error_color();
			}

			if (contains_case_insensitive(line, "couldn't exec") || contains_case_insensitive(line, "failed to open") ||
				contains_case_insensitive(line, "tried to load asset") || contains_case_insensitive(line, "could not load default asset"))
			{
				return get_error_color();
			}

			if (contains_case_insensitive(line, "warn"))
			{
				return get_warning_color();
			}

			if (contains_case_insensitive(line, "loading") || contains_case_insensitive(line, "loaded") ||
				contains_case_insensitive(line, "connecting") || contains_case_insensitive(line, "connected"))
			{
				return get_info_color();
			}

			return get_default_console_color();
		}

		void append_colored_text(const HWND richedit, const char* text, size_t len, COLORREF color)
		{
			if (!richedit || !text || len == 0)
			{
				return;
			}

			const int wlen = MultiByteToWideChar(CP_UTF8, 0, text, static_cast<int>(len), nullptr, 0);
			if (wlen <= 0)
			{
				return;
			}

			std::vector<wchar_t> wbuf(static_cast<size_t>(wlen) + 1);
			MultiByteToWideChar(CP_UTF8, 0, text, static_cast<int>(len), wbuf.data(), wlen);
			wbuf[static_cast<size_t>(wlen)] = L'\0';

			CHARRANGE cr_end;
			cr_end.cpMin = -1;
			cr_end.cpMax = -1;
			SendMessageW(richedit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&cr_end));

			CHARFORMAT2W cf{};
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_COLOR;
			cf.crTextColor = color;
			cf.dwEffects = 0;
			SendMessageW(richedit, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&cf));
			SendMessageW(richedit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(wbuf.data()));
		}

		void trim_console_buffer(const HWND richedit)
		{
			const auto text_len = static_cast<size_t>(GetWindowTextLengthW(richedit));
			if (text_len > MAX_CONSOLE_CHARS)
			{
				const auto to_remove = static_cast<LONG>(text_len - MAX_CONSOLE_CHARS / 2);
				CHARRANGE cr;
				cr.cpMin = 0;
				cr.cpMax = to_remove;
				SendMessageW(richedit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&cr));
				SendMessageW(richedit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(L""));
			}
		}

		void append_line_colored(const HWND richedit, const std::string_view line, COLORREF base_color)
		{
			size_t i = 0;
			while (i < line.size())
			{
				if (i + 1 < line.size() && line[i] == '^' && line[i + 1] >= '0' && line[i + 1] <= '9')
				{
					const COLORREF cod_color = get_cod_color(line[i + 1]);
					i += 2;
					const size_t start = i;
					while (i < line.size())
					{
						if (i + 1 < line.size() && line[i] == '^' && line[i + 1] >= '0' && line[i + 1] <= '9')
						{
							break;
						}
						if (line[i] == '\n')
						{
							break;
						}
						i++;
					}
					if (i > start)
					{
						append_colored_text(richedit, line.data() + start, i - start, cod_color);
					}
				}
				else
				{
					const size_t start = i;
					while (i < line.size())
					{
						if (i + 1 < line.size() && line[i] == '^' && line[i + 1] >= '0' && line[i + 1] <= '9')
						{
							break;
						}
						if (line[i] == '\n')
						{
							i++;
							break;
						}
						i++;
					}
					append_colored_text(richedit, line.data() + start, i - start, base_color);
				}
			}
		}

		void append_text_with_severity(const HWND richedit, const std::string& text)
		{
			if (!richedit || text.empty())
			{
				return;
			}

			SendMessageW(richedit, WM_SETREDRAW, FALSE, 0);

			CHARRANGE old_sel;
			SendMessageW(richedit, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&old_sel));

			SCROLLINFO scroll_info{};
			scroll_info.cbSize = sizeof(scroll_info);
			scroll_info.fMask = SIF_ALL;
			GetScrollInfo(richedit, SB_VERT, &scroll_info);
			const bool was_at_bottom = (scroll_info.nPos + static_cast<int>(scroll_info.nPage) >= scroll_info.nMax - 1) || scroll_info.nMax == 0;

			std::string_view remaining(text);
			while (!remaining.empty())
			{
				const auto nl = remaining.find('\n');
				std::string_view line_view;
				if (nl != std::string_view::npos)
				{
					line_view = remaining.substr(0, nl + 1);
					remaining.remove_prefix(nl + 1);
				}
				else
				{
					line_view = remaining;
					remaining = {};
				}

				const COLORREF base_color = get_line_base_color(line_view);
				append_line_colored(richedit, line_view, base_color);
			}

			trim_console_buffer(richedit);

			if (was_at_bottom)
			{
				SendMessageW(richedit, WM_VSCROLL, SB_BOTTOM, 0);
			}

			SendMessageW(richedit, WM_SETREDRAW, TRUE, 0);
			InvalidateRect(richedit, nullptr, FALSE);
		}

		void load_dvar_list()
		{
			std::thread([]()
			{
				try
				{
					std::string data;
					if (utils::io::read_file("data/lookup_tables/dvar_list.txt", &data))
					{
						std::istringstream iss(data);
						std::string line;
						std::unordered_set<std::string> seen;
						while (std::getline(iss, line))
						{
							while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
								line.pop_back();
							if (!line.empty() && seen.insert(line).second)
								dvar_name_list.push_back(line);
						}
						std::sort(dvar_name_list.begin(), dvar_name_list.end(),
							[](const std::string& a, const std::string& b)
							{
								return _stricmp(a.c_str(), b.c_str()) < 0;
							});
						dvar_list_loaded = true;
					}
				}
				catch (...)
				{
				}
			}).detach();
		}

		bool try_autocomplete_dvar(const HWND input_hwnd)
		{
			if (!dvar_list_loaded || dvar_name_list.empty())
			{
				return false;
			}

			char buf[512]{};
			GetWindowTextA(input_hwnd, buf, sizeof(buf));
			std::string partial(buf);
			while (!partial.empty() && (partial.back() == ' ' || partial.back() == '\t'))
				partial.pop_back();
			if (partial.empty())
			{
				return false;
			}

			std::vector<std::string*> matches;
			for (auto& dvar : dvar_name_list)
			{
				if (_strnicmp(dvar.c_str(), partial.c_str(), partial.size()) == 0)
				{
					matches.push_back(&dvar);
					if (matches.size() > 50)
					{
						break;
					}
				}
			}

			if (matches.empty())
			{
				return false;
			}

			if (matches.size() == 1)
			{
				SetWindowTextA(input_hwnd, matches[0]->c_str());
				SendMessageA(input_hwnd, EM_SETSEL, static_cast<WPARAM>(matches[0]->size()), static_cast<LPARAM>(matches[0]->size()));
				return true;
			}

			size_t common_len = partial.size();
			for (; common_len < matches[0]->size(); ++common_len)
			{
				const char c = (*matches[0])[common_len];
				bool all_match = true;
				for (size_t i = 1; i < matches.size(); ++i)
				{
					if (common_len >= matches[i]->size() ||
						static_cast<char>(std::tolower(static_cast<unsigned char>((*matches[i])[common_len]))) !=
						static_cast<char>(std::tolower(static_cast<unsigned char>(c))))
					{
						all_match = false;
						break;
					}
				}
				if (!all_match)
				{
					break;
				}
			}

			if (common_len > partial.size())
			{
				std::string completed = matches[0]->substr(0, common_len);
				SetWindowTextA(input_hwnd, completed.c_str());
				SendMessageA(input_hwnd, EM_SETSEL, static_cast<WPARAM>(completed.size()), static_cast<LPARAM>(completed.size()));
			}

			std::string hint = "\n";
			for (size_t i = 0; i < matches.size() && i < 20; ++i)
			{
				hint += "  " + *matches[i] + "\n";
			}
			if (matches.size() > 20)
			{
				hint += "  ... (" + std::to_string(matches.size() - 20) + " more)\n";
			}

			if (*game::s_wcd::hwndBuffer)
			{
				append_text_with_severity(*game::s_wcd::hwndBuffer, hint);
			}

			return true;
		}

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

		void resize_console_controls(const HWND hwnd)
		{
			if (!hwnd || !*game::s_wcd::hwndBuffer || !*game::s_wcd::hwndInputLine)
			{
				return;
			}

			RECT rect{};
			GetClientRect(hwnd, &rect);

			constexpr int margin = 6;
			constexpr int top_offset = 70;
			constexpr int input_height = 24;
			const int client_width = (std::max)(0, static_cast<int>((rect.right - rect.left) - margin * 2));
			const int client_height = (std::max)(0, static_cast<int>((rect.bottom - rect.top)));
			const int input_y = (std::max)(top_offset, client_height - input_height - margin);

			MoveWindow(*game::s_wcd::hwndBuffer, margin, top_offset, client_width,
				(std::max)(0, input_y - top_offset - margin), TRUE);
			MoveWindow(*game::s_wcd::hwndInputLine, margin, input_y, client_width, input_height, TRUE);

			if (*game::s_wcd::codLogo)
			{
				HBITMAP bmp = reinterpret_cast<HBITMAP>(SendMessageA(*game::s_wcd::codLogo, STM_GETIMAGE, IMAGE_BITMAP, 0));
				BITMAP bm{};
				if (bmp && GetObjectA(bmp, sizeof(bm), &bm) == sizeof(bm))
				{
					const int desired_w = bm.bmWidth;
					const int desired_h = bm.bmHeight;
					const int x = (std::max)(margin, margin + (client_width - desired_w) / 2);
					const int y = 6;
					MoveWindow(*game::s_wcd::codLogo, x, y, desired_w, desired_h, TRUE);
				}
			}
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
			case WM_SIZE:
				resize_console_controls(hwnd);
				return 0;
			case WM_APPEND_CONSOLE_TEXT:
			{
				const auto* text = reinterpret_cast<std::string*>(lparam);
				if (text)
				{
					append_text_with_severity(*game::s_wcd::hwndBuffer, *text);
					delete text;
				}
				return 0;
			}
			case WM_CLOSE:
				game::Cbuf_AddText(0, "quit\n");
				[[fallthrough]];
			default:
				return utils::hook::invoke<LRESULT>(game::select(0x142332960, 0x1405973E0), hwnd, msg, wparam, lparam);
			}
		}

		LRESULT input_line_wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wparam, const LPARAM lparam)
		{
			if (msg == WM_KEYDOWN && wparam == VK_TAB)
			{
				if (try_autocomplete_dvar(hwnd))
				{
					return 0;
				}
			}

			return utils::hook::invoke<LRESULT>(game::select(0x142332C60, 0x1405976E0), hwnd, msg, wparam, lparam);
		}

		static utils::hook::detour sys_show_console_hook;
		static std::atomic_bool console_shown_once{false};

		void sys_show_console_stub()
		{
			// First call: let the original run to properly initialize the window
			if (!console_shown_once.exchange(true))
			{
				sys_show_console_hook.invoke<void>();
				return;
			}

			// Subsequent calls (e.g. on error): just show without resizing
			if (*game::s_wcd::hWnd)
			{
				ShowWindow(*game::s_wcd::hWnd, SW_SHOW);
				SetForegroundWindow(*game::s_wcd::hWnd);
				if (*game::s_wcd::hwndInputLine)
					SetFocus(*game::s_wcd::hwndInputLine);
			}
		}

		void sys_create_console_stub(const HINSTANCE h_instance)
		{
			if (game::is_headless())
			{
				return;
			}

			LoadLibraryA("Msftedit.dll");

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
			constexpr DWORD window_style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
			AdjustWindowRect(&rect, window_style, FALSE);

			auto dc = GetDC(GetDesktopWindow());
			const auto swidth = GetDeviceCaps(dc, 8);
			const auto sheight = GetDeviceCaps(dc, 10);
			ReleaseDC(GetDesktopWindow(), dc);

			utils::hook::set<int>(game::s_wcd::windowWidth, (rect.right - rect.left + 1));
			utils::hook::set<int>(game::s_wcd::windowHeight, (rect.bottom - rect.top + 1));

			utils::hook::set<HWND>(game::s_wcd::hWnd, CreateWindowExA(
				0, class_name, window_name, window_style,
				(swidth - (rect.right - rect.left + 1)) / 2, (sheight - (rect.bottom - rect.top + 1)) / 2,
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

			utils::hook::set<HWND>(game::s_wcd::hwndInputLine, CreateWindowExA(
				0, "edit", nullptr, 0x50800080u, 6, 500, WINDOW_WIDTH, 24, *game::s_wcd::hWnd,
				reinterpret_cast<HMENU>(0x65), h_instance, nullptr));
			utils::hook::set<HWND>(game::s_wcd::hwndBuffer, CreateWindowExW(
				0, L"RICHEDIT50W", nullptr,
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | ES_NOHIDESEL,
				6, 70, WINDOW_WIDTH, 420, *game::s_wcd::hWnd,
				reinterpret_cast<HMENU>(0x64), h_instance, nullptr));
			SendMessageA(*game::s_wcd::hwndBuffer, WM_SETFONT, reinterpret_cast<WPARAM>(*game::s_wcd::hfBufferFont), 0);
			SendMessageA(*game::s_wcd::hwndBuffer, EM_SETBKGNDCOLOR, 0, RGB(50, 50, 50));
			SendMessageA(*game::s_wcd::hwndBuffer, EM_SETLIMITTEXT, 0, 0);

			utils::hook::set<WNDPROC>(game::s_wcd::SysInputLineWndProc, reinterpret_cast<WNDPROC>(SetWindowLongPtrA(
				*game::s_wcd::hwndInputLine, -4,
				reinterpret_cast<LONG_PTR>(input_line_wnd_proc))));
			SendMessageA(*game::s_wcd::hwndInputLine, WM_SETFONT, reinterpret_cast<WPARAM>(*game::s_wcd::hfBufferFont),
				0);

			SetFocus(*game::s_wcd::hwndInputLine);
			game::Con_GetTextCopy(text, std::min(0x4000, static_cast<int>(sizeof(text))));
			append_text_with_severity(*game::s_wcd::hwndBuffer, text);
			resize_console_controls(*game::s_wcd::hWnd);
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
		if (game::is_headless())
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
			if (game::is_headless())
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
			// nologs flag: skip console window entirely
			if (utils::flags::has_flag("nologs"))
			{
				return;
			}

			if (!game::is_server())
			{
				utils::hook::set<uint8_t>(0x14133D2FE_g, 0xEB); // Always enable ingame console
				utils::hook::jump(0x141344E44_g, 0x141344E2E_g);
				// Remove the need to type '\' or '/' to send a console command

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
			load_dvar_list();

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
						if (game::is_headless())
						{
							fputs(message_buffer.data(), stdout);
						}
						else if (*game::s_wcd::hWnd)
						{
							PostMessageA(*game::s_wcd::hWnd, WM_APPEND_CONSOLE_TEXT, 0,
								reinterpret_cast<LPARAM>(new std::string(std::move(message_buffer))));
						}
					}

					std::this_thread::sleep_for(5ms);
				}
			});

			this->console_runner_ = utils::thread::create_named_thread("Console Window", [this]
			{
				{
					static utils::hook::detour sys_create_console_hook;
					sys_create_console_hook.create(game::select(0x142332E00, 0x140597880), sys_create_console_stub);

					sys_show_console_hook.create(game::Sys_ShowConsole, sys_show_console_stub);

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
