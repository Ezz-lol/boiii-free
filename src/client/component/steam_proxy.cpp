#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "steam_proxy.hpp"
#include "scheduler.hpp"

#include <utils/nt.hpp>
#include <utils/flags.hpp>
#include <utils/string.hpp>
#include <utils/binary_resource.hpp>

#include "resource.hpp"

#include "steam/interface.hpp"
#include "steam/steam.hpp"

namespace steam_proxy
{
	namespace
	{
		utils::binary_resource runner_file(RUNNER, "boiii-runner.exe");

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
	}

	class component final : public component_interface
	{
	public:
		void pre_start() override
		{
			/*if (is_disabled())
			{
				return;
			}*/

			this->load_client();
			this->clean_up_on_error();

#ifndef DEV_BUILD
			try
			{
				const auto res = this->start_mod("\xE2\x98\x84\xEF\xB8\x8F" " BOIII"s, steam::SteamUtils()->GetAppID());

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
				MessageBoxA(GetForegroundWindow(), e.what(), "Error", MB_ICONERROR);
				TerminateProcess(GetCurrentProcess(), 1234);
			}
#endif
		}

		void pre_destroy() override
		{
			if (this->steam_client_module_)
			{
				if (this->steam_pipe_)
				{
					if (this->global_user_)
					{
						this->steam_client_module_.invoke<void>("Steam_ReleaseUser", this->steam_pipe_,
						                                        this->global_user_);
					}

					this->steam_client_module_.invoke<bool>("Steam_BReleaseSteamPipe", this->steam_pipe_);
				}
			}
		}

		const utils::nt::library& get_overlay_module() const
		{
			return steam_overlay_module_;
		}

	private:
		utils::nt::library steam_client_module_{};
		utils::nt::library steam_overlay_module_{};

		steam::interface client_engine_{};
		steam::interface client_user_{};
		steam::interface client_utils_{};

		void* steam_pipe_ = nullptr;
		void* global_user_ = nullptr;

		void* load_client_engine() const
		{
			if (!this->steam_client_module_) return nullptr;

			for (auto i = 1; i > 0; ++i)
			{
				std::string name = utils::string::va("CLIENTENGINE_INTERFACE_VERSION%03i", i);
				auto* const client_engine = this->steam_client_module_
				                                .invoke<void*>("CreateInterface", name.data(), nullptr);
				if (client_engine) return client_engine;
			}

			return nullptr;
		}

		void load_client()
		{
			const std::filesystem::path steam_path = steam::SteamAPI_GetSteamInstallPath();
			if (steam_path.empty()) return;

			utils::nt::library::load(steam_path / "tier0_s64.dll");
			utils::nt::library::load(steam_path / "vstdlib_s64.dll");
			this->steam_overlay_module_ = utils::nt::library::load(steam_path / "gameoverlayrenderer64.dll");
			this->steam_client_module_ = utils::nt::library::load(steam_path / "steamclient64.dll");
			if (!this->steam_client_module_) return;

			this->client_engine_ = load_client_engine();
			if (!this->client_engine_) return;

			this->steam_pipe_ = this->steam_client_module_.invoke<void*>("Steam_CreateSteamPipe");
			this->global_user_ = this->steam_client_module_.invoke<void*>(
				"Steam_ConnectToGlobalUser", this->steam_pipe_);
			this->client_user_ = this->client_engine_.invoke<void*>(8, this->steam_pipe_, this->global_user_);
			// GetIClientUser
			this->client_utils_ = this->client_engine_.invoke<void*>(14, this->steam_pipe_); // GetIClientUtils
		}

		ownership_state start_mod(const std::string& title, const size_t app_id)
		{
			__try
			{
				return this->start_mod_unsafe(title, app_id);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				this->do_cleanup();
				return ownership_state::error;
			}
		}

		ownership_state start_mod_unsafe(const std::string& title, size_t app_id)
		{
			if (!this->client_utils_ || !this->client_user_)
			{
				return ownership_state::nosteam;
			}

			if (!this->client_user_.invoke<bool>("BIsSubscribedApp", app_id))
			{
				//app_id = 480; // Spacewar
				return ownership_state::unowned;
			}

			this->client_utils_.invoke<void>("SetAppIDForCurrentPipe", app_id, false);

			char our_directory[MAX_PATH] = {0};
			GetCurrentDirectoryA(sizeof(our_directory), our_directory);

			const auto path = runner_file.get_extracted_file();
			const std::string cmdline = utils::string::va("\"%s\" -proc %d", path.data(), GetCurrentProcessId());

			steam::game_id game_id;
			game_id.raw.type = 1; // k_EGameIDTypeGameMod
			game_id.raw.app_id = app_id & 0xFFFFFF;

			const auto* mod_id = "bo3";
			game_id.raw.mod_id = *reinterpret_cast<const unsigned int*>(mod_id) | 0x80000000;

			this->client_user_.invoke<bool>("SpawnProcess", path.data(), cmdline.data(), our_directory,
			                                &game_id.bits, title.data(), 0, 0, 0);

			return ownership_state::success;
		}

		void do_cleanup()
		{
			this->client_engine_ = nullptr;
			this->client_user_ = nullptr;
			this->client_utils_ = nullptr;

			this->steam_pipe_ = nullptr;
			this->global_user_ = nullptr;

			this->steam_client_module_ = utils::nt::library{nullptr};
		}

		void clean_up_on_error()
		{
			scheduler::schedule([this]()
			{
				if (this->steam_client_module_
					&& this->steam_pipe_
					&& this->global_user_
					&& this->steam_client_module_.invoke<bool>("Steam_BConnected", this->global_user_,
					                                           this->steam_pipe_)
					&& this->steam_client_module_.invoke<bool>("Steam_BLoggedOn", this->global_user_, this->steam_pipe_)
				)
				{
					return scheduler::cond_continue;
				}

				this->do_cleanup();
				return scheduler::cond_end;
			});
		}
	};

	const utils::nt::library& get_overlay_module()
	{
		// TODO: Find a better way to do this
		return component_loader::get<component>()->get_overlay_module();
	}
}

REGISTER_COMPONENT(steam_proxy::component)
