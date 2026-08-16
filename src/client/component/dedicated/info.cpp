#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/game.hpp>
#include <game/utils.hpp>

#if __has_include("version.hpp")
#include "version.hpp"
#else
#ifndef SHORTVERSION
#define SHORTVERSION "0"
#endif
#endif

#include <component/scheduler.hpp>
#include <component/getinfo.hpp>
#include <component/console.hpp>

#include <string>
#include <utils/string.hpp>

namespace dedicated {
namespace info {
namespace {
void set_server_info_in_console_title() {
  const bool sv_running = game::get_sv_running();
  std::string server_name =
      std::string(game::get_live_steam_server_name().value_or(""));
  const char *clean_server_name = game::I_CleanStr(server_name.data());

  if (!sv_running) {
    console::set_title(utils::string::va("BOIII V%s - %s - not running",
                                         SHORTVERSION, clean_server_name));
    return;
  }

  const std::string_view mapname = game::get_mapname().value_or("");

  const std::string window_text = utils::string::va(
      "BOIII V%s - %s on %s [%zu/%zu] (%zu)", SHORTVERSION, clean_server_name,
      mapname.data(), getinfo::get_client_count(), game::get_max_client_count(),
      getinfo::get_bot_count());

  console::set_title(window_text);
}
} // namespace

class component final : public server_component {
public:
  void post_unpack() override {
    scheduler::loop(set_server_info_in_console_title, scheduler::pipeline::main,
                    1s);
  }
};
} // namespace info
} // namespace dedicated
REGISTER_COMPONENT(dedicated::info::component)
