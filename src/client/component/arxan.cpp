#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"

#include "steam/steam.hpp"
#include <utils/hook.hpp>

#include "utils/finally.hpp"
#include "utils/string.hpp"
#include "utils/hardware_breakpoint.hpp"
#include "utils/io.hpp"

#define ProcessDebugPort 7
#define ProcessDebugObjectHandle 30 // WinXP source says 31?
#define ProcessDebugFlags 31 // WinXP source says 32?

namespace arxan
{
	namespace
	{
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
			};

			if (!string.Buffer || !string.Length)
			{
				return false;
			}

			std::wstring_view path(string.Buffer, string.Length / sizeof(string.Buffer[0]));

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

			if (!remove_evil_keywords_from_string(&wstr[0], wstr.size()))
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

			const auto res = GetWindowTextW(wnd, &wstr[0], max_count);
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
				if (system_information_class == SystemProcessInformation)
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
				else if (info_class == ProcessImageFileName || static_cast<int>(info_class) == 43 /* ? */)
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
			static const std::vector<std::pair<uint8_t*, size_t>> text = [
				]() -> std::vector<std::pair<uint8_t*, size_t>>
				{
					std::vector<std::pair<uint8_t*, size_t>> texts;

					const utils::nt::library game{};
					for (const auto& section : game.get_section_headers())
					{
						std::string name(reinterpret_cast<char*>(section->Name), sizeof(section->Name));
						while (!name.empty() && !name.back()) name.pop_back();

						if (name == ".text"s)
						{
							texts.emplace_back(game.get_ptr() + section->VirtualAddress, section->Misc.VirtualSize);
						}
					}

					return texts;
				}
				();

