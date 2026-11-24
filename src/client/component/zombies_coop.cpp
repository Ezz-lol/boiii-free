#include <std_include.hpp>

#include "zombies/coop_policy.hpp"
#include "zombies/coop_runtime.hpp"
#include <game/utils.hpp>
#include <loader/component_loader.hpp>
#include <utils/flags.hpp>

namespace zombies::coop {
void validate_startup() {
  (void)game::set_dvar_string("zm_coopStatus", "initializing", false);
  (void)game::set_dvar_int("zm_coopHeartbeat", 0, false);
  if (game::get_dvar_int("zm_coopPlayerLimit").value_or(stock_limit) >
          stock_limit &&
      game::com::Com_SessionMode_GetMode() == game::eModes::ZOMBIES &&
      !supported_map(game::get_dvar_string("mapname").value_or(""))) {
    game::com::Com_Error(
        game::errorParm::DROP,
        "Experimental Zombies coop currently supports zm_zod and zm_tomb only. "
        "Set zm_coopPlayerLimit to 4 for other maps.");
  }
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("zombies/coop");

  void post_unpack() override {
    (void)game::register_dvar_string(
        "zm_coopStatus", "disabled", 0u,
        "Experimental Zombies script startup status.");
    (void)game::register_dvar_int("zm_coopHeartbeat", 0, 0, INT_MAX, 0u,
                                  "Experimental Zombies script heartbeat.");
    (void)game::register_dvar_int(
        "zm_coopPlayerLimit", stock_limit, 1,
        utils::flags::has_flag("experimental-zombies-coop") ? player_limit
                                                            : stock_limit,
        game::dvarFlags_e::ARCHIVE,
        "Experimental Zombies lobby capacity (1-8); apply before hosting. "
        "Default 4.");
  }
};
} // namespace zombies::coop

REGISTER_COMPONENT(zombies::coop::component)
