#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"

#include "game/game.hpp"
#include "steam/steam.hpp"

#include <utils/io.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/thread.hpp>

#include "integrity.hpp"

#include <stack>

#define ProcessDebugPort 7
#define ProcessDebugObjectHandle 30
#define ProcessDebugFlags 31
#define ProcessImageFileNameWin32 43

namespace arxan
{
	namespace detail
	{
		void* callstack_proxy_addr{nullptr};
		static thread_local const void* address_to_call{};

		void set_address_to_call(const void* address)
		{
			address_to_call = address;
		}
	}

	namespace
	{
		thread_local std::stack<uint64_t> address_stack{};

		const void* get_address_to_call()
		{
			return detail::address_to_call;
		}

		void store_address(const uint64_t address)
		{
			address_stack.push(address);
		}

		uint64_t get_stored_address()
		{
			const auto res = address_stack.top();
			address_stack.pop();

			return res;
		}

		void callstack_return_stub(utils::hook::assembler& a)
		{
			a.push(rax);
			a.pushad64();

			a.call_aligned(get_stored_address);
			a.mov(qword_ptr(rsp, 0x80), rax);

			a.popad64();

			a.add(rsp, 8);

			a.jmp(qword_ptr(rsp, -8));
		}

		uint64_t get_callstack_return_stub()
		{
			const auto placeholder = game::select(0x140001056, 0x140101168);
			utils::hook::set<uint8_t>(placeholder - 2, 0xFF); // fakes a call
			utils::hook::nop(placeholder, 1);
			utils::hook::jump(placeholder + 1, utils::hook::assemble(callstack_return_stub));

			return placeholder;
		}

		void callstack_stub(utils::hook::assembler& a)
		{
			a.push(rax);

			a.pushad64();
			a.call_aligned(get_address_to_call);
			a.mov(qword_ptr(rsp, 0x80), rax);

			a.mov(rcx, qword_ptr(rsp, 0x88));
			a.call_aligned(store_address);

			a.mov(rax, get_callstack_return_stub());
			a.mov(qword_ptr(rsp, 0x88), rax);

			a.popad64();

			a.add(rsp, 8);

			a.jmp(qword_ptr(rsp, -8));
		}

		constexpr auto pseudo_steam_id = 0x1337;
		const auto pseudo_steam_handle = reinterpret_cast<HANDLE>(reinterpret_cast<uint64_t>(INVALID_HANDLE_VALUE) -
			pseudo_steam_id);

		utils::hook::detour nt_close_hook;
		utils::hook::detour nt_query_system_information_hook;
		utils::hook::detour nt_query_information_process_hook;
		utils::hook::detour create_mutex_ex_a_hook;
		utils::hook::detour open_process_hook;
		utils::hook::detour create_thread_hook;
		utils::hook::detour get_thread_context_hook;
		utils::hook::detour zw_terminate_process_hook;
		utils::hook::detour get_proc_address_hook;

		void* original_first_tls_callback = nullptr;

		void** get_tls_callbacks()
		{
			const utils::nt::library game{};
			const auto& entry = game.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
			if (!entry.VirtualAddress || !entry.Size)
			{
				return nullptr;
			}

			const auto* tls_dir = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(game.get_ptr() + entry.VirtualAddress);
			return reinterpret_cast<void**>(tls_dir->AddressOfCallBacks);
		}

		void disable_tls_callbacks()
		{
			auto* tls_callbacks = get_tls_callbacks();
			if (tls_callbacks)
			{
				original_first_tls_callback = *tls_callbacks;
			}

			utils::hook::set(tls_callbacks, nullptr);
		}

		void restore_tls_callbacks()
		{
			auto* tls_callbacks = get_tls_callbacks();
			if (tls_callbacks)
			{
				utils::hook::set(tls_callbacks, original_first_tls_callback);
			}
		}

