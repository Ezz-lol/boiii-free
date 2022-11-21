#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "version.hpp"

#include "scheduler.hpp"

#include <utils/hook.hpp>

#include "utils/string.hpp"

namespace branding
{
	namespace
	{
		std::map<int, std::string> connectivity_bits = {
			{0x10000000, "???"},
			{0x01000000, "LiveInventory_IsValid"},
			{0x02000000, "LiveStorage_AreDDLsInSync"},
			{0x04000000, "LiveInventory_InitialCODPointsChecked"},
			{0x08000000, "Marketing_HaveMessages"},
			{0x00100000, "LivePublisherVariables_AreVariablesAvailable"},
			{0x00200000, "SaveGame_IsDataReady(controllerIndex, MODE_NETWORK_ONLINE)"},
			{0x00400000, "LPC_IsLPCReady() || !Dvar_GetBool(live_useLPC)"},
			{0x00800000, "LiveAntiCheat_ConsoleDetailsReported"},
			{0x00010000, "Live_Qos_Finished"},
			{0x00080000, "???"},
			{0x00002000, "Live_IsGeoLocationDataRetrieved"},
			{0x00000100, "LiveStorage_DoWeHavePlaylists"},
			{0x00000200, "LiveStorage_ValidateFFOTD"},
			{0x00000010, "LiveStats_Loadouts_Ready(controllerIndex, MODE_NETWORK_ONLINE)"},
			{0x00000040, "g_dwNetStatus == 6"},
			{0x00000080, "LiveStorage_DoWeHaveFFOTD"},
			{0x00000002, "LiveUser_IsUserSignedInToLive"},
			{0x00000004, "Live_IsUserSignedInToDemonware"},
			{0x00000008, "LiveStats_Core_Ready(controllerIndex, MODE_NETWORK_ONLINE)"},
		};

		std::string get_connectivity_info()
		{
			int bits = 0;
			game::Live_GetConnectivityInformation(0, &bits, false);
			std::string str = utils::string::va("Bits: %X\n", bits);

			int required_mask = 0x1FF923DE;
			for (int i = 0; (1 << i) <= 0x10000000; ++i)
			{
				auto value = 1 << i;

				const bool set = bits & value;
				const bool req = required_mask & value;
				if (!req) continue;

				std::string name = "?";

				auto entry = connectivity_bits.find(value);
				if (entry != connectivity_bits.end())
				{
					name = entry->second;
				}

				str += utils::string::va("%08X - %d - %s\n", value, static_cast<int>(set), name.data());
			}

			return str;
		}

		void draw_branding()
		{
			constexpr auto x = 4;
			constexpr auto y = 0;
			constexpr auto scale = 0.45f;
			float color[4] = {0.666f, 0.666f, 0.666f, 0.666f};

			auto* font = reinterpret_cast<uint32_t*(*)()>(0x141CAC8E0_g)();
			if (!font) return;

			std::string str = "BOIII: " VERSION;
			//str += "\n\n" + get_connectivity_info();
			game::R_AddCmdDrawText(str.data(), std::numeric_limits<int>::max(), font, static_cast<float>(x),
			                       y + static_cast<float>(font[2]) * scale,
			                       scale, scale, 0.0f, color, game::ITEM_TEXTSTYLE_NORMAL);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scheduler::loop(draw_branding, scheduler::renderer);
		}
	};
}

REGISTER_COMPONENT(branding::component)
