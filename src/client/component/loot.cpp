#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "scheduler.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"

namespace loot
{
	namespace
	{
		const game::dvar_t* dvar_cg_unlockall_loot;
		const game::dvar_t* dvar_cg_unlockall_purchases;
		const game::dvar_t* dvar_cg_unlockall_attachments;
		const game::dvar_t* dvar_cg_unlockall_camos_and_reticles;
		const game::dvar_t* dvar_cg_unlockall_calling_cards;
		const game::dvar_t* dvar_cg_unlockall_specialists_outfits;
		const game::dvar_t* dvar_cg_unlockall_cac_slots;

		utils::hook::detour loot_getitemquantity_hook;
		utils::hook::detour liveinventory_getitemquantity_hook;
		utils::hook::detour liveinventory_areextraslotspurchased_hook;
		utils::hook::detour bg_unlockablesisitempurchased_hook;
		utils::hook::detour bg_unlockablesisitemattachmentlocked_hook;
		utils::hook::detour bg_unlockablesisattachmentslotlocked_hook;
		utils::hook::detour bg_unlockablesemblemorbackinglockedbychallenge_hook;
		utils::hook::detour bg_unlockablesitemoptionlocked_hook;
		utils::hook::detour bg_unlockedgetchallengeunlockedforindex_hook;
		utils::hook::detour bg_unlockablescharactercustomizationitemlocked_hook;
		utils::hook::detour bg_emblemisentitlementbackgroundgranted_hook;
		utils::hook::detour liveentitlements_isentitlementactiveforcontroller_hook;
		utils::hook::detour bg_unlockablesgetcustomclasscount_hook;

		int loot_getitemquantity_stub(const game::ControllerIndex_t controller_index, const game::eModes mode, const int item_id)
		{
			if (!dvar_cg_unlockall_loot->current.value.enabled)
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
			// Item id's for CWL camo's and paid specialist outfits
			if (dvar_cg_unlockall_loot->current.value.enabled && (item_id == 99003 || item_id >= 99018 && item_id <= 99021 || item_id == 99025 ||
				item_id >= 90047 && item_id <= 90064))
			{
				return 1;
			}

			// Item id for extra CaC slots
			if (dvar_cg_unlockall_cac_slots->current.value.enabled && item_id == 99003)
			{
				return 1;
			}

			return liveinventory_getitemquantity_hook.invoke<int>(controller_index, item_id);
		}

		bool liveinventory_areextraslotspurchased_stub(const game::ControllerIndex_t controller_index)
		{
			if (dvar_cg_unlockall_cac_slots->current.value.enabled)
			{
				return true;
			}

			return liveinventory_areextraslotspurchased_hook.invoke<bool>(controller_index);
		}

		bool bg_unlockablesisitempurchased_stub(game::eModes mode, const game::ControllerIndex_t controller_index, int item_index)
		{
			if (dvar_cg_unlockall_purchases->current.value.enabled)
			{
				return true;
			}

			return bg_unlockablesisitempurchased_hook.invoke<bool>(mode, controller_index, item_index);
		}

		bool bg_unlockablesisitemattachmentlocked_stub(game::eModes mode, const game::ControllerIndex_t controller_index, int item_index, int attachment_num)
		{
			if (dvar_cg_unlockall_attachments->current.value.enabled)
			{
				return false;
			}

			return bg_unlockablesisitemattachmentlocked_hook.invoke<bool>(mode, controller_index, item_index, attachment_num);
		}

		bool bg_unlockablesisattachmentslotlocked_stub(game::eModes mode, const game::ControllerIndex_t controller_index, int item_index, int attachment_slot_index)
		{
			if (dvar_cg_unlockall_attachments->current.value.enabled)
			{
				return false;
			}

			return bg_unlockablesisattachmentslotlocked_hook.invoke<bool>(mode, controller_index, item_index, attachment_slot_index);
		}

		bool bg_unlockablesitemoptionlocked_stub(game::eModes mode, const game::ControllerIndex_t controllerIndex, int itemIndex, int optionIndex)
		{
			if (dvar_cg_unlockall_camos_and_reticles->current.value.enabled)
			{
				return false;
			}

			return bg_unlockablesitemoptionlocked_hook.invoke<bool>(mode, controllerIndex, itemIndex, optionIndex);
		}

		bool bg_unlockablesemblemorbackinglockedbychallenge_stub(game::eModes mode, const game::ControllerIndex_t controllerIndex, game::emblemChallengeLookup_t* challengeLookup, bool otherPlayer)
		{
			if (dvar_cg_unlockall_calling_cards->current.value.enabled)
			{
				return false;
			}

			return bg_unlockablesemblemorbackinglockedbychallenge_hook.invoke<bool>(mode, controllerIndex, challengeLookup, otherPlayer);
		}

		bool bg_unlockedgetchallengeunlockedforindex_stub(game::eModes mode, const game::ControllerIndex_t controllerIndex, unsigned __int16 index, int itemIndex)
		{
			if (dvar_cg_unlockall_camos_and_reticles->current.value.enabled)
			{
				return true;
			}

			return bg_unlockedgetchallengeunlockedforindex_hook.invoke<bool>(mode, controllerIndex, index, itemIndex);
		}