		HANDLE WINAPI create_thread_stub(const LPSECURITY_ATTRIBUTES thread_attributes, const SIZE_T stack_size,
		                                 const LPTHREAD_START_ROUTINE start_address, const LPVOID parameter,
		                                 const DWORD creation_flags,
		                                 const LPDWORD thread_id)
		{
			if (utils::nt::library::get_by_address(start_address) == utils::nt::library{})
			{
				restore_tls_callbacks();

				create_thread_hook.clear();
				return CreateThread(thread_attributes, stack_size, start_address, parameter, creation_flags,
				                    thread_id);
			}

			return create_thread_hook.invoke<HANDLE>(thread_attributes, stack_size, start_address, parameter,
			                                         creation_flags, thread_id);
		}

		HANDLE process_id_to_handle(const DWORD pid)
		{
			return reinterpret_cast<HANDLE>(static_cast<DWORD64>(pid));
		}

		HANDLE WINAPI open_process_stub(const DWORD access, const BOOL inherit, const DWORD pid)
		{
			if (pid == pseudo_steam_id)
			{
				return pseudo_steam_handle;
			}

			return open_process_hook.invoke<HANDLE>(access, inherit, pid);
		}


		HANDLE create_mutex_ex_a_stub(const LPSECURITY_ATTRIBUTES attributes, const LPCSTR name, const DWORD flags,
		                              const DWORD access)
		{
			if (name == "$ IDA trusted_idbs"s || name == "$ IDA registry mutex $"s)
			{
				return nullptr;
			}

			return create_mutex_ex_a_hook.invoke<HANDLE>(attributes, name, flags, access);
		}

		bool remove_evil_keywords_from_string(const UNICODE_STRING& string)
		{
			static const std::wstring evil_keywords[] =
			{
				L"IDA",
				L"ida",
				L"HxD",
				L"cheatengine",
				L"Cheat Engine",
				L"x96dbg",
				L"x32dbg",
				L"x64dbg",
				L"Wireshark",
				L"Debug",
				L"DEBUG",
				L"msvsmon",
			};

			if (!string.Buffer || !string.Length)
			{
				return false;
			}

			const std::wstring_view path(string.Buffer, string.Length / sizeof(string.Buffer[0]));

			bool modified = false;
			for (const auto& keyword : evil_keywords)
			{
				while (true)
				{
					const auto pos = path.find(keyword);
					if (pos == std::wstring::npos)
					{
						break;
					}

					modified = true;

					for (size_t i = 0; i < keyword.size(); ++i)
					{
						string.Buffer[pos + i] = L'a';
					}
				}
			}

			return modified;
		}

		bool remove_evil_keywords_from_string(wchar_t* str, const size_t length)
		{
			UNICODE_STRING unicode_string{};
			unicode_string.Buffer = str;
			unicode_string.Length = static_cast<uint16_t>(length);
			unicode_string.MaximumLength = unicode_string.Length;

			return remove_evil_keywords_from_string(unicode_string);
		}

		bool remove_evil_keywords_from_string(char* str, const size_t length)
		{
			std::string_view str_view(str, length);
			std::wstring wstr(str_view.begin(), str_view.end());

			if (!remove_evil_keywords_from_string(wstr.data(), wstr.size()))
			{
				return false;
			}

			const std::string regular_str(wstr.begin(), wstr.end());
			memcpy(str, regular_str.data(), length);

			return true;
		}


		int WINAPI get_window_text_a_stub(const HWND wnd, const LPSTR str, const int max_count)
		{
			std::wstring wstr{};
			wstr.resize(max_count);

			const auto res = GetWindowTextW(wnd, wstr.data(), max_count);
			if (res)
			{
				remove_evil_keywords_from_string(wstr.data(), res);

				const std::string regular_str(wstr.begin(), wstr.end());
				memset(str, 0, max_count);
				memcpy(str, regular_str.data(), res);
			}

			return res;
		}

