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
		utils::hook::detour liveinventory_getitemquantity_hook;
		utils::hook::detour liveinventory_areextraslotspurchased_hook;

		int loot_getitemquantity_stub(const game::ControllerIndex_t controllerIndex, const game::eModes mode, const int itemId)
		{
			if (mode == game::eModes::MODE_ZOMBIES) {
				return 999;
			}
			return 1;
		}

		int liveinventory_getitemquantity_stub(const game::ControllerIndex_t controllerIndex, const int itemId)
		{
			// Item id's for extra CaC slots, CWL camo's and paid specialist outfits
			if (itemId == 99003 || itemId >= 99018 && itemId <= 99021 || itemId == 99025 || itemId >= 90047 && itemId <= 90064) {
				return 1;
			}

			return liveinventory_getitemquantity_hook.invoke<int>(controllerIndex, itemId);
		}

		bool liveinventory_areextraslotspurchased_stub(const game::ControllerIndex_t controllerIndex)
		{
			return true;
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
			liveinventory_getitemquantity_hook.create(0x141E090C0_g, liveinventory_getitemquantity_stub);
			liveinventory_areextraslotspurchased_hook.create(0x141E089E0_g, liveinventory_areextraslotspurchased_stub);
		}
	};
};


REGISTER_COMPONENT(loot::component)