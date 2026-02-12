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
			default: return RGB(232, 230, 227);
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

			if (contains_case_insensitive(line, "com_error:") || contains_case_insensitive(line, "unrecoverable error")
				||
				contains_case_insensitive(line, "script error"))
			{
				return get_error_color();
			}

			if (contains_case_insensitive(line, "ui error") || contains_case_insensitive(line, "unable to load module")
				||
				contains_case_insensitive(line, "stack traceback") || contains_case_insensitive(
					line, "attempt to index a nil value") ||
				contains_case_insensitive(line, "function expected instead of nil"))
			{
				return get_error_color();
			}

			if (contains_case_insensitive(line, "error") || contains_case_insensitive(line, "could not find") ||
				contains_case_insensitive(line, "exec from disk failed") || contains_case_insensitive(
					line, "invalid line") ||
				contains_case_insensitive(line, "missing asset") || contains_case_insensitive(line, "failed"))
			{
				return get_error_color();
			}

			if (contains_case_insensitive(line, "couldn't exec") || contains_case_insensitive(line, "failed to open") ||
				contains_case_insensitive(line, "tried to load asset") || contains_case_insensitive(
					line, "could not load default asset"))
			{
				return get_error_color();
			}

			if (contains_case_insensitive(line, "warning") || contains_case_insensitive(line, "hitch") ||
				contains_case_insensitive(line, "waited "))
			{
				return get_warning_color();
			}

			if (contains_case_insensitive(line, "unknown command") || contains_case_insensitive(line, "shutdowngame") ||
				contains_case_insensitive(line, "redundant ") || contains_case_insensitive(
					line, "invalid line in playlist"))
			{
				return get_warning_color();
			}

			if (contains_case_insensitive(line, "fastfileload") || contains_case_insensitive(line, "loading fastfile")
				||
				contains_case_insensitive(line, "added xpaks") || contains_case_insensitive(line, "loaded sounds") ||
				contains_case_insensitive(line, "loading sounds") || contains_case_insensitive(
					line, "sound adding bank") ||
				contains_case_insensitive(line, "sound load state") || contains_case_insensitive(line, "execing ") ||
				contains_case_insensitive(line, "unloading assets") || contains_case_insensitive(
					line, "unloaded fastfile"))
			{
				return get_info_color();
			}

			return get_default_console_color();
		}

		std::wstring to_wstring(const std::string& str)
		{
			if (str.empty())
			{
				return {};
			}

			const int required = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
			if (required <= 0)
			{
				const int required_ansi = MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()),
				                                              nullptr, 0);
				if (required_ansi <= 0)
				{
					return {};
				}

				std::wstring wstr(required_ansi, L'\0');
				MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), wstr.data(), required_ansi);
				return wstr;
			}

			std::wstring wstr(required, L'\0');
			MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), wstr.data(), required);
			return wstr;
		}

		void trim_console_scrollback(const HWND hwnd)
		{
			if (!utils::flags::has_flag("trimlogs"))
			{
				return;
			}

			const auto length = static_cast<size_t>(SendMessageA(hwnd, WM_GETTEXTLENGTH, 0, 0));
			if (length <= MAX_CONSOLE_CHARS)
			{
				return;
			}

			const LONG excess = static_cast<LONG>(length - MAX_CONSOLE_CHARS);
			CHARRANGE range{0, excess};
			SendMessageA(hwnd, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&range));
			SendMessageW(hwnd, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(L""));
		}

		bool is_scrolled_to_bottom(const HWND hwnd)
		{
			SCROLLINFO scroll_info{};
			scroll_info.cbSize = sizeof(scroll_info);
			scroll_info.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
			if (!GetScrollInfo(hwnd, SB_VERT, &scroll_info))
			{
				return true;
			}

			const auto max_pos = scroll_info.nMax - static_cast<int>(scroll_info.nPage);
			return scroll_info.nPos >= (max_pos - 2);
		}

		void append_colored_text(const HWND hwnd, const std::string& message, const COLORREF base_color)
		{
			if (!hwnd)
			{
				return;
			}

			const bool should_autoscroll = is_scrolled_to_bottom(hwnd);

			CHARRANGE end_range{-1, -1};
			SendMessageA(hwnd, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&end_range));

			COLORREF current_color = base_color;
			std::string current_segment{};

			auto flush_segment = [&]()
			{
				if (current_segment.empty())
				{
					return;
				}

				CHARFORMAT2A fmt{};
				fmt.cbSize = sizeof(fmt);
				fmt.dwMask = CFM_COLOR;
				fmt.crTextColor = current_color;
				SendMessageA(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&fmt));

				const auto wseg = to_wstring(current_segment);
				SendMessageW(hwnd, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(wseg.c_str()));
				current_segment.clear();
			};

			for (size_t i = 0; i < message.size(); ++i)
			{
				const char c = message[i];
				if (c == '^' && (i + 1) < message.size())
				{
					const char code = message[i + 1];
					if ((code >= '0' && code <= '9') || code == ':')
					{
						flush_segment();
						current_color = get_cod_color(code);
						++i;
						continue;
					}
				}

				current_segment.push_back(c);
			}

			flush_segment();
			trim_console_scrollback(hwnd);
			if (should_autoscroll)
			{
				SendMessageA(hwnd, EM_SCROLLCARET, 0, 0);
			}
		}

		bool is_separator_line(std::string_view line, const char sep_char)
		{
			size_t count = 0;
			for (const char c : line)
			{
				if (c == sep_char)
				{
					++count;
				}
				else if (c != '\r' && c != '\n' && c != ' ' && c != '\t')
				{
					return false;
				}
			}
			return count >= 10;
		}

		void load_dvar_list()
		{
			if (dvar_list_loaded.exchange(true))
			{
				return;
			}

			const auto path = game::get_appdata_path() / "data/lookup_tables/dvar_list.txt";
			std::string data;
			if (!utils::io::read_file(path, &data))
			{
				return;
			}

			std::erase(data, '\r');

			std::unordered_set<std::string> unique{};
			std::istringstream stream(data);
			std::string name;
			while (std::getline(stream, name, '\n'))
			{
				if (name.empty())
				{
					continue;
				}

				if (name.rfind("//", 0) == 0)
				{
					continue;
				}

				if (unique.emplace(name).second)
				{
					dvar_name_list.emplace_back(std::move(name));
				}
			}

			std::sort(dvar_name_list.begin(), dvar_name_list.end());
		}

		bool starts_with_case_insensitive(std::string_view str, std::string_view prefix)
		{
			if (prefix.empty() || str.size() < prefix.size())
			{
				return false;
			}

			for (size_t i = 0; i < prefix.size(); ++i)
			{
				const auto a = static_cast<unsigned char>(str[i]);
				const auto b = static_cast<unsigned char>(prefix[i]);
				const char ca = (a >= 'A' && a <= 'Z') ? static_cast<char>(a + 32) : static_cast<char>(a);
				const char cb = (b >= 'A' && b <= 'Z') ? static_cast<char>(b + 32) : static_cast<char>(b);
				if (ca != cb)
				{
					return false;
				}
			}

			return true;
		}

		bool try_autocomplete_dvar(const HWND input_hwnd)
		{
			load_dvar_list();
			if (dvar_name_list.empty())
			{
				return false;
			}

			const int text_len = GetWindowTextLengthA(input_hwnd);
			std::string text(static_cast<size_t>(text_len), '\0');
			if (text_len > 0)
			{
				GetWindowTextA(input_hwnd, text.data(), text_len + 1);
			}

			DWORD sel_start = 0;
			DWORD sel_end = 0;
			SendMessageA(input_hwnd, EM_GETSEL, reinterpret_cast<WPARAM>(&sel_start),
			             reinterpret_cast<LPARAM>(&sel_end));
			const size_t caret = sel_end;
			if (caret > text.size())
			{
				return false;
			}

			size_t token_start = caret;
			while (token_start > 0)
			{
				const char c = text[token_start - 1];
				if (c == ' ' || c == '\t')
				{
					break;
				}
				--token_start;
			}

			const std::string_view prefix(text.data() + token_start, caret - token_start);
			if (prefix.empty())
			{
				return false;
			}

			static std::string last_prefix{};
			static std::vector<std::string_view> last_matches{};
			static size_t last_index = 0;

			if (last_prefix != prefix)
			{
				last_prefix.assign(prefix);
				last_matches.clear();
				last_index = 0;

				for (const auto& n : dvar_name_list)
				{
					if (starts_with_case_insensitive(n, prefix))
					{
						last_matches.emplace_back(n);
						if (last_matches.size() >= 64)
						{
							break;
						}
					}
				}
			}
			else
			{
				if (!last_matches.empty())
				{
					last_index = (last_index + 1) % last_matches.size();
				}
			}

			if (last_matches.empty())
			{
				return false;
			}

			const std::string_view match = last_matches[last_index];
			std::string new_text;
			new_text.reserve(text.size() - prefix.size() + match.size());
			new_text.append(text.begin(), text.begin() + static_cast<std::ptrdiff_t>(token_start));
			new_text.append(match.begin(), match.end());
			new_text.append(text.begin() + static_cast<std::ptrdiff_t>(caret), text.end());

			SetWindowTextA(input_hwnd, new_text.c_str());
			const auto new_caret = token_start + match.size();
			SendMessageA(input_hwnd, EM_SETSEL, new_caret, new_caret);
			return true;
		}

		void append_text_with_severity(const HWND hwnd, const std::string& message)
		{
			static bool in_fatal_block = false;
			static int fatal_sep_count = 0;

			size_t start = 0;
			while (start < message.size())
			{
				size_t end = message.find('\n', start);
				const bool has_newline = (end != std::string::npos);
				if (!has_newline)
				{
					end = message.size();
				}

				std::string_view line_view(message.data() + start, end - start);
				std::string line(line_view);
				if (has_newline)
				{
					line.push_back('\n');
				}

				const bool is_eq_sep = is_separator_line(line_view, '=');
				const bool is_star_sep = is_separator_line(line_view, '*');
				const bool is_any_sep = (is_eq_sep || is_star_sep);

				if (contains_case_insensitive(line_view, "com_error:") || contains_case_insensitive(
						line_view, "unrecoverable error") ||
					contains_case_insensitive(line_view, "script error") || contains_case_insensitive(
						line_view, "script execution error") ||
					contains_case_insensitive(line_view, "lui script execution error"))
				{
					in_fatal_block = true;
					fatal_sep_count = 0;
				}

				const COLORREF base_color = (in_fatal_block || is_any_sep)
					                            ? get_error_color()
					                            : get_line_base_color(line_view);
				append_colored_text(hwnd, line, base_color);

				if (in_fatal_block && is_any_sep)
				{
					++fatal_sep_count;
					if (fatal_sep_count >= 2)
					{
						in_fatal_block = false;
						fatal_sep_count = 0;
					}
				}
				start = has_newline ? (end + 1) : end;
			}
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

		void print_message_to_console(const char* message)
		{
			if (game::is_headless())
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
				auto bmp = reinterpret_cast<HBITMAP>(
					SendMessageA(*game::s_wcd::codLogo, STM_GETIMAGE, IMAGE_BITMAP, 0));
				BITMAP bm{};
				if (bmp && GetObjectA(bmp, sizeof(bm), &bm) == sizeof(bm))
				{
					const int desired_w = bm.bmWidth;
					const int desired_h = bm.bmHeight;
					const int x = (std::max)(margin, margin + (client_width - desired_w) / 2);
					constexpr int y = 6;
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
				                       (swidth - (rect.right - rect.left + 1)) / 2,
				                       (sheight - (rect.bottom - rect.top + 1)) / 2,
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
				                       WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY
				                       | ES_NOHIDESEL,
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