		NTSTATUS NTAPI nt_query_system_information_stub(const SYSTEM_INFORMATION_CLASS system_information_class,
		                                                const PVOID system_information,
		                                                const ULONG system_information_length,
		                                                const PULONG return_length)
		{
			const auto status = nt_query_system_information_hook.invoke<NTSTATUS>(
				system_information_class, system_information, system_information_length, return_length);

			if (NT_SUCCESS(status))
			{
				if (system_information_class == SystemProcessInformation && !utils::nt::is_shutdown_in_progress())
				{
					bool injected_steam = false;
					auto addr = static_cast<uint8_t*>(system_information);
					while (true)
					{
						const auto info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(addr);
						remove_evil_keywords_from_string(info->ImageName);

						static const auto our_pid = process_id_to_handle(GetCurrentProcessId());

						if (!injected_steam && info->UniqueProcessId != our_pid)
						{
							static wchar_t steam_path[] = L"steam.exe";

							info->UniqueProcessId = process_id_to_handle(pseudo_steam_id);
							info->ImageName.Buffer = steam_path;
							info->ImageName.Length = static_cast<uint16_t>(wcslen(steam_path) * 2u);
							info->ImageName.MaximumLength = info->ImageName.Length;

							injected_steam = true;
						}

						if (!info->NextEntryOffset)
						{
							break;
						}

						addr = addr + info->NextEntryOffset;
					}
				}
			}

			return status;
		}

		bool handle_pseudo_steam_process(const HANDLE handle, const PROCESSINFOCLASS info_class,
		                                 const PVOID info,
		                                 const ULONG info_length, const PULONG ret_length, NTSTATUS* status)
		{
			if (handle != pseudo_steam_handle || static_cast<int>(info_class) != 43)
			{
				return false;
			}

			const auto steam_folder = steam::SteamAPI_GetSteamInstallPath();
			const auto steam_path = steam_folder + "\\steam.exe"s;
			const std::wstring wide_path(steam_path.begin(), steam_path.end());

			const auto required_size = static_cast<ULONG>((wide_path.size() + 1u) * 2u + sizeof(UNICODE_STRING));

			if (ret_length)
			{
				*ret_length = required_size;
			}

			if (info_length < required_size)
			{
				*status = static_cast<LONG>(0xC0000004);
				return true;
			}

			memset(info, 0, info_length);

			auto& str = *static_cast<UNICODE_STRING*>(info);
			str.Buffer = reinterpret_cast<wchar_t*>(&str + 1);
			str.Length = static_cast<uint16_t>(wide_path.size() * 2u);
			str.MaximumLength = str.Length;

			memcpy(str.Buffer, wide_path.data(), str.Length);

			*status = 0;
			return true;
		}

		NTSTATUS WINAPI nt_query_information_process_stub(const HANDLE handle, const PROCESSINFOCLASS info_class,
		                                                  const PVOID info,
		                                                  const ULONG info_length, const PULONG ret_length)
		{
			NTSTATUS status{0};
			if (handle_pseudo_steam_process(handle, info_class, info, info_length, ret_length, &status))
			{
				return status;
			}

			status = nt_query_information_process_hook.invoke<NTSTATUS>(handle, info_class, info, info_length,
			                                                            ret_length);

			if (NT_SUCCESS(status))
			{
				if (info_class == ProcessBasicInformation)
				{
					static_cast<PPROCESS_BASIC_INFORMATION>(info)->Reserved3 = process_id_to_handle(pseudo_steam_id);
				}
				else if (info_class == ProcessDebugObjectHandle)
				{
					*static_cast<HANDLE*>(info) = nullptr;
					return static_cast<LONG>(0xC0000353);
				}
				else if (info_class == ProcessImageFileName || static_cast<int>(info_class) ==
					ProcessImageFileNameWin32)
				{
					remove_evil_keywords_from_string(*static_cast<UNICODE_STRING*>(info));
				}
				else if (info_class == ProcessDebugPort)
				{
					*static_cast<HANDLE*>(info) = nullptr;
				}
				else if (info_class == ProcessDebugFlags)
				{
					*static_cast<ULONG*>(info) = 1;
				}
			}

			return status;
		}

