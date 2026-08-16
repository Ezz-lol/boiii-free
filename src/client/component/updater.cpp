#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include "updater.hpp"
#include <game/game.hpp>

#include <utils/flags.hpp>
#include <utils/properties.hpp>
#include <utils/progress_ui.hpp>
#include <updater/updater.hpp>

namespace updater {
namespace {
bool automatic_updates_enabled() {
  const auto stored = utils::properties::load("launcherUiSettings");
  if (!stored)
    return true;
  rapidjson::Document document;
  if (document.Parse(stored->c_str()).HasParseError() || !document.IsObject())
    return true;
  const auto setting = document.FindMember("autoUpdate");
  return setting == document.MemberEnd() || !setting->value.IsBool() ||
         setting->value.GetBool();
}

bool show_updater_errors() {
  return !game::is_headless() && !utils::flags::has_flag("dedicated");
}

void report_updater_error(const char *message) {
  OutputDebugStringA(message);
  OutputDebugStringA("\n");
  if (show_updater_errors()) {
    utils::progress_ui::show_error("Updater Error", message);
  }
}
} // namespace

void update() {
  if (utils::flags::has_flag("noupdate") ||
      (!utils::flags::has_flag("update") && !automatic_updates_enabled())) {
    return;
  }

  try {
    run(game::get_appdata_path());
  } catch (update_cancelled &) {
    TerminateProcess(GetCurrentProcess(), 0);
  } catch (const std::exception &e) {
    report_updater_error(e.what());
  } catch (...) {
    report_updater_error("Unknown error occurred during update.");
  }
}

class component final : public generic_component {
public:
  component() {
    this->update_thread_ = std::thread([] { update(); });
  }

  void pre_destroy() override { join(); }

  void post_unpack() override { join(); }

  component_priority priority() const override {
    return component_priority::updater;
  }

private:
  std::thread update_thread_{};

  void join() {
    if (this->update_thread_.joinable()) {
      this->update_thread_.join();
    }
  }
};
} // namespace updater

REGISTER_COMPONENT(updater::component)
