#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/thread.hpp>
#include <utils/compression.hpp>

#include <exception/minidump.hpp>

#include <version.hpp>

#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

namespace exception
{
	namespace
	{
		DWORD main_thread_id{};
		std::once_flag sym_init_flag{};

		void ensure_symbols_initialized()
		{
			std::call_once(sym_init_flag, []
			{
				SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
				SymInitialize(GetCurrentProcess(), nullptr, TRUE);
			});
		}

		struct resolved_frame
		{
			std::string module_name;
			std::string function_name;
			std::string file_name;
			uint64_t address = 0;
			uint64_t module_base = 0;
			uint64_t rva = 0;
			DWORD line_number = 0;
		};

		resolved_frame resolve_address(void* addr)
		{
			resolved_frame frame{};
			frame.address = reinterpret_cast<uint64_t>(addr);

			const auto mod = utils::nt::library::get_by_address(addr);
			if (mod)
			{
				frame.module_name = mod.get_name();
				frame.module_base = reinterpret_cast<uint64_t>(mod.get_ptr());
				frame.rva = frame.address - frame.module_base;

				if (frame.module_name == "BlackOps3.exe")
					frame.rva += 0x140000000;
			}
			else
			{
				frame.module_name = "unknown";
				frame.rva = frame.address;
			}

			ensure_symbols_initialized();

			// Try to resolve function name from PDB symbols
			alignas(SYMBOL_INFO) char sym_buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME]{};
			auto* sym = reinterpret_cast<SYMBOL_INFO*>(sym_buffer);
			sym->SizeOfStruct = sizeof(SYMBOL_INFO);
			sym->MaxNameLen = MAX_SYM_NAME;

			DWORD64 displacement = 0;
			if (SymFromAddr(GetCurrentProcess(), frame.address, &displacement, sym))
			{
				frame.function_name = sym->Name;
				if (displacement > 0)
					frame.function_name += utils::string::va("+0x%llX", displacement);
			}

			// Try to resolve source file and line
			IMAGEHLP_LINE64 line_info{};
			line_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			DWORD line_displacement = 0;
			if (SymGetLineFromAddr64(GetCurrentProcess(), frame.address, &line_displacement, &line_info))
			{
				frame.file_name = line_info.FileName;
				frame.line_number = line_info.LineNumber;
			}

			return frame;
		}

		std::string format_frame(const resolved_frame& f, size_t index)
		{
			std::string entry = utils::string::va("\t[%zu] %s + 0x%llX", index, f.module_name.c_str(), f.rva);

			if (!f.function_name.empty())
				entry += utils::string::va("  (%s)", f.function_name.c_str());

			if (!f.file_name.empty() && f.line_number > 0)
				entry += utils::string::va("  [%s:%lu]", f.file_name.c_str(), f.line_number);

			return entry;
		}

		std::vector<resolved_frame> capture_stackwalk(const LPEXCEPTION_POINTERS exceptioninfo, int max_frames = 48)
		{
			std::vector<resolved_frame> frames;

			if (!exceptioninfo || !exceptioninfo->ContextRecord)
				return frames;

			ensure_symbols_initialized();

			CONTEXT ctx = *exceptioninfo->ContextRecord;

			STACKFRAME64 stack_frame{};
			stack_frame.AddrPC.Offset = ctx.Rip;
			stack_frame.AddrPC.Mode = AddrModeFlat;
			stack_frame.AddrFrame.Offset = ctx.Rbp;
			stack_frame.AddrFrame.Mode = AddrModeFlat;
			stack_frame.AddrStack.Offset = ctx.Rsp;
			stack_frame.AddrStack.Mode = AddrModeFlat;

			const auto process = GetCurrentProcess();
			const auto thread = GetCurrentThread();

			for (int i = 0; i < max_frames; ++i)
			{
				if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread,
					&stack_frame, &ctx, nullptr, SymFunctionTableAccess64,
					SymGetModuleBase64, nullptr))
					break;

				if (stack_frame.AddrPC.Offset == 0)
					break;

