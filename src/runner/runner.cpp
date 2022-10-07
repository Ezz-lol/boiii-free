#include <utils/nt.hpp>
#include <cstdlib>

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	const auto* const command = "-proc ";
	const char* parent_proc = strstr(GetCommandLineA(), command);

	if (parent_proc)
	{
		const auto pid = DWORD(atoi(parent_proc + strlen(command)));
		const utils::nt::handle<> process_handle = OpenProcess(SYNCHRONIZE, FALSE, pid);
		if (process_handle)
		{
			WaitForSingleObject(process_handle, INFINITE);
			return 0;
		}
	}

	return 1;
}
