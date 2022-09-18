#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "version.hpp"

#include "scheduler.hpp"

#include <utils/hook.hpp>

namespace branding
{
	namespace
	{
		void draw_branding()
		{
			constexpr auto x = 4;
			constexpr auto y = 0;
			constexpr auto scale = 0.5f;
			float color[4] = {236 / 255.0f, 113 / 255.0f, 10 / 255.0f, 1.0f};

			auto* font = reinterpret_cast<uint32_t*(*)()>(0x141CAC8E0_g)();
			if (!font) return;

			game::R_AddCmdDrawText("BOIII: " VERSION, 0x7FFFFFFF, font, static_cast<float>(x),
			                       y + static_cast<float>(font[2]) * scale,
			                       scale, scale, 0.0f, color, game::ITEM_TEXTSTYLE_SHADOWEDMORE);
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