				frames.push_back(resolve_address(reinterpret_cast<void*>(stack_frame.AddrPC.Offset)));
			}

			return frames;
		}

		std::string get_crash_module_info(void* address)
		{
			auto frame = resolve_address(address);
			std::string info = frame.module_name + utils::string::va("+0x%llX", frame.rva);
			if (!frame.function_name.empty())
				info += " (" + frame.function_name + ")";
			return info;
		}

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

		const char* get_exception_string(DWORD exception);

		void display_error_dialog()
		{
			const auto frame = resolve_address(exception_data.address);
			const auto exception_name = get_exception_string(exception_data.code);
			const auto location = get_crash_module_info(exception_data.address);

			const std::string error_str = utils::string::va(
				"%s (0x%08X) at %s\n\n"
				"Address: 0x%p (RVA: 0x%llX)\n"
				"Module: %s\n"
				"%s%s"
				"\nA crash dump has been saved to the 'minidumps' folder.\n"
				"Please report this crash and upload the dump file on our Discord:\n"
				"https://dc.ezz.lol\n",
				exception_name, exception_data.code, location.c_str(),
				exception_data.address, frame.rva, frame.module_name.c_str(),
				frame.function_name.empty() ? "" : "Function: ",
				frame.function_name.empty() ? "" : (frame.function_name + "\n").c_str());

			utils::thread::suspend_other_threads();
			show_mouse_cursor();

			game::show_error(error_str.data(), "Ezz ERROR");

			ShellExecuteA(nullptr, "open", "minidumps", nullptr, nullptr, SW_SHOWNORMAL);

			TerminateProcess(GetCurrentProcess(), exception_data.code);
		}


		void reset_state()
		{
			if (game::is_server())
			{
				if (!server_restart::restart_pending.load())
				{
					if (server_restart::consecutive_crash_count.fetch_add(1) < 3)
					{
						server_restart::schedule("Unhandled server exception");
					}
				}

				if (!is_game_thread())
				{
					SuspendThread(GetCurrentThread());
				}
				return;
			}

			if (is_recoverable())
			{
				const auto location = get_crash_module_info(exception_data.address);
				const auto exception_name = get_exception_string(exception_data.code);

				recovery_data.last_recovery = std::chrono::high_resolution_clock::now();
				++recovery_data.recovery_counts;

				scheduler::once([]
				{
					if (game::Com_IsInGame())
						game::Cbuf_AddText(0, "disconnect\n");
				}, scheduler::pipeline::main);

				game::Com_Error(game::ERR_DROP,
				                "%s (0x%08X) at %s\n\n"
				                "A crash dump has been saved to the 'minidumps' folder.\n\n"
				                "Ezz has tried to recover your game, but it might not run stable anymore.\n\n"
				                "Make sure to update your graphics card drivers and install operating system updates!\n"
				                "Closing or restarting Steam might also help.\n\n"
				                "If this keeps happening, please report it on our Discord: https://dc.ezz.lol",
				                exception_name, exception_data.code, location.c_str());
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

		std::string get_callstack_summary(const LPEXCEPTION_POINTERS exceptioninfo, int trace_depth = 48)
		{
			std::string info{"callstack:\r\n{\r\n"};

			auto frames = capture_stackwalk(exceptioninfo, trace_depth);

			if (frames.empty())
			{
				// Fallback to RtlCaptureStackBackTrace if StackWalk64 fails
				void* backtrace_stack[32]{};
				const auto count = RtlCaptureStackBackTrace(0, 32, backtrace_stack, nullptr);
				for (USHORT i = 0; i < count; ++i)
				{
					auto f = resolve_address(backtrace_stack[i]);
					info.append(format_frame(f, i));
					info.append("\r\n");
				}
			}
			else
			{
				for (size_t i = 0; i < frames.size(); ++i)
				{
					info.append(format_frame(frames[i], i));
					info.append("\r\n");
				}
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

			const auto crash_frame = resolve_address(exceptioninfo->ExceptionRecord->ExceptionAddress);

			line("Ezz Crash Dump");
			line(std::string{});
			line("Version: "s + VERSION);
			line("Timestamp: "s + get_timestamp());
			line(utils::string::va("Exception: 0x%08X (%s)", exceptioninfo->ExceptionRecord->ExceptionCode,
			                      get_exception_string(exceptioninfo->ExceptionRecord->ExceptionCode)));
			line(utils::string::va("Address: 0x%llX", crash_frame.address));
			line(utils::string::va("Module: %s + 0x%llX", crash_frame.module_name.c_str(), crash_frame.rva));
			if (!crash_frame.function_name.empty())
				line("Function: " + crash_frame.function_name);
			if (!crash_frame.file_name.empty() && crash_frame.line_number > 0)
				line(utils::string::va("Source: %s:%lu", crash_frame.file_name.c_str(), crash_frame.line_number));
			line(utils::string::va("Base: 0x%llX", game::get_base()));
			line(utils::string::va("Thread ID: %lu (%s)", GetCurrentThreadId(), is_game_thread() ? "main" : "auxiliary"));      

			if (exceptioninfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
			{
				const char* op = exceptioninfo->ExceptionRecord->ExceptionInformation[0] == 1 ? "write to" : "read from";
				auto target = exceptioninfo->ExceptionRecord->ExceptionInformation[1];
				line(utils::string::va("Access Violation: Attempted to %s 0x%012llX%s",
					op, target, target < 0x10000 ? " (NULL pointer dereference)" : ""));
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
			line(get_callstack_summary(exceptioninfo));
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
			const char* patch_name = nullptr;

			switch (offset)
			{
			// Killcam animation crash - invalid anim data access
			case 0x234B9BD:
				patch_name = "Killcam animation (invalid anim data)";
				context->Rax = 0;
				context->Rip = base + 0x234D14B;
				break;

			// CG_ZBarrierAttachWeapon - null weapon pointer in zombie barriers
			case 0x464FEF:
				patch_name = "ZBarrier weapon attach (null weapon)";
				context->Rax = 0;
				context->Rip = base + 0x4651A2;
				break;

			// asmsetanimationrate - bad entity reference
			case 0x15E4B5A:
				patch_name = "asmsetanimationrate (bad entity ref)";
				context->Rip = base + 0x15E4B83;
				break;

			// Orphaned thread crash
			case 0x12EE4CC:
				patch_name = "Orphaned thread";
				context->Rip = base + 0x12EE5C8;
				break;

			// Character index out-of-bounds crash
			case 0x234210C:
				patch_name = "Character index out-of-bounds";
				context->Rip = base + 0x2342136;
				break;

			// HKS internal crash
			case 0x1CAB4F1:
				patch_name = "HKS/Lua internal error";
				context->Rip = base + 0x1CAB69E;
				break;

			// Null localization string crashes
			case 0x2279323:
				patch_name = "Null localization string (UI)";
				context->Rdx = reinterpret_cast<uintptr_t>(ui_localize_fallback);
				break;

			case 0x2278B96:
				patch_name = "Null localization string (UI)";
				context->Rsi = reinterpret_cast<uintptr_t>(ui_localize_fallback);
				break;

			case 0x228ED56:
				patch_name = "Null localization string (UI)";
				context->Rcx = reinterpret_cast<uintptr_t>(ui_localize_fallback);
				break;

			// Unknown UI crash
			case 0x1EAAA27:
				patch_name = "UI crash (unknown)";
				context->Rip = base + 0x1EAABB3;
				break;

			// Non-existent clientfield crashes (CSC side)
			case 0xC15B80:
			case 0xC15C50:
			case 0xC18CF5:
				patch_name = "Non-existent clientfield (CSC)";
				context->Rcx = 1; // CSC instance
				context->Rdx = reinterpret_cast<uintptr_t>("Clientfield does not exist");
				context->R8 = 0;
				context->Rip = base + 0x12EA430; // Scr_Error
				break;

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
				patch_name = "Non-existent clientfield (GSC)";
				context->Rcx = 0; // GSC instance
				context->Rdx = reinterpret_cast<uintptr_t>("Clientfield does not exist");
				context->R8 = 0;
				context->Rip = base + 0x12EA430; // Scr_Error
				break;

			// Non-existent clientfield (additional crash sites)
			case 0x133EC1:
			case 0x133EEB:
				patch_name = "Non-existent clientfield (additional)";
				context->Rip = base + 0x133F12;
				break;

			case 0x133F31:
				patch_name = "Non-existent clientfield (additional)";
				context->Rip = base + 0x133F42;
				break;

			// Random crash on Zetsubou No Shima
			case 0x13591D3:
				patch_name = "Zetsubou No Shima map bug";
				context->Rip = base + 0x13591DA;
				break;

			default:
				// Server restart recovery: skip crashes using udis86 instruction decode
				if (game::is_server() && server_restart::restart_recovery_active.load()
					&& record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
				{
					const int skips = server_restart::recovery_skip_count.fetch_add(1);
					if (skips < 20) // Max 20 instruction skips per recovery cycle
					{
						ud_t ud;
						ud_init(&ud);
						ud_set_mode(&ud, 64);
						ud_set_pc(&ud, addr);
						ud_set_input_buffer(&ud, reinterpret_cast<const uint8_t*>(addr), 15);
						if (ud_decode(&ud))
						{
							const auto len = ud_insn_len(&ud);

							context->Rip += len;
							context->Rax = 0;
							return EXCEPTION_CONTINUE_EXECUTION;
						}
					}
					else
					{

						server_restart::restart_recovery_active.store(false);
					}
				}
				return EXCEPTION_CONTINUE_SEARCH;
			}

			if (patch_name)
			{
				printf("^3[Exception] Known crash patched: %s (base+0x%llX)\n", patch_name, offset);
			}

			return EXCEPTION_CONTINUE_EXECUTION;
		}

		bool is_harmless_error(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			const auto code = exceptioninfo->ExceptionRecord->ExceptionCode;
			return code == STATUS_INTEGER_OVERFLOW || code == STATUS_FLOAT_OVERFLOW || code == STATUS_SINGLE_STEP;
		}

		void handle_server_script_vm_crash()
		{
			server_restart::schedule("Script VM crash (auxiliary thread)");
			SuspendThread(GetCurrentThread());
		}

		size_t get_script_error_stub()
		{
			static auto* stub = utils::hook::assemble([](utils::hook::assembler& a)
			{
				a.sub(rsp, 0x10);
				a.or_(rsp, 0x8);
				a.jmp(handle_server_script_vm_crash);
			});

			return reinterpret_cast<size_t>(stub);
		}

		bool is_server_script_vm_crash(uint64_t offset, DWORD exception_code, const EXCEPTION_RECORD* record)
		{
			if (exception_code != EXCEPTION_ACCESS_VIOLATION)
				return false;

			if (record->ExceptionInformation[1] >= 0x10000)
				return false;

			switch (offset)
			{
			case 0x269F8A:
			case 0x2ADCF9:
			case 0x2ADA49:
			case 0x2AD551:
				return true;
			default:
				return false;
			}
		}

		LONG WINAPI exception_filter(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			if (is_harmless_error(exceptioninfo))
			{
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			const auto addr = reinterpret_cast<uintptr_t>(exceptioninfo->ExceptionRecord->ExceptionAddress);
			const auto base = game::get_base();
			const auto filter_offset = addr - base;

			// Handle known server script VM crashes on ANY thread
			if (game::is_server() && is_server_script_vm_crash(filter_offset, exceptioninfo->ExceptionRecord->ExceptionCode, exceptioninfo->ExceptionRecord))
			{
				if (!is_game_thread())
				{
					exceptioninfo->ContextRecord->Rip = get_script_error_stub();
					return EXCEPTION_CONTINUE_EXECUTION;
				}
				else
				{
					return EXCEPTION_CONTINUE_SEARCH;
				}
			}

			const auto crash_frame = resolve_address(exceptioninfo->ExceptionRecord->ExceptionAddress);
			const auto exception_name = get_exception_string(exceptioninfo->ExceptionRecord->ExceptionCode);

			// Detailed console crash report
			printf("\n^1========== CRASH DETECTED ==========\n");
			printf("^1  Exception:  %s (0x%08X)\n", exception_name, exceptioninfo->ExceptionRecord->ExceptionCode);
			printf("^1  Module:     %s + 0x%llX\n", crash_frame.module_name.c_str(), crash_frame.rva);
			if (!crash_frame.function_name.empty())
				printf("^1  Function:   %s\n", crash_frame.function_name.c_str());
			if (!crash_frame.file_name.empty() && crash_frame.line_number > 0)
				printf("^1  Source:     %s:%lu\n", crash_frame.file_name.c_str(), crash_frame.line_number);
			printf("^1  Address:    0x%llX\n", crash_frame.address);
			printf("^1  Thread:     %lu (%s)\n", GetCurrentThreadId(), is_game_thread() ? "main" : "auxiliary");

			if (exceptioninfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
			{
				const char* op = exceptioninfo->ExceptionRecord->ExceptionInformation[0] == 1 ? "write to" : "read from";
				auto target = exceptioninfo->ExceptionRecord->ExceptionInformation[1];
				printf("^1  Details:    Attempted to %s 0x%012llX%s\n",
					op, target, target < 0x10000 ? " (NULL pointer dereference)" : "");
			}

			// Print condensed callstack to console
			auto frames = capture_stackwalk(exceptioninfo, 16);
			if (!frames.empty())
			{
				printf("^1  Callstack:\n");
				for (size_t i = 0; i < frames.size(); ++i)
				{
					const auto& f = frames[i];
					if (!f.function_name.empty())
						printf("^1    [%zu] %s!%s\n", i, f.module_name.c_str(), f.function_name.c_str());
					else
						printf("^1    [%zu] %s + 0x%llX\n", i, f.module_name.c_str(), f.rva);
				}
			}
			printf("^1=====================================\n\n");

			if (!game::is_server())
			{
				const auto crash_info = generate_crash_info(exceptioninfo);
				write_minidump(exceptioninfo);
			}
			
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

			AddVectoredExceptionHandler(1, crash_fix_exception_handler);
		}
	};
}

REGISTER_COMPONENT(exception::component)