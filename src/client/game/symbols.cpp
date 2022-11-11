#include <std_include.hpp>

#include "game.hpp"

namespace game
{
	eModes Com_SessionMode_GetMode()
	{
		return eModes(*reinterpret_cast<uint32_t*>(0x1568EF7F4_g) << 28 >> 28);
	}
}
