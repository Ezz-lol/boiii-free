#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "steam_proxy.hpp"
#include "scheduler.hpp"

#include <utils/nt.hpp>
#include <utils/flags.hpp>
#include <utils/string.hpp>
#include <utils/finally.hpp>
#include <utils/concurrency.hpp>
#include <utils/binary_resource.hpp>

#include "resource.hpp"

#include "steam/interface.hpp"
#include "steam/steam.hpp"

namespace steam_proxy
{
	namespace
	{
		utils::binary_resource runner_file(RUNNER, "boiii-runner.exe");

		utils::nt::library steam_client_module{};
		utils::nt::library steam_overlay_module{};

		void* steam_pipe = nullptr;
		void* global_user = nullptr;

		steam::interface client_engine{};
		steam::interface client_user{};
		steam::interface client_utils{};
		steam::interface client_friends{};
		steam::interface client_ugc{};

		utils::concurrency::container<subscribed_item_map> subscribed_items;

		enum class ownership_state
		{
			success,
			unowned,
			nosteam,
			error,
		};

		bool is_disabled()
		{
			static const auto disabled = utils::flags::has_flag("nosteam");
			return disabled;
		}

		void* load_client_engine()
		{
			if (!steam_client_module) return nullptr;

			for (auto i = 1; i > 0; ++i)
			{
				std::string name = utils::string::va("CLIENTENGINE_INTERFACE_VERSION%03i", i);
				auto* const temp_client_engine = steam_client_module
					.invoke<void*>("CreateInterface", name.data(), nullptr);
				if (temp_client_engine) return temp_client_engine;
			}

			return nullptr;
		}

		void load_client()
		{
			const std::filesystem::path steam_path = steam::SteamAPI_GetSteamInstallPath();
			if (steam_path.empty()) return;

			utils::nt::library::load(steam_path / "tier0_s64.dll");
			utils::nt::library::load(steam_path / "vstdlib_s64.dll");
			steam_overlay_module = utils::nt::library::load(steam_path / "gameoverlayrenderer64.dll");
			steam_client_module = utils::nt::library::load(steam_path / "steamclient64.dll");
			if (!steam_client_module) return;

			client_engine = load_client_engine();
			if (!client_engine) return;

			steam_pipe = steam_client_module.invoke<void*>("Steam_CreateSteamPipe");
			global_user = steam_client_module.invoke<void*>(
				"Steam_ConnectToGlobalUser", steam_pipe);

			client_user = client_engine.invoke<void*>(8, global_user, steam_pipe);
			client_utils = client_engine.invoke<void*>(14, steam_pipe);
			client_friends = client_engine.invoke<void*>(13, global_user, steam_pipe);
			client_ugc = client_engine.invoke<void*>(61, global_user, steam_pipe);
		}

		void do_cleanup()
		{
			client_engine = nullptr;
			client_user = nullptr;
			client_utils = nullptr;
			client_friends = nullptr;
			client_ugc = nullptr;

			steam_pipe = nullptr;
			global_user = nullptr;

			steam_client_module = utils::nt::library{nullptr};
		}

		bool perform_cleanup_if_needed()
		{
			if (steam_client_module
				&& steam_pipe
				&& global_user
				&& steam_client_module.invoke<bool>("Steam_BConnected", global_user,
				                                    steam_pipe)
				&& steam_client_module.invoke<bool>("Steam_BLoggedOn", global_user, steam_pipe)
			)
			{
				return false;
			}

			do_cleanup();
			return true;
		}

		void clean_up_on_error()
		{
			scheduler::schedule([]
			{
				if (perform_cleanup_if_needed())
				{
					return scheduler::cond_end;
				}

				return scheduler::cond_continue;
			}, scheduler::main);
		}