		bool bg_unlockablescharactercustomizationitemlocked_stub(game::eModes mode, const game::ControllerIndex_t controllerIndex, uint32_t characterIndex, game::CharacterItemType itemType, int itemIndex)
		{
			if (dvar_cg_unlockall_specialists_outfits->current.value.enabled)
			{
				return false;
			}

			return bg_unlockablescharactercustomizationitemlocked_hook.invoke<bool>(mode, controllerIndex, characterIndex, itemType, itemIndex);
		}

		bool bg_emblemisentitlementbackgroundgranted_stub(const game::ControllerIndex_t controllerIndex, game::BGEmblemBackgroundID backgroundId)
		{
			// backgroundId's for blank CWL calling cards
			if (dvar_cg_unlockall_calling_cards->current.value.enabled && (backgroundId != 684 && backgroundId != 685 && backgroundId != 687 && backgroundId != 693 &&
				backgroundId != 695 && backgroundId != 701 && backgroundId != 703 && backgroundId != 707 && backgroundId != 708))
			{
				return true;
			}

			return bg_emblemisentitlementbackgroundgranted_hook.invoke<bool>(controllerIndex, backgroundId);
		}

		bool liveentitlements_isentitlementactiveforcontroller_stub(const game::ControllerIndex_t controllerIndex, int incentiveId)
		{
			// incentiveId for unavailable incentive
			if (dvar_cg_unlockall_calling_cards->current.value.enabled && incentiveId != 29)
			{
				return true;
			}

			return liveentitlements_isentitlementactiveforcontroller_hook.invoke<bool>(controllerIndex, incentiveId);
		}

		int bg_unlockablesgetcustomclasscount_stub(game::eModes mode, const game::ControllerIndex_t controllerIndex)
		{
			if (dvar_cg_unlockall_cac_slots->current.value.enabled)
			{
				return 10;
			}

			return bg_unlockablesgetcustomclasscount_hook.invoke<int>(mode, controllerIndex);
		}
	};

	struct component final: client_component
	{
		void post_unpack() override
		{
			dvar_cg_unlockall_loot = game::register_dvar_bool("cg_unlockall_loot", false, game::DVAR_ARCHIVE, "Unlocks blackmarket loot");
			dvar_cg_unlockall_purchases = game::register_dvar_bool("cg_unlockall_purchases", false, game::DVAR_ARCHIVE, "Unlock all purchases with tokens");
			dvar_cg_unlockall_attachments = game::register_dvar_bool("cg_unlockall_attachments", false, game::DVAR_ARCHIVE, "Unlocks all attachments");
			dvar_cg_unlockall_camos_and_reticles = game::register_dvar_bool("cg_unlockall_camos_and_reticles", false, game::DVAR_ARCHIVE, "Unlocks all camos and reticles");
			dvar_cg_unlockall_calling_cards = game::register_dvar_bool("cg_unlockall_calling_cards", false, game::DVAR_ARCHIVE, "Unlocks all calling cards");
			dvar_cg_unlockall_specialists_outfits = game::register_dvar_bool("cg_unlockall_specialists_outfits", false, game::DVAR_ARCHIVE, "Unlocks all specialists outfits");
			dvar_cg_unlockall_cac_slots = game::register_dvar_bool("cg_unlockall_cac_slots", false, game::DVAR_ARCHIVE, "Unlocks all Create a Class Slots");

			loot_getitemquantity_hook.create(0x141E82C00_g, loot_getitemquantity_stub);
			liveinventory_getitemquantity_hook.create(0x141E09030_g, liveinventory_getitemquantity_stub);
			liveinventory_areextraslotspurchased_hook.create(0x141E08950_g, liveinventory_areextraslotspurchased_stub);
			bg_unlockablesisitempurchased_hook.create(0x1426A9620_g, bg_unlockablesisitempurchased_stub);
			bg_unlockablesisitemattachmentlocked_hook.create(0x1426A88D0_g, bg_unlockablesisitemattachmentlocked_stub);
			bg_unlockablesisattachmentslotlocked_hook.create(0x1426A86D0_g, bg_unlockablesisattachmentslotlocked_stub);
			bg_unlockablesitemoptionlocked_hook.create(0x1426AA6C0_g, bg_unlockablesitemoptionlocked_stub);
			bg_unlockablesemblemorbackinglockedbychallenge_hook.create(0x1426A3AE0_g, bg_unlockablesemblemorbackinglockedbychallenge_stub);
			bg_unlockedgetchallengeunlockedforindex_hook.create(0x1426AF5F0_g, bg_unlockedgetchallengeunlockedforindex_stub);
			bg_unlockablescharactercustomizationitemlocked_hook.create(0x1426A2030_g, bg_unlockablescharactercustomizationitemlocked_stub);
			bg_emblemisentitlementbackgroundgranted_hook.create(0x142667520_g, bg_emblemisentitlementbackgroundgranted_stub);
			liveentitlements_isentitlementactiveforcontroller_hook.create(0x141E124E0_g, liveentitlements_isentitlementactiveforcontroller_stub);
			bg_unlockablesgetcustomclasscount_hook.create(0x1426A5900_g, bg_unlockablesgetcustomclasscount_stub);

			scheduler::once([]() {
				if (dvar_cg_unlockall_loot->current.value.enabled)
				{
					game::Dvar_SetFromStringByName("ui_enableAllHeroes", "1", true);
				}
			}, scheduler::pipeline::dvars_loaded);
		}
	};
};

REGISTER_COMPONENT(loot::component)
