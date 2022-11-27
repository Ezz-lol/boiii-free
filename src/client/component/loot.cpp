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
		void set_dvars_on_startup()
		{
			game::Dvar_SetFromStringByName("ui_enableAllHeroes", "1", true);
			game::Dvar_SetFromStringByName("allItemsUnlocked", "1", true);
			game::Dvar_SetFromStringByName("allItemsPurchased", "1", true);
			game::Dvar_SetFromStringByName("ui_allLootUnlocked", "1", true);
		}
	};

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scheduler::once(set_dvars_on_startup, scheduler::pipeline::main);
		}
	};
};


REGISTER_COMPONENT(loot::component)