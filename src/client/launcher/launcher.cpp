#include <std_include.hpp>
#include <utils/nt.hpp>

#include "launcher.hpp"
#include "html/html_window.hpp"

#include "resource.hpp"
#include "component/updater.hpp"

namespace launcher
{
	bool run()
	{
		updater::update();

		bool run_game = false;
		html_window window("BOIII", 550, 380);

		window.get_html_frame()->register_callback(
			"openUrl", [](const std::vector<html_argument>& params) -> CComVariant
			{
				if (params.empty()) return {};

				const auto& param = params[0];
				if (!param.is_string()) return {};

				const auto url = param.get_string();
				ShellExecuteA(nullptr, "open", url.data(), nullptr, nullptr, SW_SHOWNORMAL);

				return {};
			});

		window.get_html_frame()->register_callback(
			"runGame", [&](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				run_game = true;
				window.get_window()->close();
				return {};
			});

		window.get_html_frame()->load_html(utils::nt::load_resource(MENU_MAIN));

		window::run();
		return run_game;
	}
}
