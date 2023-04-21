#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "auth.hpp"

#include "steam/steam.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace colors
{
	namespace
	{
		utils::hook::detour cl_get_client_name_hook;

		std::optional<int> get_color_for_xuid(const uint64_t xuid)
		{
			if (xuid == 0xCD02AF6448291209
				|| xuid == 0x10F0C433E08E1357
				|| xuid == 0x60E0FEFE42341715)
			{
				return 2;
			}

			return {};
		}

		std::optional<int> get_color_for_client(const int client_num)
		{
			const auto xuid = auth::get_guid(static_cast<size_t>(client_num));
			return get_color_for_xuid(xuid);
		}

		template <size_t index>
		void patch_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255)
		{
			auto* color_table = reinterpret_cast<uint8_t*>(0x142FEFE20_g);
			auto* g_color_table = reinterpret_cast<float*>(0x142FB5CF0_g);

			uint8_t color_int[4];
			color_int[0] = r;
			color_int[1] = g;
			color_int[2] = b;
			color_int[3] = a;

			utils::hook::copy(color_table + index * 4, color_int, sizeof(color_int));

			float color_float[4];
			color_float[0] = static_cast<float>(r) / 255.0f;
			color_float[1] = static_cast<float>(g) / 255.0f;
			color_float[2] = static_cast<float>(b) / 255.0f;
			color_float[3] = static_cast<float>(a) / 255.0f;

			utils::hook::copy(g_color_table + index * 4, color_float, sizeof(color_float));
		}

		bool cl_get_client_name_stub(const int local_client_num, const int index, char* buf, const int size,
		                             const bool add_clan_name)
		{
			const auto res = cl_get_client_name_hook.invoke<bool>(local_client_num, index, buf, size, add_clan_name);

			if (_ReturnAddress() == reinterpret_cast<void*>(0x1406A7B56_g))
			{
				return res;
			}

			const auto color = get_color_for_client(index);
			if (!color)
			{
				return res;
			}

			const auto val = utils::string::va("^%d%s", *color, buf);
			utils::string::copy(buf, size, val);

			return res;
		}

		/*const char* get_gamer_tag_stub(const uint32_t num)
		{
			const auto color = get_color_for_xuid(steam::SteamUser()->GetSteamID().bits);
			const auto name = reinterpret_cast<const char* (*)(uint32_t)>(0x141EC6E80)(num) + 8;

			if (!color || num)
			{
				return name;
			}

			return utils::string::va("^1%s", *color, name);
		}*/
	}

	struct component final : client_component
	{
		void post_unpack() override
		{
			patch_color<1>(255, 49, 49); // 1  - Red
			patch_color<2>(134, 192, 0); // 2  - Green
			patch_color<3>(255, 173, 34); // 3  - Yellow
			patch_color<4>(0, 135, 193); // 4  - Blue
			patch_color<5>(32, 197, 255); // 5  - Light Blue
			patch_color<6>(151, 80, 221); // 6  - Pink

			// Old addresses
			cl_get_client_name_hook.create(game::CL_GetClientName, cl_get_client_name_stub);
			//utils::hook::jump(0x141EC72E0_g, get_gamer_tag_stub);
		}
	};
}

REGISTER_COMPONENT(colors::component)
