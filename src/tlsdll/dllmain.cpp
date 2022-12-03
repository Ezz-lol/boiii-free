#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#define TLS_PAYLOAD_SIZE 0x2000
thread_local char tls_data[TLS_PAYLOAD_SIZE];

__declspec(dllexport) void* get_tls_data()
{
	return &tls_data[0];
}

int WINAPI _DllMainCRTStartup(const HMODULE module, const unsigned long reason, void*)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);
	}

	return 1;
}
