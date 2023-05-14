#include <std_include.hpp>

#include "game.hpp"

namespace game
{
	bool I_islower(int c)
	{
		return c >= 'a' && c <= 'z';
	}

	bool I_isupper(int c)
	{
		return c >= 'A' && c <= 'Z';
	}

	unsigned int Scr_CanonHash(const char* str)
	{
#define FNV_OFFSET 0x4B9ACE2F
#define FNV_PRIME 16777619

		const auto* s = str;
		const int first_char = I_islower(*s) ? static_cast<unsigned char>(*s) : tolower(static_cast<unsigned char>(*str));

		unsigned int hash = FNV_PRIME * (first_char ^ FNV_OFFSET);
		while (*s)
		{
			int acc = I_islower(*++s)
				? static_cast<unsigned char>(*s)
				: std::tolower(static_cast<unsigned char>(*s));

			hash = FNV_PRIME * (acc ^ hash);
		}

		return hash;
	}
}
