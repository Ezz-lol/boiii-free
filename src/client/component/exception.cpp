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
					const auto root_path = utils::nt::library{}.get_path().parent_path();
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
			static volatile bool initialized = true;
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

			ShellExecuteA(nullptr, "open", "minidumps", nullptr, nullptr, SW_SHOWNORMAL);

			TerminateProcess(GetCurrentProcess(), exception_data.code);
		}

		void reset_state()
		{
			printf("[Exception] reset_state called: code=0x%08X addr=0x%llX game_thread=%d recoverable=%d\n",
				exception_data.code,
				reinterpret_cast<uint64_t>(exception_data.address),
				is_game_thread() ? 1 : 0, is_recoverable() ? 1 : 0);

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

		const char* get_exception_string(const DWORD exception)
		{
#define EXCEPTION_CASE(code) case EXCEPTION_##code: return "EXCEPTION_" #code
			switch (exception)
			{
				EXCEPTION_CASE(ACCESS_VIOLATION);
				EXCEPTION_CASE(DATATYPE_MISALIGNMENT);
				EXCEPTION_CASE(BREAKPOINT);
				EXCEPTION_CASE(SINGLE_STEP);
				EXCEPTION_CASE(ARRAY_BOUNDS_EXCEEDED);
				EXCEPTION_CASE(FLT_DENORMAL_OPERAND);
				EXCEPTION_CASE(FLT_DIVIDE_BY_ZERO);
				EXCEPTION_CASE(FLT_INEXACT_RESULT);
				EXCEPTION_CASE(FLT_INVALID_OPERATION);
				EXCEPTION_CASE(FLT_OVERFLOW);
				EXCEPTION_CASE(FLT_STACK_CHECK);
				EXCEPTION_CASE(FLT_UNDERFLOW);
				EXCEPTION_CASE(INT_DIVIDE_BY_ZERO);
				EXCEPTION_CASE(INT_OVERFLOW);
				EXCEPTION_CASE(PRIV_INSTRUCTION);
				EXCEPTION_CASE(IN_PAGE_ERROR);
				EXCEPTION_CASE(ILLEGAL_INSTRUCTION);
				EXCEPTION_CASE(NONCONTINUABLE_EXCEPTION);
				EXCEPTION_CASE(STACK_OVERFLOW);
				EXCEPTION_CASE(INVALID_DISPOSITION);
				EXCEPTION_CASE(GUARD_PAGE);
				EXCEPTION_CASE(INVALID_HANDLE);
			default:
				return "UNKNOWN";
			}
#undef EXCEPTION_CASE
		}

		std::string get_memory_registers(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			if (!exceptioninfo || !exceptioninfo->ContextRecord)
			{
				return {};
			}

			const auto* ctx = exceptioninfo->ContextRecord;
			std::string info{"registers:\r\n{\r\n"};

			const auto reg = [&info](const char* key, const DWORD64 value)
			{
				info.append(utils::string::va("\t%s = 0x%llX\r\n", key, value));
			};

			reg("rax", ctx->Rax);
			reg("rbx", ctx->Rbx);
			reg("rcx", ctx->Rcx);
			reg("rdx", ctx->Rdx);
			reg("rsp", ctx->Rsp);
			reg("rbp", ctx->Rbp);
			reg("rsi", ctx->Rsi);
			reg("rdi", ctx->Rdi);
			reg("r8", ctx->R8);
			reg("r9", ctx->R9);
			reg("r10", ctx->R10);
			reg("r11", ctx->R11);
			reg("r12", ctx->R12);
			reg("r13", ctx->R13);
			reg("r14", ctx->R14);
			reg("r15", ctx->R15);
			reg("rip", ctx->Rip);

			info.append("}");
			return info;
		}

		std::string get_callstack_summary(void* exception_addr, int trace_depth = 32)
		{
			std::string info{"callstack:\r\n{\r\n"};

			void* backtrace_stack[32]{};
			if (trace_depth > static_cast<int>(ARRAYSIZE(backtrace_stack)))
			{
				trace_depth = ARRAYSIZE(backtrace_stack);
			}

			const auto count = RtlCaptureStackBackTrace(0, trace_depth, backtrace_stack, nullptr);
			auto* start = backtrace_stack;
			auto* end = backtrace_stack + count;
			const auto itr = std::find(start, end, exception_addr);
			const auto exception_start_index = (itr == end) ? 0 : static_cast<size_t>(std::distance(start, itr));

			for (size_t i = exception_start_index; i < count; ++i)
			{
				const auto from = utils::nt::library::get_by_address(backtrace_stack[i]);
				const auto address = reinterpret_cast<uint64_t>(backtrace_stack[i]);
				const auto base = reinterpret_cast<uint64_t>(from.get_ptr());
				uint64_t rva = (from && address >= base) ? (address - base) : address;

				if (from.get_name() == "BlackOps3.exe")
				{
					rva += 0x140000000;
				}

				info.append(utils::string::va("\t%s: %012llX\r\n", from.get_name().c_str(), rva));
			}

			info.append("}");
			return info;
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
			line(utils::string::va("Exception: 0x%08X (%s)", exceptioninfo->ExceptionRecord->ExceptionCode,
			                      get_exception_string(exceptioninfo->ExceptionRecord->ExceptionCode)));
			line(utils::string::va("Address: 0x%llX [%s]", exceptioninfo->ExceptionRecord->ExceptionAddress,
			                      utils::nt::library::get_by_address(exceptioninfo->ExceptionRecord->ExceptionAddress).get_name().c_str()));
			line(utils::string::va("Base: 0x%llX", game::get_base()));
			line(utils::string::va("Thread ID: %lu (%s)", GetCurrentThreadId(), is_game_thread() ? "main" : "auxiliary"));      

			if (exceptioninfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
			{
				line(utils::string::va("Extended Info: Attempted to %s 0x%012llX",
					exceptioninfo->ExceptionRecord->ExceptionInformation[0] == 1 ? "write to" : "read from",
					exceptioninfo->ExceptionRecord->ExceptionInformation[1]));
			}

#pragma warning(push)
#pragma warning(disable: 4996)
			OSVERSIONINFOEXA version_info;
			ZeroMemory(&version_info, sizeof(version_info));
			version_info.dwOSVersionInfoSize = sizeof(version_info);
			GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&version_info));
#pragma warning(pop)

			line(utils::string::va("OS Version: %u.%u", version_info.dwMajorVersion, version_info.dwMinorVersion));
			line(std::string{});
			line(get_callstack_summary(exceptioninfo->ExceptionRecord->ExceptionAddress));
			const auto registers = get_memory_registers(exceptioninfo);
			if (!registers.empty())
			{
				line(std::string{});
				line(registers);
			}

			return info;
		}

		void write_minidump(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			const std::string crash_name = utils::string::va("minidumps/ezz-crash-%s.zip",
			                                                 get_timestamp().data());

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

			printf("[Exception] Caught fatal exception 0x%08X at 0x%llX\n",
				exceptioninfo->ExceptionRecord->ExceptionCode,
				reinterpret_cast<uint64_t>(exceptioninfo->ExceptionRecord->ExceptionAddress));

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

			const auto root_path = utils::nt::library{}.get_path().parent_path();
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