		NTSTATUS NTAPI nt_close_stub(const HANDLE handle)
		{
			if (handle == pseudo_steam_handle)
			{
				return 0;
			}

			char info[16];
			if (NtQueryObject(handle, OBJECT_INFORMATION_CLASS(4), &info, 2, nullptr) >= 0 && size_t(handle) != 0x12345)
			{
				return nt_close_hook.invoke<NTSTATUS>(handle);
			}

			return STATUS_INVALID_HANDLE;
		}

		void hide_being_debugged()
		{
			auto* const peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
			peb->BeingDebugged = false;
			*reinterpret_cast<PDWORD>(LPSTR(peb) + 0xBC) &= ~0x70;
		}

		void restore_debug_functions()
		{
			static const char* functions[] = {
				"DbgBreakPoint",
				"DbgUserBreakPoint",
				"DbgUiConnectToDbg",
				"DbgUiContinue",
				"DbgUiConvertStateChangeStructure",
				"DbgUiDebugActiveProcess",
				"DbgUiGetThreadDebugObject",
				"DbgUiIssueRemoteBreakin",
				"DbgUiRemoteBreakin",
				"DbgUiSetThreadDebugObject",
				"DbgUiStopDebugging",
				"DbgUiWaitStateChange",
				"DbgPrintReturnControlC",
				"DbgPrompt",
			};

			using buffer = uint8_t[15];
			static buffer buffers[ARRAYSIZE(functions)] = {};
			static bool loaded = false;

			const utils::nt::library ntdll("ntdll.dll");

			for (auto i = 0u; i < ARRAYSIZE(functions); ++i)
			{
				const auto func = ntdll.get_proc<void*>(functions[i]);
				if (!func)
				{
					continue;
				}

				if (!loaded)
				{
					memcpy(buffers[i], func, sizeof(buffer));
				}
				else
				{
					utils::hook::copy(func, buffers[i], sizeof(buffer));
				}
			}

			loaded = true;
		}

		const std::vector<std::pair<uint8_t*, size_t>>& get_text_sections()
		{
			static const std::vector<std::pair<uint8_t*, size_t>> text = []
			{
				std::vector<std::pair<uint8_t*, size_t>> texts{};

				const utils::nt::library game{};
				for (const auto& section : game.get_section_headers())
				{
					if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
					{
						texts.emplace_back(game.get_ptr() + section->VirtualAddress, section->Misc.VirtualSize);
					}
				}

				return texts;
			}();

			return text;
		}

		bool is_in_texts(const uint64_t addr)
		{
			const auto& texts = get_text_sections();
			for (const auto& text : texts)
			{
				const auto start = reinterpret_cast<ULONG_PTR>(text.first);
				if (addr >= start && addr <= (start + text.second))
				{
					return true;
				}
			}

			return false;
		}

		bool is_in_texts(const void* addr)
		{
			return is_in_texts(reinterpret_cast<uint64_t>(addr));
		}

		struct integrity_handler_context
		{
			uint32_t* computed_checksum;
			uint32_t* original_checksum;
		};

		bool is_on_stack(uint8_t* stack_frame, const void* pointer)
		{
			const auto stack_value = reinterpret_cast<uint64_t>(stack_frame);
			const auto pointer_value = reinterpret_cast<uint64_t>(pointer);

			const auto diff = static_cast<int64_t>(stack_value - pointer_value);
			return std::abs(diff) < 0x1000;
		}

