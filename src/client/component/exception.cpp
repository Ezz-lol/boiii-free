#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/thread.hpp>
#include <utils/compression.hpp>

#include <exception/minidump.hpp>

#include <version.hpp>

#include <DbgHelp.h>

namespace exception
{
	namespace
	{
		DWORD main_thread_id{};

		utils::hook::detour mini_dump_write_dump_hook;

		BOOL WINAPI mini_dump_write_dump_stub(const HANDLE h_process, const DWORD process_id, const HANDLE h_file,
		                                      const MINIDUMP_TYPE dump_type,
		                                      const PMINIDUMP_EXCEPTION_INFORMATION exception_param,
		                                      const PMINIDUMP_USER_STREAM_INFORMATION user_stream_param,
		                                      const PMINIDUMP_CALLBACK_INFORMATION callback_param)
		{
			wchar_t filename[MAX_PATH];
			if (GetFinalPathNameByHandleW(h_file, filename, ARRAYSIZE(filename), VOLUME_NAME_DOS))
			{
				std::wstring path = filename;
				if (path.find(L"\\\\?\\") == 0)
				{
					path = path.substr(4);
				}

				const std::filesystem::path p(path);
				if (p.extension() == L".dmp")
				{
					const auto root_path = game::get_game_path();
					const auto minidumps_path = root_path / "minidumps";
					std::filesystem::create_directories(minidumps_path);

					if (p.parent_path() == root_path)
					{
						const auto new_path = minidumps_path / p.filename();
						const auto new_handle = CreateFileW(new_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
						                                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

						if (new_handle != INVALID_HANDLE_VALUE)
						{
							const auto result = mini_dump_write_dump_hook.invoke<BOOL>(
								h_process, process_id, new_handle, dump_type, exception_param, user_stream_param,
								callback_param);
							CloseHandle(new_handle);
							return result;
						}
					}
				}
			}

			return mini_dump_write_dump_hook.invoke<BOOL>(h_process, process_id, h_file, dump_type, exception_param,
			                                              user_stream_param, callback_param);
		}

		thread_local struct
		{
			DWORD code = 0;
			PVOID address = nullptr;
		} exception_data{};

		struct
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> last_recovery{};
			std::atomic<int> recovery_counts = {0};
		} recovery_data{};

		bool is_game_thread()
		{
			return main_thread_id == GetCurrentThreadId();
		}

		bool is_exception_interval_too_short()
		{
			const auto delta = std::chrono::high_resolution_clock::now() - recovery_data.last_recovery;
			return delta < 1min;
		}

		bool too_many_exceptions_occured()
		{
			return recovery_data.recovery_counts >= 3;
		}

		volatile bool& is_initialized()
		{
			static volatile bool initialized = false;
			return initialized;
		}

		bool is_recoverable()
		{
			return is_initialized()
				&& is_game_thread()
				&& !is_exception_interval_too_short()
				&& !too_many_exceptions_occured();
		}

		void show_mouse_cursor()
		{
			while (ShowCursor(TRUE) < 0);
		}

		void display_error_dialog()
		{
			const std::string error_str = utils::string::va(
				"Fatal error (0x%08X) at 0x%p (0x%p).\n\n"
				"A crash dump has been saved to the 'minidumps' folder.\n\n"
				"Please report this crash and upload the dump file on our Discord:\n"
				"https://dc.ezz.lol\n",
				exception_data.code, exception_data.address,
				game::derelocate(reinterpret_cast<uint64_t>(exception_data.address)));

			utils::thread::suspend_other_threads();
			show_mouse_cursor();

			game::show_error(error_str.data(), "Ezz ERROR");

			const auto minidumps_path = game::get_game_path() / "minidumps";
			ShellExecuteA(nullptr, "open", minidumps_path.string().data(), nullptr, nullptr, SW_SHOWNORMAL);

			TerminateProcess(GetCurrentProcess(), exception_data.code);
		}

		void reset_state()
		{
			if (is_recoverable())
			{
				recovery_data.last_recovery = std::chrono::high_resolution_clock::now();
				++recovery_data.recovery_counts;

				game::Com_Error(game::ERR_DROP,
				                "Fatal error (0x%08X) at 0x%p (0x%p).\nA crash dump has been saved to the 'minidumps' folder.\n\n"
				                "Ezz has tried to recover your game, but it might not run stable anymore.\n\n"
				                "Make sure to update your graphics card drivers and install operating system updates!\n"
				                "Closing or restarting Steam might also help.\n\n"
				                "If this keeps happening, please report it on our Discord: https://dc.ezz.lol",
				                exception_data.code, exception_data.address,
				                game::derelocate(reinterpret_cast<uint64_t>(exception_data.address)))				;
			}
			else
			{
				display_error_dialog();
			}
		}

