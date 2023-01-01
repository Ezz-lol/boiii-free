#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "scheduler.hpp"
#include "game/game.hpp"

namespace loot
{
	namespace
	{
		utils::hook::detour loot_getitemquantity_hook;
		utils::hook::detour liveinventory_getitemquantity_hook;
		utils::hook::detour liveinventory_areextraslotspurchased_hook;

		int loot_getitemquantity_stub(const game::ControllerIndex_t /*controller_index*/, const game::eModes mode,
		                              const int /*item_id*/)
		{
			if (mode == game::eModes::MODE_ZOMBIES)
			{
				return 999;
			}

			return 1;
		}

		int liveinventory_getitemquantity_stub(const game::ControllerIndex_t controller_index, const int item_id)
		{
			// Item id's for extra CaC slots, CWL camo's and paid specialist outfits
			if (item_id == 99003 || item_id >= 99018 && item_id <= 99021 || item_id == 99025 || item_id >= 90047 &&
				item_id <= 90064)
			{
				return 1;
			}

			return liveinventory_getitemquantity_hook.invoke<int>(controller_index, item_id);
		}

		bool liveinventory_areextraslotspurchased_stub(const game::ControllerIndex_t /*controller_index*/)
		{
			return true;
		}

		void set_dvars_on_startup()
		{
			game::Dvar_SetFromStringByName("ui_enableAllHeroes", "1", true);
			game::Dvar_SetFromStringByName("ui_allLootUnlocked", "1", true);
		}
	};

	struct component final : client_component_interface
	{
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
