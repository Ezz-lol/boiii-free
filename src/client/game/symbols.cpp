#include <std_include.hpp>

#include "game.hpp"

namespace game
{
	eModes Com_SessionMode_GetMode()
	{
		return eModes(*reinterpret_cast<uint32_t*>(game::select(0x1568EF7F4, 0x14948DB04)) << 28 >> 28);
	}
}
