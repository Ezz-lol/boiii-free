#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include <game/game.hpp>

#include <utils/hook.hpp>

namespace status
{
	namespace
	{
		int client_num_;

		void print_client_num(int channel, int label, const char* fmt, const int client_num)
		{
			client_num_ = client_num;
			game::Com_Printf(channel, label, fmt, client_num);
		}

		void print_client_xuid(int channel, int label, [[maybe_unused]] const char* fmt, const uint64_t xuid)
		{
			if (game::SV_IsTestClient(client_num_))
			{
				game::Com_Printf(channel, label, "%16s ", "bot0");
				return;
			}

			game::Com_Printf(channel, label, "%12llx ", xuid);
		}
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			// Patch the status command for test clients
			utils::hook::call(game::select(0x142246E37, 0x14052C527), print_client_num);
			utils::hook::call(game::select(0x142246EDE, 0x14052C5CE), print_client_xuid);

			utils::hook::copy_string(game::select(0x143050480, 0x140E85A20),
			                         "num score ping xuid             name             address                  qport  \n");
			utils::hook::copy_string(game::select(0x1430504E0, 0x140E85A80),
			                         "--- ----- ---- ---------------- ---------------- ------------------------ ------ \n");
		}
	};
}

REGISTER_COMPONENT(status::component)