		// Pretty trashy, but working, heuristic to search the integrity handler context
		bool is_handler_context(uint8_t* stack_frame, const uint32_t computed_checksum, const uint32_t frame_offset)
		{
			const auto* potential_context = reinterpret_cast<integrity_handler_context*>(stack_frame + frame_offset);
			return is_on_stack(stack_frame, potential_context->computed_checksum)
				&& *potential_context->computed_checksum == computed_checksum
				&& is_in_texts(potential_context->original_checksum);
		}

		integrity_handler_context* search_handler_context(uint8_t* stack_frame, const uint32_t computed_checksum)
		{
			for (uint32_t frame_offset = 0; frame_offset < 0x90; frame_offset += 8)
			{
				if (is_handler_context(stack_frame, computed_checksum, frame_offset))
				{
					return reinterpret_cast<integrity_handler_context*>(stack_frame + frame_offset);
				}
			}

			return nullptr;
		}

		uint32_t adjust_integrity_checksum(const uint64_t return_address, uint8_t* stack_frame,
		                                   const uint32_t current_checksum)
		{
			const auto handler_address = game::derelocate(return_address - 5);
			const auto* context = search_handler_context(stack_frame, current_checksum);

			if (!context)
			{
				MessageBoxA(nullptr, utils::string::va("No frame offset for: %llX", handler_address), "Error",
				            MB_ICONERROR);
				TerminateProcess(GetCurrentProcess(), 0xBAD);
				return current_checksum;
			}

			const auto correct_checksum = *context->original_checksum;
			*context->computed_checksum = correct_checksum;

			if (current_checksum != correct_checksum)
			{
#ifndef NDEBUG
				/*printf("Adjusting checksum (%llX): %X -> %X\n", handler_address,
				       current_checksum, correct_checksum);*/
#endif
			}

			return correct_checksum;
		}

		void patch_intact_basic_block_integrity_check(void* address)
		{
			const auto game_address = reinterpret_cast<uint64_t>(address);
			constexpr auto inst_len = 3;

			const auto next_inst_addr = game_address + inst_len;
			const auto next_inst = *reinterpret_cast<uint32_t*>(next_inst_addr);

			if ((next_inst & 0xFF00FFFF) != 0xFF004583)
			{
				throw std::runtime_error(utils::string::va("Unable to patch intact basic block: %llX", game_address));
			}

			const auto other_frame_offset = static_cast<uint8_t>(next_inst >> 16);
			static const auto stub = utils::hook::assemble([](utils::hook::assembler& a)
			{
				a.push(rax);

				a.mov(rax, qword_ptr(rsp, 8));
				a.sub(rax, 2); // Skip the push we inserted

				a.push(rax);
				a.pushad64();

				a.mov(r8, qword_ptr(rsp, 0x88));
				a.mov(rcx, rax);
				a.mov(rdx, rbp);
				a.call_aligned(adjust_integrity_checksum);

				a.mov(qword_ptr(rsp, 0x80), rax);

				a.popad64();
				a.pop(rax);

				a.add(rsp, 8);

				a.mov(dword_ptr(rdx, rcx, 4), eax);

				a.pop(rax); // return addr
				a.xchg(rax, qword_ptr(rsp)); // switch with push

				a.add(dword_ptr(rbp, rax), 0xFFFFFFFF);

				a.mov(rax, dword_ptr(rdx, rcx, 4)); // restore rax

				a.ret();
			});

			// push other_frame_offset
			utils::hook::set<uint16_t>(game_address, static_cast<uint16_t>(0x6A | (other_frame_offset << 8)));
			utils::hook::call(game_address + 2, stub);
		}

