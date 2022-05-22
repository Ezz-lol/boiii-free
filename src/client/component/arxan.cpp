#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>

namespace arxan
{
	namespace
	{
		DWORD get_steam_pid()
		{
			static auto steam_pid = [] {
			  HKEY hRegKey;
			  DWORD pid{};

			  if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam\\ActiveProcess", 0, KEY_QUERY_VALUE,
			    &hRegKey) != ERROR_SUCCESS)
			    return pid;

			  DWORD dwLength = sizeof(pid);
			  RegQueryValueExA(hRegKey, "pid", nullptr, nullptr, reinterpret_cast<BYTE*>(&pid), &dwLength);
			  RegCloseKey(hRegKey);

			  return pid;
			}();

			return steam_pid;
		}

		utils::hook::detour nt_close_hook;
		utils::hook::detour nt_query_information_process_hook;
		utils::hook::detour create_mutex_ex_a_hook;

		HANDLE create_mutex_ex_a_stub(const LPSECURITY_ATTRIBUTES attributes, const LPCSTR name, const DWORD flags, const DWORD access)
		{
			if (name == "$ IDA trusted_idbs"s || name == "$ IDA registry mutex $"s)
			{
				return nullptr;
			}

			return create_mutex_ex_a_hook.invoke<HANDLE>(attributes, name, flags, access);
		}

		NTSTATUS WINAPI nt_query_information_process_stub(const HANDLE handle, const PROCESSINFOCLASS info_class,
			const PVOID info,
			const ULONG info_length, const PULONG ret_length)
		{
			auto* orig = static_cast<decltype(NtQueryInformationProcess)*>(nt_query_information_process_hook.
				get_original());
			const auto status = orig(handle, info_class, info, info_length, ret_length);

			if (NT_SUCCESS(status))
			{
				if (info_class == ProcessBasicInformation)
				{
					static DWORD explorer_pid = 0;
					if (!explorer_pid)
					{
						auto* const shell_window = GetShellWindow();
						GetWindowThreadProcessId(shell_window, &explorer_pid);
					}

					static_cast<PPROCESS_BASIC_INFORMATION>(info)->Reserved3 = PVOID(DWORD64(get_steam_pid()));
				}
				else if (info_class == 30) // ProcessDebugObjectHandle
				{
					*static_cast<HANDLE*>(info) = nullptr;

					return 0xC0000353;
				}
				else if (info_class == 7) // ProcessDebugPort
				{
					*static_cast<HANDLE*>(info) = nullptr;
				}
				else if (info_class == 31)
				{
					*static_cast<ULONG*>(info) = 1;
				}
			}

			return status;
		}

		NTSTATUS NTAPI nt_close_stub(const HANDLE handle)
		{
			char info[16];
			if (NtQueryObject(handle, OBJECT_INFORMATION_CLASS(4), &info, 2, nullptr) >= 0 && size_t(handle) != 0x12345)
			{
				auto* orig = static_cast<decltype(NtClose)*>(nt_close_hook.get_original());
				return orig(handle);
			}

			return STATUS_INVALID_HANDLE;
		}

		LONG WINAPI exception_filter(const LPEXCEPTION_POINTERS info)
		{
			if (info->ExceptionRecord->ExceptionCode == STATUS_INVALID_HANDLE)
			{
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			return EXCEPTION_CONTINUE_SEARCH;
		}

		void hide_being_debugged()
		{
			auto* const peb = PPEB(__readgsqword(0x60));
			peb->BeingDebugged = false;
			*reinterpret_cast<PDWORD>(LPSTR(peb) + 0xBC) &= ~0x70;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			hide_being_debugged();
			scheduler::loop(hide_being_debugged, scheduler::pipeline::async);

			create_mutex_ex_a_hook.create(CreateMutexExA, create_mutex_ex_a_stub);

			const utils::nt::library ntdll("ntdll.dll");
			nt_close_hook.create(ntdll.get_proc<void*>("NtClose"), nt_close_stub);
			nt_query_information_process_hook.create(ntdll.get_proc<void*>("NtQueryInformationProcess"),
				nt_query_information_process_stub);

			AddVectoredExceptionHandler(1, exception_filter);
		}
	};
}

REGISTER_COMPONENT(arxan::component)
