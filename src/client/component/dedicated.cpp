#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace dedicated
{
	namespace
	{
	}

	struct component final : server_component
	{
		void post_unpack() override
		{
			// Ignore "bad stats"
			utils::hook::set<uint8_t>(0x14052D523_g, 0xEB);
			utils::hook::nop(0x14052D4E4_g, 2);
		}
	};
}

REGISTER_COMPONENT(dedicated::component)