		void patch_split_basic_block_integrity_check(void* address)
		{
			const auto game_address = reinterpret_cast<uint64_t>(address);
			constexpr auto inst_len = 3;

			const auto next_inst_addr = game_address + inst_len;

			if (*reinterpret_cast<uint8_t*>(next_inst_addr) != 0xE9)
			{
				throw std::runtime_error(utils::string::va("Unable to patch split basic block: %llX", game_address));
			}

			const auto jump_target = utils::hook::extract<void*>(reinterpret_cast<void*>(next_inst_addr + 1));
			const auto stub = utils::hook::assemble([jump_target](utils::hook::assembler& a)
			{
				a.push(rax);

				a.mov(rax, qword_ptr(rsp, 8));
				a.push(rax);

				a.pushad64();

				a.mov(r8, qword_ptr(rsp, 0x88));
				a.mov(rcx, rax);
				a.mov(rdx, rbp);
				a.call_aligned(adjust_integrity_checksum);

				a.mov(qword_ptr(rsp, 0x80), rax);

				a.popad64();
				a.pop(rax);

				a.add(rsp, 8);

				a.mov(dword_ptr(rdx, rcx, 4), eax);

				a.add(rsp, 8);

				a.jmp(jump_target);
			});

			utils::hook::call(game_address, stub);
		}

		void search_and_patch_integrity_checks_precomputed()
		{
			if (game::is_server())
			{
				for (const auto i : intact_integrity_check_blocks_server)
				{
					patch_intact_basic_block_integrity_check(reinterpret_cast<void*>(game::relocate(i)));
				}

				for (const auto i : split_integrity_check_blocks_server)
				{
					patch_split_basic_block_integrity_check(reinterpret_cast<void*>(game::relocate(i)));
				}
			}
			else
			{
				for (const auto i : intact_integrity_check_blocks)
				{
					patch_intact_basic_block_integrity_check(reinterpret_cast<void*>(game::relocate(i)));
				}

				for (const auto i : split_integrity_check_blocks)
				{
					patch_split_basic_block_integrity_check(reinterpret_cast<void*>(game::relocate(i)));
				}
			}
		}

		void search_and_patch_integrity_checks()
		{
			// There seem to be 1219 results.
			// Searching them is quite slow.
			// Maybe precomputing that might be better?

			/*const auto intact_results = "89 04 8A 83 45 ? FF"_sig;
			const auto split_results = "89 04 8A E9"_sig;

			for (auto* i : intact_results)
			{
				patch_intact_basic_block_integrity_check(i);
			}

			for (auto* i : split_results)
			{
				patch_split_basic_block_integrity_check(i);
			}*/

			search_and_patch_integrity_checks_precomputed();
		}

		LONG WINAPI exception_filter(const LPEXCEPTION_POINTERS info)
		{
			if (info->ExceptionRecord->ExceptionCode == STATUS_INVALID_HANDLE)
			{
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			return EXCEPTION_CONTINUE_SEARCH;
		}

		const char* get_command_line_a_stub()
		{
			static auto cmd = []
			{
				std::string cmd_line = GetCommandLineA();
				if (!strstr(cmd_line.data(), "fs_game"))
				{
					cmd_line += " +set fs_game \"boiii\"";
				}

				return cmd_line;
			}();

			return cmd.data();
		}
	}

	int WINAPI get_system_metrics_stub(const int index)
	{
		if (SM_REMOTESESSION == index)
		{
			return 0;
		}

		return GetSystemMetrics(index);
	}

	BOOL WINAPI get_thread_context_stub(const HANDLE thread_handle, const LPCONTEXT context)
	{
		constexpr auto debug_registers_flag = (CONTEXT_DEBUG_REGISTERS & ~CONTEXT_AMD64);
		if (context->ContextFlags & debug_registers_flag)
		{
			auto* source = _ReturnAddress();
			const auto game = utils::nt::library{};
			const auto source_module = utils::nt::library::get_by_address(source);

			if (source_module == game)
			{
				context->ContextFlags &= ~debug_registers_flag;
			}
		}

		return get_thread_context_hook.invoke<BOOL>(thread_handle, context);
	}

