#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include "scheduler.hpp"

namespace dvars
{
	namespace
	{
		bool initial_config_read = false;
		utils::hook::detour dvar_register_new_hook;
		utils::hook::detour dvar_set_variant_hook;

		utils::hook::detour set_config_dvar_hook;
		utils::hook::detour for_each_name_match_hook;
		utils::hook::detour get_debug_name_hook;


		const std::string get_config_file_path()
		{
			return "players/user/config.cfg";
		}

		void write_archive_dvars()
		{
			std::string config_buffer;

			for (int i = 0; i < *game::g_dvarCount; ++i)
			{
				const auto* dvar = reinterpret_cast<const game::dvar_t*>(&game::s_dvarPool[160 * i]);

				if (!dvar->debugName || (game::dvarFlags_e::DVAR_SAVED & dvar->flags) == 0)
					continue;

				auto name = dvar->debugName;
				auto value = game::Dvar_DisplayableValue(dvar);

				config_buffer.append(utils::string::va("set %s %s\n", name, value));
			}

			if (config_buffer.length() == 0)
				return;

			utils::io::write_file(get_config_file_path(), config_buffer);
		}

		void dvar_set_variant_stub(game::dvar_t* dvar, game::DvarValue* value, unsigned int source)
		{
			dvar_set_variant_hook.invoke(dvar, value, source);

			if (initial_config_read && (game::dvarFlags_e::DVAR_SAVED & dvar->flags) != 0 && dvar->debugName)
			{
				write_archive_dvars();
			}
		}

		void read_archive_dvars()
		{
			const std::string path = get_config_file_path();

			if (!utils::io::file_exists(path))
				return;

			std::string filedata;
			utils::io::read_file(path, &filedata);

			game::Cbuf_ExecuteBuffer(0, game::ControllerIndex_t::CONTROLLER_INDEX_0, filedata.c_str());
			initial_config_read = true;
		}

		game::dvar_t* dvar_register_new_stub(game::dvarStrHash_t hash, const char* dvar_name, game::dvarType_t type, unsigned int flags,
			game::DvarValue value, game::DvarLimits domain, const char* description)
		{
			auto dvar = dvar_register_new_hook.invoke<game::dvar_t*>(hash, dvar_name, type, flags, value, domain, description);

			dvar->debugName = dvar_name; // TODO: gives access violation error

			return dvar;
		}
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			scheduler::once(read_archive_dvars, scheduler::pipeline::main);

			//dvar_register_new_hook.create(0x1422C5330_g, dvar_register_new_stub);
			dvar_set_variant_hook.create(0x1422C9A90_g, dvar_set_variant_stub);
		}
	};
}

REGISTER_COMPONENT(dvars::component)
