#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "network.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>

namespace party
{
	namespace
	{
		void connect_stub(const char* address)
		{
			auto addr = network::address_from_string(address);

			game::XSESSION_INFO info{};
			game::CL_ConnectFromLobby(0, &info, &addr, 1, 0, "mp_nuketown_x", "tdm");
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			utils::hook::jump(0x141EE6030_g, connect_stub);

			/*network::on("_pong", [](const game::netadr_t& source, const network::data_view& data)
			{
			
			});*/
		}
	};
}

REGISTER_COMPONENT(party::component)
