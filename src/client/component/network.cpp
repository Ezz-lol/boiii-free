#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace network
{
	namespace
	{
		int net_compare_base_address(const game::netadr_t* a1, const game::netadr_t* a2)
		{
			if (a1->type == a2->type)
			{
				switch (a1->type)
				{
				case game::netadrtype_t::NA_BOT:
				case game::netadrtype_t::NA_LOOPBACK:
					return a1->port == a2->port;

				case game::netadrtype_t::NA_IP:
					return memcmp(&a1->ipv4.a, &a2->ipv4.a, 4) == 0;
				default:
					break;
				}
			}

			return false;
		}

		int net_compare_address(const game::netadr_t* a1, const game::netadr_t* a2)
		{
			return net_compare_base_address(a1, a2) && a1->port == a2->port;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// redirect dw_sendto to raw socket
			utils::hook::jump(0x14233307E_g, 0x1423330C7_g);

			// intercept command handling
			/*utils::hook::jump(0x14020A175, utils::hook::assemble(handle_command_stub), true);

			// handle xuid without secure connection
			utils::hook::nop(0x14043FFF8, 6);

			utils::hook::jump(0x1403DA700, net_compare_address);
			utils::hook::jump(0x1403DA750, net_compare_base_address);

			// don't establish secure conenction
			utils::hook::set<uint8_t>(0x140232BBD, 0xEB);
			utils::hook::set<uint8_t>(0x140232C9A, 0xEB);
			utils::hook::set<uint8_t>(0x140232F8D, 0xEB);
			utils::hook::set<uint8_t>(0x14020862F, 0xEB);

			// ignore unregistered connection
			utils::hook::jump(0x140439EA9, reinterpret_cast<void*>(0x140439E28));
			utils::hook::set<uint8_t>(0x140439E9E, 0xEB);

			// disable xuid verification
			utils::hook::set<uint8_t>(0x140022319, 0xEB);
			utils::hook::set<uint8_t>(0x140022334, 0xEB);

			// disable xuid verification
			utils::hook::nop(0x14043CC4C, 2);
			utils::hook::set<uint8_t>(0x14043CCA8, 0xEB);

			// ignore configstring mismatch
			utils::hook::set<uint8_t>(0x140211610, 0xEB);

			// ignore dw handle in SV_PacketEvent
			utils::hook::set<uint8_t>(0x140442F6D, 0xEB);
			utils::hook::call(0x140442F61, &net_compare_address);

			// ignore dw handle in SV_FindClientByAddress
			utils::hook::set<uint8_t>(0x14044256D, 0xEB);
			utils::hook::call(0x140442561, &net_compare_address);

			// ignore dw handle in SV_DirectConnect
			utils::hook::set<uint8_t>(0x140439BA8, 0xEB);
			utils::hook::set<uint8_t>(0x140439DA5, 0xEB);
			utils::hook::call(0x140439B9B, &net_compare_address);
			utils::hook::call(0x140439D98, &net_compare_address);
			*/
		}
	};
}

//REGISTER_COMPONENT(network::component)
