#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "scheduler.hpp"
#include "game/game.hpp"

namespace loot
{
	namespace
	{
		game::dvar_t* dvar_cg_unlockall_loot;

		utils::hook::detour loot_getitemquantity_hook;
		utils::hook::detour liveinventory_getitemquantity_hook;
		utils::hook::detour liveinventory_areextraslotspurchased_hook;

		int loot_getitemquantity_stub(const game::ControllerIndex_t controller_index, const game::eModes mode,
		                              const int item_id)
		{
			if (!dvar_cg_unlockall_loot->current.enabled)
			{
				return loot_getitemquantity_hook.invoke<int>(controller_index, mode, item_id);
			}

			if (mode == game::eModes::MODE_ZOMBIES)
			{
				return 999;
			}

			return 1;
		}

		int liveinventory_getitemquantity_stub(const game::ControllerIndex_t controller_index, const int item_id)
		{
			// Item id's for extra CaC slots, CWL camo's and paid specialist outfits
			if (dvar_cg_unlockall_loot->current.enabled && (item_id == 99003 || item_id >= 99018 && item_id <= 99021 || item_id == 99025||
				item_id >= 90047 && item_id <= 90064))
			{
				return 1;
			}

			return liveinventory_getitemquantity_hook.invoke<int>(controller_index, item_id);
		}

		bool liveinventory_areextraslotspurchased_stub(const game::ControllerIndex_t controller_index)
		{
			if (dvar_cg_unlockall_loot->current.enabled)
			{
				return true;
			}

			return liveinventory_areextraslotspurchased_hook.invoke<bool>(controller_index);
		}
	};

	struct component final : client_component
	{
		void post_unpack() override
		{
			dvar_cg_unlockall_loot = game::Dvar_RegisterBool(game::Dvar_GenerateHash("cg_unlockall_loot"), "cg_unlockall_loot", false, (game::dvarFlags_e)0x0, "Unlocks blackmarket loot");
			dvar_cg_unlockall_loot->debugName = "cg_unlockall_loot";

			loot_getitemquantity_hook.create(0x141E82C00_g, loot_getitemquantity_stub);
			liveinventory_getitemquantity_hook.create(0x141E09030_g, liveinventory_getitemquantity_stub);
			liveinventory_areextraslotspurchased_hook.create(0x141E08950_g, liveinventory_areextraslotspurchased_stub);

			scheduler::once([]() {
				if (dvar_cg_unlockall_loot->current.enabled)
				{
					game::Dvar_SetFromStringByName("ui_enableAllHeroes", "1", true);
				}
			}, scheduler::pipeline::dvars_loaded);
		}
	};
};

REGISTER_COMPONENT_WORKING(loot::component)
