#include <std_include.hpp>
#include <utils/nt.hpp>
#include <utils/properties.hpp>

#include "launcher.hpp"
#include "html/html_window.hpp"

#include <game/game.hpp>
#include <utils/string.hpp>
#include <version.hpp>

namespace launcher
{
	namespace
	{
		std::string normalize_option_token(std::string token)
		{
			utils::string::trim(token);

			if (token.empty())
			{
				return {};
			}

			while (!token.empty() && token.front() == '-')
			{
				token.erase(token.begin());
			}

			token = utils::string::to_lower(std::move(token));

			if (token == "safe-mode")
			{
				token = "safe";
			}
			return token;
		}

		void relaunch_with_launch_options(const std::vector<std::string>& options)
		{
			const auto self = utils::nt::library::get_by_address(relaunch_with_launch_options);
			const auto exe_path = self.get_path().generic_string();

			STARTUPINFOA startup_info;
			PROCESS_INFORMATION process_info;
			ZeroMemory(&startup_info, sizeof(startup_info));
			ZeroMemory(&process_info, sizeof(process_info));
			startup_info.cb = sizeof(startup_info);

			char current_dir[MAX_PATH];
			GetCurrentDirectoryA(sizeof(current_dir), current_dir);

			std::string command_line = "\"" + exe_path + "\"";

			int num_args = 0;
			auto* const argv = CommandLineToArgvW(GetCommandLineW(), &num_args);
			if (argv)
			{
				for (auto i = 1; i < num_args; ++i)
				{
					std::wstring wide_arg(argv[i]);
					std::string arg = utils::string::convert(wide_arg);

					const auto lower = utils::string::to_lower(arg);
					if (lower == "norelaunch" || lower == "update")
					{
						continue;
					}

					command_line += " \"" + arg + "\"";
				}

				LocalFree(argv);
			}

			command_line += " \"-launch\"";
			for (const auto& raw : options)
			{
				auto token = normalize_option_token(raw);
				if (token.empty())
				{
					continue;
				}

				command_line += " \"-" + token + "\"";
			}

			if (CreateProcessA(exe_path.data(), command_line.data(), nullptr, nullptr, false,
				CREATE_NEW_CONSOLE, nullptr, current_dir, &startup_info, &process_info))
			{
				if (process_info.hThread && process_info.hThread != INVALID_HANDLE_VALUE)
				{
					CloseHandle(process_info.hThread);
				}

				if (process_info.hProcess && process_info.hProcess != INVALID_HANDLE_VALUE)
				{
					CloseHandle(process_info.hProcess);
				}
			}
		}
	}

	bool run()
	{
		bool run_game = false;
		std::vector<std::string> launch_options{};
		html_window window("BOIII", 590, 360);

		window.get_html_frame()->register_callback(
			"getVersion", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				return CComVariant(SHORTVERSION);
			});

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
			"readLaunchOptions", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				const auto stored = utils::properties::load("launchOptions");
				if (!stored)
				{
					return CComVariant("");
				}
				return CComVariant(stored->c_str());
			});

		window.get_html_frame()->register_callback(
			"readPlayerName", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				const auto stored_name = utils::properties::load("playerName");
				if (!stored_name)
				{
					auto fallback = utils::nt::get_user_name();
					if (fallback.empty())
					{
						fallback = "Unknown Soldier";
					}
					return CComVariant(fallback.c_str());
				}
				return CComVariant(stored_name->c_str());
			});

		window.get_html_frame()->register_callback(
			"runGame", [&](const std::vector<html_argument>& params) -> CComVariant
			{
				std::string new_name{};
				if (!params.empty() && params[0].is_string())
				{
					new_name = params[0].get_string();
					utils::string::trim(new_name);
				}

				if (new_name.empty())
				{
					new_name = utils::nt::get_user_name();
					if (new_name.empty())
					{
						new_name = "Unknown Soldier";
					}
				}

				if (new_name.size() > 16)
				{
					new_name.resize(16);
				}

				utils::properties::store("playerName", new_name);

				std::string option_list{};
				if (params.size() >= 2 && params[1].is_string())
				{
					option_list = params[1].get_string();
					utils::string::trim(option_list);
				}

				utils::properties::store("launchOptions", option_list);

				launch_options.clear();
				if (!option_list.empty())
				{
					for (auto& part : utils::string::split(option_list, ' '))
					{
						auto token = normalize_option_token(std::move(part));
						if (!token.empty())
						{
							launch_options.emplace_back(std::move(token));
						}
					}
				}

				if (!launch_options.empty())
				{
					run_game = false;
				}
				else
				{
					run_game = true;
				}

				window.get_window()->close();
				return {};
			});

		//window.get_html_frame()->load_html(utils::nt::load_resource(MENU_MAIN));
		window.get_html_frame()->load_url(
			utils::string::va("file:///%ls", get_launcher_ui_file().wstring().c_str()));

		window::run();
		if (!launch_options.empty())
		{
			relaunch_with_launch_options(launch_options);
			return false;
		}
		return run_game;
	}

	std::filesystem::path get_launcher_ui_file()
	{
		return game::get_appdata_path() / "data/launcher/main.html";
	}
}