		ownership_state start_mod_unsafe(const std::string& title, size_t app_id)
		{
			if (!client_utils || !client_user)
			{
				return ownership_state::nosteam;
			}

			if (!client_user.invoke<bool>("BIsSubscribedApp", app_id))
			{
				//app_id = 480; // Spacewar
				return ownership_state::unowned;
			}

			if (is_disabled())
			{
				return ownership_state::success;
			}

			client_utils.invoke<void>("SetAppIDForCurrentPipe", app_id, false);

			char our_directory[MAX_PATH] = {0};
			GetCurrentDirectoryA(sizeof(our_directory), our_directory);

			const auto path = runner_file.get_extracted_file();
			const std::string cmdline = utils::string::va("\"%s\" -proc %d", path.data(), GetCurrentProcessId());

			steam::game_id game_id;
			game_id.raw.type = 1; // k_EGameIDTypeGameMod
			game_id.raw.app_id = app_id & 0xFFFFFF;

			const auto* mod_id = "bo3";
			game_id.raw.mod_id = *reinterpret_cast<const unsigned int*>(mod_id) | 0x80000000;

			client_user.invoke<bool>("SpawnProcess", path.data(), cmdline.data(), our_directory,
			                         &game_id.bits, title.data(), 0, 0, 0);

			return ownership_state::success;
		}

		ownership_state start_mod(const std::string& title, const size_t app_id)
		{
			__try
			{
				return start_mod_unsafe(title, app_id);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				do_cleanup();
				return ownership_state::error;
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void pre_start() override
		{
			load_client();
			perform_cleanup_if_needed();
		}

		void post_unpack() override
		{
			try
			{
				const auto res = start_mod("\xE2\x98\x84\xEF\xB8\x8F" " BOIII"s, steam::SteamUtils()->GetAppID());

				switch (res)
				{
				case ownership_state::nosteam:
					throw std::runtime_error("Steam must be running to play this game!");
				case ownership_state::unowned:
					throw std::runtime_error("You must own the game on steam to play this mod!");
				case ownership_state::error:
					throw std::runtime_error("Failed to verify ownership of the game!");
				case ownership_state::success:
					break;
				}
			}
			catch (std::exception& e)
			{
				printf("Steam: %s\n", e.what());
				MessageBoxA(GetForegroundWindow(), e.what(), "BOIII Error", MB_ICONERROR);
				TerminateProcess(GetCurrentProcess(), 1234);
			}

			clean_up_on_error();
		}

		void pre_destroy() override
		{
			if (steam_client_module && steam_pipe)
			{
				if (global_user)
				{
					steam_client_module.invoke<void>("Steam_ReleaseUser", steam_pipe,
					                                 global_user);
				}

				steam_client_module.invoke<bool>("Steam_BReleaseSteamPipe", steam_pipe);
			}
		}

		int priority() override
		{
			return 998; // Run after the updater
		}
	};

	const utils::nt::library& get_overlay_module()
	{
		return steam_overlay_module;
	}

	const char* get_player_name()
	{
		if (client_friends)
		{
			return client_friends.invoke<const char*>("GetPersonaName");
		}

		return "boiii";
	}

	void update_subscribed_items()
	{
		subscribed_item_map map{};

		const auto _ = utils::finally([&]
		{
			subscribed_items.access([&](subscribed_item_map& items)
			{
				items = std::move(map);
			});
		});

		if (!client_ugc)
		{
			return;
		}

		try
		{
			const auto app_id = steam::SteamUtils()->GetAppID();
			const auto num_items = client_ugc.invoke<uint32_t>("GetNumSubscribedItems", app_id);

			if (!num_items)
			{
				return;
			}

			std::vector<uint64_t> ids;
			ids.resize(num_items);

			auto result = client_ugc.invoke<uint32_t>("GetSubscribedItems", app_id, ids.data(),
			                                          num_items);
			result = std::min(num_items, result);

			for (uint32_t i = 0; i < result; ++i)
			{
				char buffer[0x1000] = {0};
				subscribed_item item{};

				item.state = client_ugc.invoke<uint32_t>("GetItemState", app_id, ids[i]);
				item.available = client_ugc.invoke<bool>("GetItemInstallInfo", app_id, ids[i],
				                                         &item.size_on_disk,
				                                         buffer,
				                                         sizeof(buffer), &item.time_stamp);
				item.path = buffer;

				map[ids[i]] = std::move(item);
			}
		}
		catch (...)
		{
			client_ugc = {};
		}
	}

	void access_subscribed_items(
		const std::function<void(const subscribed_item_map&)>& callback)
	{
		subscribed_items.access(callback);
	}
}

REGISTER_COMPONENT(steam_proxy::component)
