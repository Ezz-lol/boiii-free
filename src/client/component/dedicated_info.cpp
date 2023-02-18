#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/utils.hpp"
#include "scheduler.hpp"
#include "getinfo.hpp"
#include "console.hpp"

#include <utils/string.hpp>

namespace dedicated_info
{
	namespace
	{
		void set_server_info_in_console_title()
		{
			const auto sv_running = game::Dvar_FindVar("sv_running");
			const auto server_name = game::get_dvar_string("live_steam_server_name");

			if (!sv_running || !sv_running->current.enabled)
			{
				console::set_title(server_name + " - not running");
				return;
			}

			const auto mapname = game::get_dvar_string("mapname");

			const std::string window_text = utils::string::va("%s on %s [%d/%d] (%d)",
			                                                  server_name.data(),
			                                                  mapname.data(),
			                                                  getinfo::get_client_count(),
			                                                  getinfo::get_max_client_count(),
			                                                  getinfo::get_bot_count());

			console::set_title(window_text);
		}
	}

	class component final : public server_component
	{
	public:
		void post_unpack() override
		{
			scheduler::loop([]()
			{
				set_server_info_in_console_title();
			}, scheduler::pipeline::main, 1s);
		}
	};
}

REGISTER_COMPONENT(dedicated_info::component)
