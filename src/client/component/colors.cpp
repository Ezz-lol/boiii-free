#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace colors
{
	namespace
	{
		utils::hook::detour get_player_name_hook;
		utils::hook::detour get_gamer_tag_hook;

		template <size_t index>
		void patch_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255)
		{
			auto* color_table = reinterpret_cast<uint8_t*>(0x142FF0E20_g);
			auto* g_color_table = reinterpret_cast<float*>(0x142FB6CF0_g);

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

		uint64_t get_player_name_stub(const uint64_t client, int client_num, char* buffer, const int size,
		                              const bool has_clan_tag)
		{
			const auto res = get_player_name_hook.invoke<uint64_t>(client, client_num, buffer, size, has_clan_tag);

			if (_ReturnAddress() != reinterpret_cast<void*>(0x1406A7B56_g))
			{
				const auto val = utils::string::va("^%d%s", rand() % 7, buffer);
				strncpy_s(buffer, size, val, size);
			}

			return res;
		}

		const char* get_gamer_tag_stub(const uint64_t num)
		{
			return utils::string::va("^3%s", get_gamer_tag_hook.invoke<const char*>(num));
		}
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

			//utils::hook::set<uint8_t>(0x141EB0457_g, 0xEB);
			//utils::hook::set<uint8_t>(0x141EB0350_g, 0xC3);

			//utils::hook::jump(game::select(0x14166F580, 0x140278840), clean_client_name_stub);
			//utils::hook::call(game::select(0x1413E31E6, 0), clean_str);

			get_player_name_hook.create(0x1413E3140_g, get_player_name_stub);
			get_gamer_tag_hook.create(0x141EC7370_g, get_gamer_tag_stub);
		}
	};
}

REGISTER_COMPONENT(colors::component)
