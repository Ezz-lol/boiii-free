#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>
#include <version.hpp>

namespace branding
{
	namespace
	{
		utils::hook::detour r_end_frame_hook;

		void draw_branding()
		{
			constexpr auto x = 4;
			constexpr auto y = 0;
			constexpr auto scale = 0.5f;
			//float color[4] = {0.666f, 0.666f, 0.666f, 0.666f};
			float color[4] = {236 / 255.0f, 113 / 255.0f, 10 / 255.0f, 1.0f};

			auto* font = reinterpret_cast<uint32_t*(*)()>(0x141CAC8E0_g)();
			if (!font) return;

			game::R_AddCmdDrawText("BOIII: " VERSION, 0x7FFFFFFF, font, static_cast<float>(x),
			                       y + static_cast<float>(font[2]) * scale,
			                       scale, scale, 0.0f, color, game::ITEM_TEXTSTYLE_SHADOWEDMORE);
		}

		void r_end_frame_stub()
		{
			draw_branding();
			r_end_frame_hook.invoke<void>();
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			r_end_frame_hook.create(0x142273560_g, r_end_frame_stub);
		}
	};
}

REGISTER_COMPONENT(branding::component)