	NTSTATUS NTAPI get_proc_address_stub(const HMODULE module_handle, const PANSI_STRING function_name,
	                                     const WORD oridinal,
	                                     PVOID* function_address, const BOOL b_value,
	                                     PVOID* callback_address)
	{
		OutputDebugStringA(utils::string::va("Proc: %s %X\n",
		                                     (function_name && function_name->Buffer)
			                                     ? function_name->Buffer
			                                     : "(null)", static_cast<DWORD>(oridinal)));

		return get_proc_address_hook.invoke<NTSTATUS>(module_handle, function_name, oridinal, function_address, b_value,
		                                              callback_address);
	}

	NTSTATUS zw_terminate_process_stub(const HANDLE process_handle, const NTSTATUS exit_status)
	{
		MessageBoxA(nullptr, "TERMINATING", nullptr, 0);
		return zw_terminate_process_hook.invoke<NTSTATUS>(process_handle, exit_status);
	}

	struct component final : generic_component
	{
		void post_load() override
		{
			auto* dll_characteristics = &utils::nt::library().get_optional_header()->DllCharacteristics;
			utils::hook::set<WORD>(dll_characteristics, *dll_characteristics | IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE);

			disable_tls_callbacks();
			restore_debug_functions();

			hide_being_debugged();
			scheduler::loop(hide_being_debugged, scheduler::pipeline::async);

			create_thread_hook.create(CreateThread, create_thread_stub);
			create_mutex_ex_a_hook.create(CreateMutexExA, create_mutex_ex_a_stub);

			const utils::nt::library ntdll("ntdll.dll");
			nt_close_hook.create(ntdll.get_proc<void*>("NtClose"), nt_close_stub);

			const auto nt_query_information_process = ntdll.get_proc<void*>("NtQueryInformationProcess");
			nt_query_information_process_hook.create(nt_query_information_process,
			                                         nt_query_information_process_stub);

			const auto nt_query_system_information = ntdll.get_proc<void*>("NtQuerySystemInformation");
			nt_query_system_information_hook.create(nt_query_system_information, nt_query_system_information_stub);
			nt_query_system_information_hook.move();

			open_process_hook.create(OpenProcess, open_process_stub);

#ifndef NDEBUG
			auto* get_thread_context_func = utils::nt::library("kernelbase.dll").get_proc<void*>("GetThreadContext");
			get_thread_context_hook.create(get_thread_context_func, get_thread_context_stub);
#endif

			utils::hook::copy(this->window_text_buffer_, GetWindowTextA, sizeof(this->window_text_buffer_));
			utils::hook::jump(GetWindowTextA, get_window_text_a_stub, true, true);
			utils::hook::move_hook(GetWindowTextA);

			AddVectoredExceptionHandler(1, exception_filter);

			auto* sys_met_import = utils::nt::library{}.get_iat_entry("user32.dll", "GetSystemMetrics");
			if (sys_met_import) utils::hook::set(sys_met_import, get_system_metrics_stub);

			// TODO: Remove as soon as real hooking works
			//auto* get_cmd_import = utils::nt::library{}.get_iat_entry("kernel32.dll", "GetCommandLineA");
			//if (get_cmd_import) utils::hook::set(get_cmd_import, get_command_line_a_stub);

			//zw_terminate_process_hook.create(ntdll.get_proc<void*>("ZwTerminateProcess"), zw_terminate_process_stub);
			//zw_terminate_process_hook.move();

			//auto* gpafc = ntdll.get_proc<void*>("LdrGetProcedureAddressForCaller");
			//get_proc_address_hook.create(gpafc, get_proc_address_stub);
			//get_proc_address_hook.move();
		}

		void post_unpack() override
		{
			search_and_patch_integrity_checks();
			//restore_debug_functions();

			detail::callstack_proxy_addr = utils::hook::assemble(callstack_stub);
		}

		component_priority priority() const override
		{
			return component_priority::arxan;
		}

	private:
		uint8_t window_text_buffer_[15]{};
	};
}

REGISTER_COMPONENT(arxan::component)
