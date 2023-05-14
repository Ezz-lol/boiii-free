#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include <game/game.hpp>

#include "game_event.hpp"

#include <utils/hook.hpp>

namespace game_event
{
	namespace
	{
		std::vector<std::function<void()>> g_init_game_tasks_;
		std::vector<std::function<void()>> g_shutdown_game_tasks_;

		void rope_init_ropes_stub()
		{
			for (const auto& func : g_init_game_tasks_)
			{
				func();
			}

			game::Rope_InitRopes();
		}

		void mantle_shutdown_anims_stub()
		{
			for (const auto& func : g_init_game_tasks_)
			{
				func();
			}

			game::Mantle_ShutdownAnims();
		}
	}

	void on_g_init_game(const std::function<void()>& callback)
	{
		g_init_game_tasks_.emplace_back(callback);
	}

	void on_g_shutdown_game(const std::function<void()>& callback)
	{
		g_shutdown_game_tasks_.emplace_back(callback);
	}

	class component final : public generic_component
	{
	public:
		void post_unpack() override
		{
			utils::hook::call(game::select(0x1419DD6EC, 0x1402ABC1B), rope_init_ropes_stub);
			utils::hook::call(game::select(0x141A02AAD, 0x1402ADF1D), rope_init_ropes_stub);
		}
	};
}

REGISTER_COMPONENT(game_event::component)