		size_t get_reset_state_stub()
		{
			static auto* stub = utils::hook::assemble([](utils::hook::assembler& a)
			{
				a.sub(rsp, 0x10);
				a.or_(rsp, 0x8);
				a.jmp(reset_state);
			});

			return reinterpret_cast<size_t>(stub);
		}

		std::string get_timestamp()
		{
			tm ltime{};
			char timestamp[MAX_PATH] = {0};
			const auto time = _time64(nullptr);

			_localtime64_s(&ltime, &time);
			strftime(timestamp, sizeof(timestamp) - 1, "%Y-%m-%d-%H-%M-%S", &ltime);

			return timestamp;
		}

		std::string generate_crash_info(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			std::string info{};
			const auto line = [&info](const std::string& text)
			{
				info.append(text);
				info.append("\r\n");
			};

			line("Ezz Crash Dump");
			line(std::string{});
			line("Version: "s + VERSION);
			line("Timestamp: "s + get_timestamp());
			line(utils::string::va("Exception: 0x%08X", exceptioninfo->ExceptionRecord->ExceptionCode));
			line(utils::string::va("Address: 0x%llX", exceptioninfo->ExceptionRecord->ExceptionAddress));
			line(utils::string::va("Base: 0x%llX", game::get_base()));

#pragma warning(push)
#pragma warning(disable: 4996)
			OSVERSIONINFOEXA version_info;
			ZeroMemory(&version_info, sizeof(version_info));
			version_info.dwOSVersionInfoSize = sizeof(version_info);
			GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&version_info));
#pragma warning(pop)

			line(utils::string::va("OS Version: %u.%u", version_info.dwMajorVersion, version_info.dwMinorVersion));