			return text;
		}

		bool was_in_texts(const ULONG_PTR addr)
		{
			const auto& texts = get_text_sections();
			for (const auto& text : texts)
			{
				if (addr >= reinterpret_cast<ULONG_PTR>(text.first) && addr <= reinterpret_cast<ULONG_PTR>(text.first +
					text.second))
				{
					return true;
				}
			}

			return false;
		}

		void protect_texts()
		{
			const auto& texts = get_text_sections();
			for (const auto& text : texts)
			{
				DWORD old_protect{};
				VirtualProtect(text.first, text.second, PAGE_EXECUTE_READ, &old_protect);
			}
		}

		void unprotect_texts()
		{
			const auto& texts = get_text_sections();
			for (const auto& text : texts)
			{
				DWORD old_protect{};
				VirtualProtect(text.first, text.second, PAGE_EXECUTE_READWRITE, &old_protect);
			}
		}

		struct integrity_handler_context
		{
			uint32_t* computed_checksum;
			uint32_t* original_checksum;
		};

		struct integrity_handler_data
		{
			uint64_t address;
			uint64_t checksum_address;
			uint32_t checksum;
			uint32_t frame_offset;

			std::string to_string() const
			{
				return utils::string::va("%llX,%llX,%X,%X", this->address, this->checksum_address, this->checksum,
				                         this->frame_offset);
			}

			static integrity_handler_data from_string(const std::string& line)
			{
				integrity_handler_data res{};
				sscanf_s(line.data(), "%llX,%llX,%X,%X", &res.address, &res.checksum_address, &res.checksum,
				         &res.frame_offset);

				return res;
			}
		};

		bool patchMode = false;
		std::unordered_map<uint64_t, integrity_handler_data> integrity_handlers;

		void load_handlers()
		{
			std::string data{};
			if (!utils::io::read_file("integrity.txt", &data)) return;

			const auto lines = utils::string::split(data, '\n');
			for (const auto& line : lines)
			{
				if (line.empty())continue;
				auto handler = integrity_handler_data::from_string(line);
				integrity_handlers[operator"" _g(handler.address)] = handler;
			}
		}

		void write_handlers()
		{
			std::string txt{};
			for (auto& h : integrity_handlers)
			{
				txt += h.second.to_string();
				txt += "\n";
			}

			utils::io::write_file("integrity.txt", txt);
		}

		LONG WINAPI exception_filter(const LPEXCEPTION_POINTERS info)
		{
			static thread_local struct
			{
				bool needs_protect_change = false;
				bool had_single_step = false;
			} analysis_context{};

			if (info->ExceptionRecord->ExceptionCode == STATUS_INVALID_HANDLE)
			{
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			if (info->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP && false)
			{
				auto dr6_clear = utils::finally([info]
				{
					info->ContextRecord->Dr6 = 0;
				});

				//utils::thread::suspend_other_threads();
				//restore_debug_functions();
				//MessageBoxA(0, "SS", 0, 0);
				static uint64_t addr = 0;
				/*if (info->ContextRecord->Rip == 0x15E4EBF6B_g)
				{
					OutputDebugStringA(utils::string::va("Lul: %X | %X", (uint32_t)info->ContextRecord->Rax,
					                                     (uint32_t)info->ContextRecord->Rdx));

					return EXCEPTION_CONTINUE_EXECUTION;
				}*/

				static const std::unordered_map<uint64_t, std::pair<uint32_t, bool>> lulul = {
					{0x15AF356A9_g, {0x58, true}},
					{0x15BC7D9D4_g, {0x54, false}}, // ?
					{0x15BC4905F_g, {0x64, true}},
					{0x15B2D7326_g, {0x64, true}},
					{0x15B4ABC16_g, {0x5C, true}},

					{0x15EF9A75B_g, {0x54, true}},
					{0x15E5FD1E9_g, {0x58, true}},
					{0x15CA3965D_g, {0x58, true}},
					{0x15F3397B2_g, {0x64, true}},
					{0x15DCB1A2C_g, {0x50, true}},
					{0x15F6B4001_g, {0x5C, true}},
					{0x15BE521F1_g, {0x5C, true}},

					{0x15F6AE373_g, {0x54, true}},
					{0x15C4CACB2_g, {0x54, true}},
					{0x15EF6AF3F_g, {0x5C, true}},
					{0x15BB1DAB8_g, {0x5C, true}},
					{0x15D1C5569_g, {0x50, false}}, // -> 15EAE384F, 15EA9C476
					{0x15BF469D7_g, {0x5C, true}},
					{0x15F9801A6_g, {0x60, false}}, // -> 15C2B8D85, 15D93EF1E
					{0x15B5C48A3_g, {0x60, true}},
					{0x15E49ABC2_g, {0x64, true}},
					//{0x_g, {0x0, true}},

					// OLD

					{0x15C0EDE96_g, {0x58, false}}, // -> 15D390285, 15CE36ADD, 15BDEC8E2
					{0x15F0643FD_g, {0x58, false}}, // -> 14219FDD3, 15E4EBF68, 15C02F8BD -> 4FF3DF01
					{0x142306BD9_g, {0x50, false}}, // -> 15B923CD2, 15B7F5204, 15EFBB4C9

					{0x15BEA0BE8_g, {0x60, false}}, // -> 15B2FB386, 15D0379AC
					{0x15CC4E93C_g, {0x60, false}}, // -> 15CCDC12E, 15DB862E9
					{0x15C2BB16B_g, {0x50, false}}, // -> 15EEC0E00, 15F5CA333
					{0x15C34F5BB_g, {0x58, false}}, // -> 15D7FFF48, 15CAAB671
					{0x15EA91E38_g, {0x50, false}}, // -> 15E54D475, 15F5C90B6 -> 4B7F42B5

					{0x15C998E3C_g, {0x5C, false}}, // ?
					{0x15EA018CD_g, {0x5C, false}}, // ?
					{0x15C591543_g, {0x54, false}}, // ?
					{0x15F9ADF78_g, {0x6C, false}}, // ?

					{0x15D5F91F7_g, {0x58, false}}, // <- 0x15F0643FD
					{0x15D5F91E4_g, {0x58, false}}, // <- 0x15F0643FD
				};

				if (!addr && lulul.contains(info->ContextRecord->Rip) && lulul.at(info->ContextRecord->Rip).second)
				{
					addr = info->ContextRecord->Rbp + lulul.at(info->ContextRecord->Rip).first;

					OutputDebugStringA(utils::string::va("Begin Trace: %llX -> %llX", info->ContextRecord->Rip, addr));
					utils::hardware_breakpoint::deactivate_all(*info->ContextRecord);
					activate(addr, 4, utils::hardware_breakpoint::read_write, *info->ContextRecord);
					//activate(0x15E4EBFAA_g, 1, utils::hardware_breakpoint::execute, *info->ContextRecord);

					//info->ContextRecord->Rax = 0xC3;
					//utils::hook::nop(0x15E4EBFA6_g, 4);
					//utils::hook::nop(0x15EA17E28_g, 4);

					return EXCEPTION_CONTINUE_EXECUTION;
				}
				else if (!lulul.contains(info->ContextRecord->Rip))
				{
					if (addr)
					{
						static std::unordered_set<uint64_t> aaa;
						if (aaa.emplace(info->ContextRecord->Rip).second)
						{
							/*if (info->ContextRecord->Rip == 0x14219FDD3_g)
							{
								OutputDebugStringA(utils::string::va("OOO: %X - %llX | %llX",
								                                     *(uint32_t*)addr, info->ContextRecord->Rcx,
								                                     0x1421E54A7_g + info->ContextRecord->Rcx * 4));
								*(uint32_t*)addr = 0x4FF3DF01;
								//info->ContextRecord->Rax = 0x4FF3DF01;
								utils::hardware_breakpoint::deactivate_all(*info->ContextRecord);
							}*/
							if (info->ContextRecord->Rip == 0x15E4EBF68_g)
							{
								auto other = info->ContextRecord->Rbx + info->ContextRecord->Rcx * 4;
								OutputDebugStringA(utils::string::va("Singlestep: %llX | %X | %X | %llX",
								                                     info->ContextRecord->Rip,
								                                     *(uint32_t*)addr, *(uint32_t*)other, other));
								utils::hardware_breakpoint::deactivate_all(*info->ContextRecord);
							}
							else
							{
								OutputDebugStringA(utils::string::va("Singlestep: %llX | %X",
								                                     info->ContextRecord->Rip, *(uint32_t*)addr));
							}
						}
					}
					else
					{
						static std::unordered_set<uint64_t> bbb;
						if (!lulul.contains(info->ContextRecord->Rip) && bbb.emplace(info->ContextRecord->Rip).second)
						{
							OutputDebugStringA(utils::string::va("Singlestep: %llX", info->ContextRecord->Rip));
						}
					}
				}

				return EXCEPTION_CONTINUE_EXECUTION;
			}


			if (info->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
			{
				if (!analysis_context.needs_protect_change)
				{
					return EXCEPTION_CONTINUE_SEARCH;
				}

				analysis_context.needs_protect_change = false;

				if (!analysis_context.had_single_step)
				{
					info->ContextRecord->EFlags &= 0x0100;
				}

				protect_texts();
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			if (info->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
			{
				// Was write?
				if (info->ExceptionRecord->ExceptionInformation[0] != 1)
				{
					return EXCEPTION_CONTINUE_SEARCH;
				}

				const auto addr = info->ExceptionRecord->ExceptionInformation[1];
				if (!was_in_texts(addr))
				{
					return EXCEPTION_CONTINUE_SEARCH;
				}

				analysis_context.needs_protect_change = true;
				analysis_context.had_single_step = info->ContextRecord->EFlags & 0x0100;
				info->ContextRecord->EFlags |= 0x0100;

				const auto data = *reinterpret_cast<uint32_t*>(info->ContextRecord->Rip);
				if ((data & 0xFFFFFF) == 0x8A0489)
				{
					auto handler = integrity_handlers.find(info->ContextRecord->Rip);
					const auto contains = handler != integrity_handlers.end();

					if (patchMode)
					{
						unprotect_texts();

						if (!contains)
						{
							OutputDebugStringA("Unknown handler executed :(");
							return EXCEPTION_CONTINUE_EXECUTION;
						}

						const auto old = info->ContextRecord->Rax;

						auto h = (integrity_handler_context*)(info->ContextRecord->Rbp + handler->second.frame_offset);
						*h->computed_checksum = *h->original_checksum;
						info->ContextRecord->Rax = *h->original_checksum;

						if (old != *h->original_checksum)
						{
							//once = true;
							OutputDebugStringA(utils::string::va("Adjusted wrong checksum: %X -> %X", old, handler->second.checksum));
						}
						else
						{
								OutputDebugStringA("Nothing to adjust");
						}

						return EXCEPTION_CONTINUE_EXECUTION;
					}

					if (!contains)
					{
						auto checksum = static_cast<uint32_t>(info->ContextRecord->Rax);
						auto offset = static_cast<int64_t>(info->ContextRecord->Rcx);
						if (offset != 0)
						{
							OutputDebugStringA(utils::string::va("OFFSET: %X\n", offset));
						}

						std::vector<uint32_t> discoveries{};

						for (uint32_t stack = 0x30; stack < 0x80; stack += 8)
						{
							auto* value_ptr = *(uint32_t**)(info->ContextRecord->Rbp + stack);
							auto* _addr = &value_ptr[offset];
							if (IsBadReadPtr(_addr, 4) || *_addr != checksum)
							{
								continue;
							}

							discoveries.push_back(stack);
						}

						if (discoveries.size() != 2)
						{
							OutputDebugStringA(utils::string::va(
								"!!! Unknown handler: %llX - Checksum: %X | rbp: %llX - offset: %X | discoveries: %zX",
								info->ContextRecord->Rip,
								checksum, info->ContextRecord->Rbp, offset, discoveries.size()));

							for (auto discovery : discoveries)
							{
								OutputDebugStringA(utils::string::va(
									"%X --> %llX",
									discovery, *(uint32_t**)(info->ContextRecord->Rbp + discovery) + offset));
							}
						}
						else
						{
							uint32_t* value_ptrs[] = {
								*(uint32_t**)(info->ContextRecord->Rbp + discoveries[0]) + offset,
								*(uint32_t**)(info->ContextRecord->Rbp + discoveries[1]) + offset
							};

							auto diff_0 = std::abs((int64_t)info->ContextRecord->Rbp - (int64_t)value_ptrs[0]);
							auto diff_1 = std::abs((int64_t)info->ContextRecord->Rbp - (int64_t)value_ptrs[1]);

							auto store_index = diff_0 < diff_1 ? 0 : 1;
							auto other_index = 1 - store_index;

							OutputDebugStringA(utils::string::va(
								"Handler: %llX\t| Checksum: %X\t| Checksum in memory: %llX\t(%X)\t| Calculated checksum location: %llX\t(%X)",
								info->ContextRecord->Rip,
								checksum, value_ptrs[other_index], discoveries[other_index], value_ptrs[store_index],
								discoveries[store_index]));

							integrity_handler_data h{};
							h.address = reverse_g(info->ContextRecord->Rip);
							h.checksum = checksum;
							h.checksum_address = reverse_g(value_ptrs[other_index]);
							h.frame_offset = discoveries[store_index];

							integrity_handlers[info->ContextRecord->Rip] = h;
							write_handlers();
						}
					}
				}
				/*	OutputDebugStringA(utils::string::va("Switch at: %llX -> %llX (%llX -> %llX)", addr,
					                                     reverse_g(addr),
					                                     info->ContextRecord->Rip,
					                                     reverse_g(info->ContextRecord->Rip)));*/

				unprotect_texts();
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

	class component final : public component_interface
	{
	public:
		component()
		{
#ifndef NDEBUG
			auto* dll_characteristics = &utils::nt::library().get_optional_header()->DllCharacteristics;
			utils::hook::set<WORD>(dll_characteristics, *dll_characteristics | IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE);
#endif
		}

		void pre_start() override
		{
			disable_tls_callbacks();
			restore_debug_functions();

			hide_being_debugged();
			scheduler::loop(hide_being_debugged, scheduler::pipeline::async);

			load_handlers();

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
			auto* get_cmd_import = utils::nt::library{}.get_iat_entry("kernel32.dll", "GetCommandLineA");
			if (get_cmd_import) utils::hook::set(get_cmd_import, get_command_line_a_stub);

			//zw_terminate_process_hook.create(ntdll.get_proc<void*>("ZwTerminateProcess"), zw_terminate_process_stub);
			//zw_terminate_process_hook.move();

			//auto* gpafc = ntdll.get_proc<void*>("LdrGetProcedureAddressForCaller");
			//get_proc_address_hook.create(gpafc, get_proc_address_stub);
			//get_proc_address_hook.move();
		}

		void post_unpack() override
		{
			//restore_debug_functions();
			/*
			MessageBoxA(0, "done", 0, 0);
			*/

			std::thread([]()
			{
				MessageBoxA(0, 0, 0, 0);
				protect_texts();
				MessageBoxA(0, "PATCH?", 0, 0);
				patchMode = true;
				utils::hook::set<uint8_t>(0x1423339C0_g, 0xC3);

				constexpr auto rdx_rbx = 0xda894890;
			constexpr auto rcx_rdx = 0xd1894890;
			constexpr auto rax_rcx = 0xc8894890;
			constexpr auto rbx_rax = 0xc3894890;

			utils::hook::nop(0x142AA20A1_g, 4);
			utils::hook::set<uint32_t>(0x15BDEC91F_g, rdx_rbx);

			utils::hook::nop(0x15E4EBFA6_g, 4);
			utils::hook::set<uint32_t>(0x15EA17E28_g, rcx_rdx);

			utils::hook::nop(0x15B7F5209_g, 6);
			utils::hook::set<uint32_t>(0x15EFBB508_g, rbx_rax);

			utils::hook::set<uint32_t>(0x15D0379CC_g, rdx_rbx);
			utils::hook::set<uint32_t>(0x15D1177B8_g, rcx_rdx);
			utils::hook::set<uint32_t>(0x15BFFF30D_g, rdx_rbx);
			utils::hook::set<uint32_t>(0x15DE3AAE7_g, rax_rcx);
			utils::hook::set<uint32_t>(0x15E48F80C_g, rbx_rax);
			}).detach();


			auto tid = GetCurrentThreadId();
			//activate(0x15D5F922A_g, 1, utils::hardware_breakpoint::execute, tid);
			//activate(0x15BCA4A2A_g, 1, utils::hardware_breakpoint::execute, tid);
			//activate(0x15C4BC192_g, 1, utils::hardware_breakpoint::execute, tid);
			//activate(0x15DA1FBA1_g, 1, utils::hardware_breakpoint::execute, tid); // <--

			//activate(0x15DB61C4A_g, 1, utils::hardware_breakpoint::execute, tid);
			//activate(0x15BE5BC6D_g, 1, utils::hardware_breakpoint::execute, tid); // <--

			//activate(0x15B88A716_g, 1, utils::hardware_breakpoint::execute, tid);
			//activate(0x15DBD74A0_g, 1, utils::hardware_breakpoint::execute, tid); // <--

			//activate(0x15CA2514F_g, 1, utils::hardware_breakpoint::execute, tid); // <--
			//activate(0x15CD0B431_g, 1, utils::hardware_breakpoint::execute, tid);

			//activate(0x15B8447FF_g, 1, utils::hardware_breakpoint::execute, tid);
			//activate(0x15C74FE9C_g, 1, utils::hardware_breakpoint::execute, tid);

			OutputDebugStringA("Trigger...");
			//activate(0x1423339C0_g, 1, utils::hardware_breakpoint::read_write, tid);
			//activate(0x15D0379D0_g, 1, utils::hardware_breakpoint::execute, tid);

			/*
			activate(0x142AA20A1_g, 1, utils::hardware_breakpoint::read_write, tid);
			activate(0x15BDEC91F_g, 1, utils::hardware_breakpoint::read_write, tid);
			activate(0x15E4EBFA6_g, 1, utils::hardware_breakpoint::read_write, tid);
			activate(0x15EA17E28_g, 1, utils::hardware_breakpoint::read_write, tid);
			*/
			//activate(0x15B7F5209_g, 4, utils::hardware_breakpoint::read_write, tid);
			//activate(0x15EFBB508_g, 4, utils::hardware_breakpoint::read_write, tid);
			//activate(0x15D0379CC_g, 4, utils::hardware_breakpoint::read_write, tid);
			//activate(0x15D1177B8_g, 4, utils::hardware_breakpoint::read_write, tid);

			// Some integrity check patches. More to come.

			/*constexpr auto rdx_rbx = 0xda894890;
			constexpr auto rcx_rdx = 0xd1894890;
			constexpr auto rax_rcx = 0xc8894890;
			constexpr auto rbx_rax = 0xc3894890;

			utils::hook::nop(0x142AA20A1_g, 4);
			utils::hook::set<uint32_t>(0x15BDEC91F_g, rdx_rbx);

			utils::hook::nop(0x15E4EBFA6_g, 4);
			utils::hook::set<uint32_t>(0x15EA17E28_g, rcx_rdx);

			utils::hook::nop(0x15B7F5209_g, 6);
			utils::hook::set<uint32_t>(0x15EFBB508_g, rbx_rax);

			utils::hook::set<uint32_t>(0x15D0379CC_g, rdx_rbx);
			utils::hook::set<uint32_t>(0x15D1177B8_g, rcx_rdx);
			utils::hook::set<uint32_t>(0x15BFFF30D_g, rdx_rbx);
			utils::hook::set<uint32_t>(0x15DE3AAE7_g, rax_rcx);
			utils::hook::set<uint32_t>(0x15E48F80C_g, rbx_rax);*/

			/*std::thread([tid]()
			{
				MessageBoxA(0, 0, 0, 0);
				//utils::hook::set<uint8_t>(0x1423339C0_g, 0xC3);
				OutputDebugStringA("Logging..\n");
				activate(0x1423339C0_g, 1, utils::hardware_breakpoint::read_write, tid);
			}).detach();*/
		}

		void pre_destroy() override
		{
			utils::hook::copy(GetWindowTextA, this->window_text_buffer_, sizeof(this->window_text_buffer_));
			nt_query_system_information_hook.clear();
			nt_query_information_process_hook.clear();
			nt_close_hook.clear();
			create_mutex_ex_a_hook.clear();
			create_thread_hook.clear();
			open_process_hook.clear();
			get_thread_context_hook.clear();
			zw_terminate_process_hook.clear();
			get_proc_address_hook.clear();
		}

		int priority() override
		{
			return 9999;
		}

	private:
		uint8_t window_text_buffer_[15]{};
	};
}

REGISTER_COMPONENT(arxan::component)
