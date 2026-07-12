#include "../std_include.hpp"
#include "../loader/component_loader.hpp"

#include "../../common/utils/hook.hpp"

#include "command.hpp"
#include "scheduler.hpp"
#include "toast.hpp"
#include "../game/game.hpp"
#include "../game/utils.hpp"

namespace loot {
namespace {
game::EngineDependentDvar dvar_cg_unlockall_loot;
game::EngineDependentDvar dvar_cg_unlockall_purchases;
game::EngineDependentDvar dvar_cg_unlockall_attachments;
game::EngineDependentDvar dvar_cg_unlockall_camos_and_reticles;
game::EngineDependentDvar dvar_cg_unlockall_calling_cards;
game::EngineDependentDvar dvar_cg_unlockall_specialists_outfits;
game::EngineDependentDvar dvar_cg_unlockall_cac_slots;

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
utils::hook::detour gscr_isitempurchasedforclientnum_hook;

int loot_getitemquantity_stub(const game::ControllerIndex_t controller_index,
                              const game::eModes mode, const int item_id) {
  if (!game::get_dvar_bool(dvar_cg_unlockall_loot)) {
    return loot_getitemquantity_hook.invoke<int>(controller_index, mode,
                                                 item_id);
  }

  if (mode == game::eModes::ZOMBIES) {
    return 999;
  }

  return 1;
}

int liveinventory_getitemquantity_stub(
    const game::ControllerIndex_t controller_index, const int item_id) {
  // Item id's for CWL camo's and paid specialist outfits
  if (game::get_dvar_bool(dvar_cg_unlockall_loot) &&
      (item_id == 99003 || (item_id >= 99018 && item_id <= 99021) ||
       item_id == 99025 || (item_id >= 90047 && item_id <= 90064))) {
    return 1;
  }

  // Item id for extra CaC slots
  if (game::get_dvar_bool(dvar_cg_unlockall_cac_slots) && item_id == 99003) {
    return 1;
  }

  return liveinventory_getitemquantity_hook.invoke<int>(controller_index,
                                                        item_id);
}

bool liveinventory_areextraslotspurchased_stub(
    const game::ControllerIndex_t controller_index) {
  if (game::get_dvar_bool(dvar_cg_unlockall_cac_slots)) {
    return true;
  }

  return liveinventory_areextraslotspurchased_hook.invoke<bool>(
      controller_index);
}

bool bg_unlockablesisitempurchased_stub(
    game::eModes mode, const game::ControllerIndex_t controller_index,
    int item_index) {
  if (game::get_dvar_bool(dvar_cg_unlockall_purchases)) {
    return true;
  }

  return bg_unlockablesisitempurchased_hook.invoke<bool>(mode, controller_index,
                                                         item_index);
}

bool bg_unlockablesisitemattachmentlocked_stub(
    game::eModes mode, const game::ControllerIndex_t controller_index,
    int item_index, int attachment_num) {
  if (game::get_dvar_bool(dvar_cg_unlockall_attachments)) {
    return false;
  }

  return bg_unlockablesisitemattachmentlocked_hook.invoke<bool>(
      mode, controller_index, item_index, attachment_num);
}

bool bg_unlockablesisattachmentslotlocked_stub(
    game::eModes mode, const game::ControllerIndex_t controller_index,
    int item_index, int attachment_slot_index) {
  if (game::get_dvar_bool(dvar_cg_unlockall_attachments)) {
    return false;
  }

  return bg_unlockablesisattachmentslotlocked_hook.invoke<bool>(
      mode, controller_index, item_index, attachment_slot_index);
}

bool bg_unlockablesitemoptionlocked_stub(
    game::eModes mode, const game::ControllerIndex_t controllerIndex,
    int itemIndex, int optionIndex) {
  if (game::get_dvar_bool(dvar_cg_unlockall_camos_and_reticles)) {
    return false;
  }

  return bg_unlockablesitemoptionlocked_hook.invoke<bool>(
      mode, controllerIndex, itemIndex, optionIndex);
}

bool bg_unlockablesemblemorbackinglockedbychallenge_stub(
    game::eModes mode, const game::ControllerIndex_t controllerIndex,
    game::emblemChallengeLookup_t *challengeLookup, bool otherPlayer) {
  if (game::get_dvar_bool(dvar_cg_unlockall_calling_cards)) {
    return false;
  }

  return bg_unlockablesemblemorbackinglockedbychallenge_hook.invoke<bool>(
      mode, controllerIndex, challengeLookup, otherPlayer);
}

bool bg_unlockedgetchallengeunlockedforindex_stub(
    game::eModes mode, const game::ControllerIndex_t controllerIndex,
    unsigned __int16 index, int itemIndex) {
  if (game::get_dvar_bool(dvar_cg_unlockall_camos_and_reticles)) {
    return true;
  }

  return bg_unlockedgetchallengeunlockedforindex_hook.invoke<bool>(
      mode, controllerIndex, index, itemIndex);
}

bool bg_unlockablescharactercustomizationitemlocked_stub(
    game::eModes mode, const game::ControllerIndex_t controllerIndex,
    uint32_t characterIndex, game::CharacterItemType itemType, int itemIndex) {
  if (game::get_dvar_bool(dvar_cg_unlockall_specialists_outfits)) {
    return false;
  }

  return bg_unlockablescharactercustomizationitemlocked_hook.invoke<bool>(
      mode, controllerIndex, characterIndex, itemType, itemIndex);
}

bool bg_emblemisentitlementbackgroundgranted_stub(
    const game::ControllerIndex_t controllerIndex,
    game::BGEmblemBackgroundID backgroundId) {
  // backgroundId's for blank CWL calling cards
  if (game::get_dvar_bool(dvar_cg_unlockall_calling_cards) &&
      (backgroundId != 684 && backgroundId != 685 && backgroundId != 687 &&
       backgroundId != 693 && backgroundId != 695 && backgroundId != 701 &&
       backgroundId != 703 && backgroundId != 707 && backgroundId != 708)) {
    return true;
  }

  return bg_emblemisentitlementbackgroundgranted_hook.invoke<bool>(
      controllerIndex, backgroundId);
}

bool liveentitlements_isentitlementactiveforcontroller_stub(
    const game::ControllerIndex_t controllerIndex, int incentiveId) {
  // incentiveId for unavailable incentive
  if (game::get_dvar_bool(dvar_cg_unlockall_calling_cards) &&
      incentiveId != 29) {
    return true;
  }

  return liveentitlements_isentitlementactiveforcontroller_hook.invoke<bool>(
      controllerIndex, incentiveId);
}

int bg_unlockablesgetcustomclasscount_stub(
    game::eModes mode, const game::ControllerIndex_t controllerIndex) {
  if (game::get_dvar_bool(dvar_cg_unlockall_cac_slots)) {
    return 10;
  }

  return bg_unlockablesgetcustomclasscount_hook.invoke<int>(mode,
                                                            controllerIndex);
}

bool gscr_isitempurchasedforclientnum_stub(
    [[maybe_unused]] unsigned int clientNum, [[maybe_unused]] int itemIndex) {
  return true;
}
}; // namespace

struct component final : generic_component {
  void post_unpack() override {
    gscr_isitempurchasedforclientnum_hook.create(
        game::select(0x1415F1490, 0x140252A20),
        gscr_isitempurchasedforclientnum_stub);

    if (game::is_server()) {
      return;
    }

    dvar_cg_unlockall_loot =
        game::register_dvar_bool("cg_unlockall_loot", false, game::DVAR_ARCHIVE,
                                 "Unlocks blackmarket loot");
    dvar_cg_unlockall_purchases = game::register_dvar_bool(
        "cg_unlockall_purchases", false, game::DVAR_ARCHIVE,
        "Unlock all purchases with tokens");
    dvar_cg_unlockall_attachments =
        game::register_dvar_bool("cg_unlockall_attachments", false,
                                 game::DVAR_ARCHIVE, "Unlocks all attachments");
    dvar_cg_unlockall_camos_and_reticles = game::register_dvar_bool(
        "cg_unlockall_camos_and_reticles", false, game::DVAR_ARCHIVE,
        "Unlocks all camos and reticles");
    dvar_cg_unlockall_calling_cards = game::register_dvar_bool(
        "cg_unlockall_calling_cards", false, game::DVAR_ARCHIVE,
        "Unlocks all calling cards");
    dvar_cg_unlockall_specialists_outfits = game::register_dvar_bool(
        "cg_unlockall_specialists_outfits", false, game::DVAR_ARCHIVE,
        "Unlocks all specialists outfits");
    dvar_cg_unlockall_cac_slots = game::register_dvar_bool(
        "cg_unlockall_cac_slots", false, game::DVAR_ARCHIVE,
        "Unlocks all Create a Class Slots");

    command::add("unlockall", [](const command::params &) {
      if (game::com::Com_IsInGame()) {
        toast::error(
            "Unlock All",
            "Cannot use unlockall while in-game. Return to main menu first.");
        return;
      }
      // Enable all unlock dvars (mode-independent)
      game::set_dvar_bool(dvar_cg_unlockall_loot, true);
      game::set_dvar_bool(dvar_cg_unlockall_purchases, true);
      game::set_dvar_bool(dvar_cg_unlockall_attachments, true);
      game::set_dvar_bool(dvar_cg_unlockall_camos_and_reticles, true);
      game::set_dvar_bool(dvar_cg_unlockall_calling_cards, true);
      game::set_dvar_bool(dvar_cg_unlockall_specialists_outfits, true);
      game::set_dvar_bool(dvar_cg_unlockall_cac_slots, true);
      game::set_dvar_bool(game::ui_enableAllHeroes(), true);

      // Set master prestige for all 3 modes (eModes: ZM=0, MP=1, CP=2)
      game::cbuf::Cbuf_AddText(0, "PrestigeStatsMaster 0\n"); // ZM
      game::cbuf::Cbuf_AddText(0, "PrestigeStatsMaster 1\n"); // MP
      game::cbuf::Cbuf_AddText(0, "PrestigeStatsMaster 2\n"); // CP

      // statsetbyname only affects the current session mode
      game::cbuf::Cbuf_AddText(0, "statsetbyname plevel 11\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname hasprestiged 1\n");

      const game::eModes mode = game::com::Com_SessionMode_GetMode();
      const char *mode_name = "";

      if (mode == game::eModes::MULTIPLAYER) {
        game::cbuf::Cbuf_AddText(0, "statsetbyname rank 54\n");
        game::cbuf::Cbuf_AddText(0, "statsetbyname paragon_rank 944\n");
        game::cbuf::Cbuf_AddText(0, "statsetbyname paragon_rankxp 56800000\n");
        mode_name = " Multiplayer";
      } else if (mode == game::eModes::ZOMBIES) {
        game::cbuf::Cbuf_AddText(0, "statsetbyname rank 34\n");
        game::cbuf::Cbuf_AddText(0, "statsetbyname paragon_rank 999\n");
        game::cbuf::Cbuf_AddText(0, "statsetbyname paragon_rankxp 56800000\n");
        mode_name = " Zombies";
      } else if (mode == game::eModes::CAMPAIGN) {
        game::cbuf::Cbuf_AddText(0, "statsetbyname rank 19\n");
        game::cbuf::Cbuf_AddText(0, "statsetbyname paragon_rank 999\n");
        game::cbuf::Cbuf_AddText(0, "statsetbyname paragon_rankxp 0\n");
        mode_name = " Campaign";
      }

      // Unlock all easter eggs (zombie darkops)
      game::cbuf::Cbuf_AddText(0, "statsetbyname darkops_zod_ee 1\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname darkops_zod_super_ee 1\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname darkops_factory_ee 1\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname darkops_factory_super_ee 1\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname darkops_castle_ee 1\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname darkops_castle_super_ee 1\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname darkops_island_ee 1\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname darkops_island_super_ee 1\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname darkops_stalingrad_ee 1\n");
      game::cbuf::Cbuf_AddText(0,
                               "statsetbyname darkops_stalingrad_super_ee 1\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname darkops_genesis_ee 1\n");
      game::cbuf::Cbuf_AddText(0, "statsetbyname DARKOPS_GENESIS_SUPER_EE 1\n");

      // Upload stats for all modes (eModes: ZM=0, MP=1, CP=2)
      game::cbuf::Cbuf_AddText(0, "uploadstats 0\n"); // ZM
      game::cbuf::Cbuf_AddText(0, "uploadstats 1\n"); // MP
      game::cbuf::Cbuf_AddText(0, "uploadstats 2\n"); // CP

      printf("[Loot] Unlock All (%s): all items, master prestige (all modes), "
             "max rank (%s), easter eggs\n",
             mode_name, mode_name);
      toast::success("Unlock All",
                     std::string("Unlocked all") + mode_name + " stats!");
    });

    loot_getitemquantity_hook.create(0x141E82C00_g, loot_getitemquantity_stub);
    liveinventory_getitemquantity_hook.create(
        0x141E09030_g, liveinventory_getitemquantity_stub);
    liveinventory_areextraslotspurchased_hook.create(
        0x141E08950_g, liveinventory_areextraslotspurchased_stub);
    bg_unlockablesisitempurchased_hook.create(
        0x1426A9620_g, bg_unlockablesisitempurchased_stub);
    bg_unlockablesisitemattachmentlocked_hook.create(
        0x1426A88D0_g, bg_unlockablesisitemattachmentlocked_stub);
    bg_unlockablesisattachmentslotlocked_hook.create(
        0x1426A86D0_g, bg_unlockablesisattachmentslotlocked_stub);
    bg_unlockablesitemoptionlocked_hook.create(
        0x1426AA6C0_g, bg_unlockablesitemoptionlocked_stub);
    bg_unlockablesemblemorbackinglockedbychallenge_hook.create(
        0x1426A3AE0_g, bg_unlockablesemblemorbackinglockedbychallenge_stub);
    bg_unlockedgetchallengeunlockedforindex_hook.create(
        0x1426AF5F0_g, bg_unlockedgetchallengeunlockedforindex_stub);
    bg_unlockablescharactercustomizationitemlocked_hook.create(
        0x1426A2030_g, bg_unlockablescharactercustomizationitemlocked_stub);
    bg_emblemisentitlementbackgroundgranted_hook.create(
        0x142667520_g, bg_emblemisentitlementbackgroundgranted_stub);
    liveentitlements_isentitlementactiveforcontroller_hook.create(
        0x141E124E0_g, liveentitlements_isentitlementactiveforcontroller_stub);
    bg_unlockablesgetcustomclasscount_hook.create(
        0x1426A5900_g, bg_unlockablesgetcustomclasscount_stub);

    scheduler::once(
        []() {
          if (game::get_dvar_bool(dvar_cg_unlockall_loot)) {
            // TODO: why does setting the dvar directly here cause
            // the game to freeze?
            // Why do we have to use Dvar_SetFromStringByName?
            // game::set_dvar_bool(game::ui_enableAllHeroes(), true);
            game::Dvar_SetFromStringByName("ui_enableAllHeroes", "1", true);
          }
        },
        scheduler::pipeline::dvars_loaded);
  }
};
}; // namespace loot

REGISTER_COMPONENT(loot::component)