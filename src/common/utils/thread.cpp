#include "thread.hpp"
#include "string.hpp"
#include "finally.hpp"

#include <TlHelp32.h>

namespace utils::thread
{
	bool set_name(const HANDLE t, const std::string& name)
	{
		const nt::library kernel32("kernel32.dll");
		if (!kernel32)
		{
			return false;
		}

		const auto set_description = kernel32.get_proc<HRESULT(WINAPI *)(HANDLE, PCWSTR)>("SetThreadDescription");
		if (!set_description)
		{
			return false;
		}

		return SUCCEEDED(set_description(t, string::convert(name).data()));
	}

	bool set_name(const DWORD id, const std::string& name)
	{
		auto* const t = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, id);
		if (!t) return false;

		const auto _ = utils::finally([t]()
		{
			CloseHandle(t);
		});

		return set_name(t, name);
	}

	bool set_name(std::thread& t, const std::string& name)
	{
		return set_name(t.native_handle(), name);
	}

	bool set_name(const std::string& name)
	{
		return set_name(GetCurrentThread(), name);
	}

	std::vector<DWORD> get_thread_ids()
	{
		nt::handle<INVALID_HANDLE_VALUE> h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
		if (!h)
		{
			return {};
		}

		THREADENTRY32 entry{};
		entry.dwSize = sizeof(entry);
		if (!Thread32First(h, &entry))
		{
			return {};
		}

		std::vector<DWORD> ids{};

		do
		{
			const auto check_size = entry.dwSize < FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID)
				+ sizeof(entry.th32OwnerProcessID);
			entry.dwSize = sizeof(entry);

			if (check_size && entry.th32OwnerProcessID == GetCurrentProcessId())
			{
				ids.emplace_back(entry.th32ThreadID);
			}
		}
		while (Thread32Next(h, &entry));

		return ids;
	}

	void for_each_thread(const std::function<void(HANDLE)>& callback, const DWORD access)
	{
		const auto ids = get_thread_ids();

		for (const auto& id : ids)
		{
			handle thread(id, access);
			if (thread)
			{
				callback(thread);
			}
		}
	}

	void suspend_other_threads()
	{
		for_each_thread([](const HANDLE thread)
		{
			if (GetThreadId(thread) != GetCurrentThreadId())
			{
				SuspendThread(thread);
			}
		});
	}

	void resume_other_threads()
	{
		for_each_thread([](const HANDLE thread)
		{
			if (GetThreadId(thread) != GetCurrentThreadId())
			{
				ResumeThread(thread);
			}
		});
	}
}
