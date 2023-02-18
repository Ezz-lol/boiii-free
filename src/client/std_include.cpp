#include <std_include.hpp>

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language=''\"")

extern "C"
{
	int s_read_arc4random(void*, size_t)
	{
		return -1;
	}

	int s_read_getrandom(void*, size_t)
	{
		return -1;
	}

	int s_read_urandom(void*, size_t)
	{
		return -1;
	}

	int s_read_ltm_rng(void*, size_t)
	{
		return -1;
	}
}

extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 1;
	__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;
}