			return info;
		}

		void write_minidump(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			const auto minidumps_path = game::get_game_path() / "minidumps";
			const std::string crash_name = (minidumps_path / utils::string::va("ezz-crash-%s.zip", get_timestamp().data())).string();

			utils::compression::zip::archive zip_file{};
			zip_file.add("crash.dmp", create_minidump(exceptioninfo));
			zip_file.add("info.txt", generate_crash_info(exceptioninfo));
			if (!zip_file.write(crash_name, "Ezz Crash Dump"))
			{
				utils::io::remove_file(crash_name);
			}
		}

		// Empty string used as fallback for null localization pointers
		char ui_localize_fallback[4] = "";

		LONG WINAPI crash_fix_exception_handler(PEXCEPTION_POINTERS exception_info)
		{
			const auto* record = exception_info->ExceptionRecord;
			auto* context = exception_info->ContextRecord;

			if (record->ExceptionCode != EXCEPTION_ACCESS_VIOLATION &&
			    record->ExceptionCode != STATUS_ILLEGAL_INSTRUCTION)
			{
				return EXCEPTION_CONTINUE_SEARCH;
			}

			const auto addr = reinterpret_cast<uintptr_t>(record->ExceptionAddress);
			const auto base = game::get_base();
			const auto offset = addr - base;

			switch (offset)
			{
			// Killcam animation crash - invalid anim data access
			case 0x234B9BD:
				context->Rax = 0;
				context->Rip = base + 0x234D14B;
				return EXCEPTION_CONTINUE_EXECUTION;

			// CG_ZBarrierAttachWeapon - null weapon pointer in zombie barriers
			case 0x464FEF:
				context->Rax = 0;
				context->Rip = base + 0x4651A2;
				return EXCEPTION_CONTINUE_EXECUTION;

			// asmsetanimationrate - bad entity reference
			case 0x15E4B5A:
				context->Rip = base + 0x15E4B83;
				return EXCEPTION_CONTINUE_EXECUTION;

			// Orphaned thread crash
			case 0x12EE4CC:
				context->Rip = base + 0x12EE5C8;
				return EXCEPTION_CONTINUE_EXECUTION;

			// Character index out-of-bounds crash
			case 0x234210C:
				context->Rip = base + 0x2342136;
				return EXCEPTION_CONTINUE_EXECUTION;

			// HKS internal crash
			case 0x1CAB4F1:
				context->Rip = base + 0x1CAB69E;
				return EXCEPTION_CONTINUE_EXECUTION;

			// Null localization string crashes
			case 0x2279323:
				context->Rdx = reinterpret_cast<uintptr_t>(ui_localize_fallback);
				return EXCEPTION_CONTINUE_EXECUTION;

			case 0x2278B96:
				context->Rsi = reinterpret_cast<uintptr_t>(ui_localize_fallback);
				return EXCEPTION_CONTINUE_EXECUTION;

			case 0x228ED56:
				context->Rcx = reinterpret_cast<uintptr_t>(ui_localize_fallback);
				return EXCEPTION_CONTINUE_EXECUTION;

			// Unknown UI crash
			case 0x1EAAA27:
				context->Rip = base + 0x1EAABB3;
				return EXCEPTION_CONTINUE_EXECUTION;

			// Non-existent clientfield crashes (CSC side)
			case 0xC15B80:
			case 0xC15C50:
			case 0xC18CF5:
				context->Rcx = 1; // CSC instance
				context->Rdx = reinterpret_cast<uintptr_t>("Clientfield does not exist");
				context->R8 = 0;
				context->Rip = base + 0x12EA430; // Scr_Error
				return EXCEPTION_CONTINUE_EXECUTION;

			// Non-existent clientfield crashes (GSC side)
			case 0x1A6BD1B:
			case 0x1A6BE2E:
			case 0x1A6BF2E:
			case 0x1A6BFCD:
			case 0x1A6C246:
			case 0x1A6C356:
			case 0x1A6C40D:
			case 0x1A6C697:
			case 0x1A6C894:
				context->Rcx = 0; // GSC instance
				context->Rdx = reinterpret_cast<uintptr_t>("Clientfield does not exist");
				context->R8 = 0;
				context->Rip = base + 0x12EA430; // Scr_Error
				return EXCEPTION_CONTINUE_EXECUTION;

			// Non-existent clientfield (additional crash sites)
			case 0x133EC1:
			case 0x133EEB:
				context->Rip = base + 0x133F12;
				return EXCEPTION_CONTINUE_EXECUTION;

			case 0x133F31:
				context->Rip = base + 0x133F42;
				return EXCEPTION_CONTINUE_EXECUTION;

			// Random crash on Zetsubou No Shima
			case 0x13591D3:
				context->Rip = base + 0x13591DA;
				return EXCEPTION_CONTINUE_EXECUTION;

			default:
				break;
			}

			return EXCEPTION_CONTINUE_SEARCH;
		}

		bool is_harmless_error(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			const auto code = exceptioninfo->ExceptionRecord->ExceptionCode;
			return code == STATUS_INTEGER_OVERFLOW || code == STATUS_FLOAT_OVERFLOW || code == STATUS_SINGLE_STEP;
		}

		LONG WINAPI exception_filter(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			if (is_harmless_error(exceptioninfo))
			{
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			write_minidump(exceptioninfo);

			exception_data.code = exceptioninfo->ExceptionRecord->ExceptionCode;
			exception_data.address = exceptioninfo->ExceptionRecord->ExceptionAddress;
			exceptioninfo->ContextRecord->Rip = get_reset_state_stub();

			return EXCEPTION_CONTINUE_EXECUTION;
		}

		void WINAPI set_unhandled_exception_filter_stub(LPTOP_LEVEL_EXCEPTION_FILTER)
		{
			// Don't register anything here...
		}
	}

	struct component final : generic_component
	{
		component()
		{
			main_thread_id = GetCurrentThreadId();
			SetUnhandledExceptionFilter(exception_filter);

			const auto root_path = game::get_game_path();
			std::filesystem::create_directories(root_path / "minidumps");
		}

		void post_load() override
		{
			const utils::nt::library ntdll("ntdll.dll");
			auto* set_filter = ntdll.get_proc<void(*)(LPTOP_LEVEL_EXCEPTION_FILTER)>("RtlSetUnhandledExceptionFilter");

			set_filter(exception_filter);
			utils::hook::jump(set_filter, set_unhandled_exception_filter_stub);

			const auto dbghelp = utils::nt::library::load("dbghelp.dll");
			if (dbghelp)
			{
				mini_dump_write_dump_hook.create(dbghelp.get_proc<void*>("MiniDumpWriteDump"), mini_dump_write_dump_stub);
			}

			if (!game::is_server())
			{
				AddVectoredExceptionHandler(1, crash_fix_exception_handler);
			}
		}
	};
}

REGISTER_COMPONENT(exception::component)