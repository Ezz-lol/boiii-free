#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/thread.hpp>

#include <component/scheduler.hpp>
#include "game/game.hpp"

namespace loot
{
	namespace
	{
		utils::hook::detour loot_getitemquantity_hook;

		int loot_getitemquantity_stub(const game::ControllerIndex_t controllerIndex, const game::eModes mode, const int itemId)
		{
			return 1;
		}

		void set_dvars_on_startup()
		{
			game::Dvar_SetFromStringByName("ui_enableAllHeroes", "1", true);
			game::Dvar_SetFromStringByName("ui_allLootUnlocked", "1", true);
		}
	};

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scheduler::once(set_dvars_on_startup, scheduler::pipeline::main);
			loot_getitemquantity_hook.create(0x141E82C90_g, loot_getitemquantity_stub);
		}
	};
};


REGISTER_COMPONENT(loot::component)