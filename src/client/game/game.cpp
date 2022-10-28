#include <std_include.hpp>

#include "loader/component_loader.hpp"
#include "game.hpp"

namespace game
{
	// inlined in cod, functionality stolen from https://github.com/id-Software/Quake-III-Arena/blob/master/code/win32/win_syscon.c#L520
	int Conbuf_CleanText(const char* source, char* target)
	{
		char* b = target;
		int i = 0;
		while (source[i] && ((b - target) < sizeof(target) - 1))
		{
			if (source[i] == '\n' && source[i + 1] == '\r')
			{
				b[0] = '\r';
				b[1] = '\n';
				b += 2;
				i++;
			}
			else if (source[i] == '\r' || source[i] == '\n')
			{
				b[0] = '\r';
				b[1] = '\n';
				b += 2;
			}
			else if (source && source[0] == '^' && source[1] && source[1] != '^' && source[1] >= 48 && source[1] <= 64)
			// Q_IsColorString
			{
				i++;
			}
			else
			{
				*b = source[i]; // C6011 here, should we be worried?
				b++;
			}
			i++;
		}

		*b = 0;
		return static_cast<int>(b - target);
	}

	game::eModes Com_SessionMode_GetMode()
	{
		return game::eModes(*reinterpret_cast<uint32_t*>(0x1568EF7F4_g) << 28 >> 28);
	}
}
