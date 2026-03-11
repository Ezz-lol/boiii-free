#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/nt.hpp>
#include <utils/flags.hpp>
#include <utils/string.hpp>
#include <utils/hook.hpp>
#include <utils/finally.hpp>
#include <utils/concurrency.hpp>

#include "steam/interface.hpp"
#include "steam/steam.hpp"

#include "steam_proxy.hpp"

#include "scheduler.hpp"

#define NO_CHECK

namespace steam_proxy
{
	namespace
	{
		utils::nt::library steam_client_module{};
		utils::nt::library steam_overlay_module{};

		steam::HSteamPipe steam_pipe = 0;
		steam::HSteamPipe original_pipe = 0;
		steam::HSteamUser global_user = 0;

		steam::interface client_engine{};
		steam::interface client_user{};
		steam::interface client_utils{};
		steam::interface client_friends{};
		steam::interface client_ugc{};

		// real ISteamFriends from steamclient64.dll for rich presence/invites
		steam::interface steam_friends_real{};

		steam::client* steam_client{};
		steam::ugc* steam_ugc{};

		utils::concurrency::container<subscribed_item_map> subscribed_items;

		// Steam callback interception via hook
		std::mutex pending_connect_mutex;
		std::string pending_connect_addr;
		uint64_t pending_connect_friend_id{0}; // steam ID of whoever invited us

		// Steam callback message struct (matches Valve's CallbackMsg_t)
		struct callback_msg_t
		{
			int32_t steam_user;
			int callback_id;
			uint8_t* data;
			int data_size;
		};

		// Callback IDs we care about
		constexpr int k_GameRichPresenceJoinRequested = 337; // k_iSteamFriendsCallbacks + 37
		constexpr int k_GameLobbyJoinRequested = 333;        // k_iSteamFriendsCallbacks + 33

		struct game_rich_presence_join_requested_t
		{
			uint64_t steam_id_friend;
			char connect[256];
		};

		struct game_lobby_join_requested_t
		{
			uint64_t steam_id_lobby;
			uint64_t steam_id_friend;
		};

		utils::hook::detour steam_bgetcallback_hook;

		bool steam_bgetcallback_stub(int32_t pipe, callback_msg_t* msg)
		{
			const auto result = steam_bgetcallback_hook.invoke<bool>(pipe, msg);
			if (!result || !msg) return result;

			if (msg->callback_id == k_GameRichPresenceJoinRequested && msg->data
				&& msg->data_size >= static_cast<int>(sizeof(game_rich_presence_join_requested_t)))
			{
				const auto* join = reinterpret_cast<const game_rich_presence_join_requested_t*>(msg->data);
				std::string connect_str(join->connect);
				printf("[SteamProxy] Invite received: connect='%s' from=%llu\n",
					connect_str.c_str(), join->steam_id_friend);
				if (!connect_str.empty())
				{
					std::lock_guard lock(pending_connect_mutex);
					pending_connect_addr = connect_str;
					pending_connect_friend_id = join->steam_id_friend;
				}
			}

			if (msg->callback_id == k_GameLobbyJoinRequested && msg->data
				&& msg->data_size >= static_cast<int>(sizeof(game_lobby_join_requested_t)))
			{
				const auto* lobby = reinterpret_cast<const game_lobby_join_requested_t*>(msg->data);
				printf("[SteamProxy] Lobby join request: lobby=%llu friend=%llu\n",
					lobby->steam_id_lobby, lobby->steam_id_friend);
			}

			return result;
		}

		std::atomic_bool hook_installed{false};

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

			for (auto i = 1; i <= 999; ++i)
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
			SetEnvironmentVariableA("SteamAppId", "480");

			const std::filesystem::path steam_path = steam::SteamAPI_GetSteamInstallPath();
			if (steam_path.empty()) return;

			utils::nt::library::load(steam_path / "tier0_s64.dll");
			utils::nt::library::load(steam_path / "vstdlib_s64.dll");
			steam_overlay_module = utils::nt::library::load(steam_path / "gameoverlayrenderer64.dll");
			steam_client_module = utils::nt::library::load(steam_path / "steamclient64.dll");
			if (!steam_client_module) return;

			client_engine = load_client_engine();
			if (!client_engine) return;

			steam_pipe = steam_client_module.invoke<steam::HSteamPipe>("Steam_CreateSteamPipe");
			original_pipe = steam_pipe; // save for callback pumping fallback
			global_user = steam_client_module.invoke<steam::HSteamUser>(
				"Steam_ConnectToGlobalUser", steam_pipe);

			client_user = client_engine.invoke<void*>(8, global_user, steam_pipe);
			client_utils = client_engine.invoke<void*>(14, steam_pipe);
			client_friends = client_engine.invoke<void*>(13, global_user, steam_pipe);
			client_ugc = client_engine.invoke<void*>(62, global_user, steam_pipe);

		}

		void do_cleanup()
		{
			client_engine = nullptr;
			client_user = nullptr;
			client_utils = nullptr;
			client_friends = nullptr;
			client_ugc = nullptr;
			steam_friends_real = nullptr;

			steam_pipe = 0;
			original_pipe = 0;
			global_user = 0;

			steam_client = nullptr;
			steam_ugc = nullptr;
		}

		bool perform_cleanup_if_needed()
		{
			if (steam_client) return true;

			if (steam_client_module
				&& steam_pipe
				&& global_user
				&& steam_client_module.invoke<bool>("Steam_BConnected", global_user, steam_pipe)
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

			app_id = 480;

			if (is_disabled())
			{
				return ownership_state::success;
			}

			client_utils.invoke<void>("SetAppIDForCurrentPipe", app_id, false);

			char our_directory[MAX_PATH] = {0};
			GetCurrentDirectoryA(sizeof(our_directory), our_directory);

			const auto self = utils::nt::library::get_by_address(start_mod_unsafe);
			const auto path = self.get_path();
			const auto* cmdline = utils::string::va("\"%s\" -proc %d", path.generic_string().data(),
			                                        GetCurrentProcessId());

			steam::game_id game_id;
			game_id.raw.type = 0; // k_EGameIDTypeApp
			game_id.raw.app_id = app_id & 0xFFFFFF;
			game_id.raw.mod_id = 0;

			client_user.invoke<bool>("SpawnProcess", path.generic_string().data(), cmdline, our_directory,
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

		void evaluate_ownership_state(const ownership_state state)
		{
#if defined(DEV_BUILD) || defined(NO_CHECK)
			(void)state;
#else
			switch (state)
			{
			case ownership_state::nosteam:
			//throw std::runtime_error("Steam must be running to play this game!");
			case ownership_state::unowned:
			//throw std::runtime_error("You must own the game on steam to play this mod!");
			case ownership_state::error:
			//throw std::runtime_error("Failed to verify ownership of the game!");
			case ownership_state::success:
				break;
			}
#endif
		}
	}

	struct component final : client_component
	{
		void post_load() override
		{
			load_client();
			perform_cleanup_if_needed();
		}

		void post_unpack() override
		{
			const auto res = start_mod("\xE2\x98\x84\xEF\xB8\x8F" " BOIII"s, steam::SteamUtils()->GetAppID());
			evaluate_ownership_state(res);
			clean_up_on_error();

			// install callback hook immediately so we don't miss any invites
			if (steam_client_module && !hook_installed.load())
			{
				auto* bget = steam_client_module.get_proc<void*>("Steam_BGetCallback");
				if (bget)
				{
					steam_bgetcallback_hook.create(bget, &steam_bgetcallback_stub);
					hook_installed.store(true);
					printf("[SteamProxy] Hooked Steam_BGetCallback\n");
				}
			}

			// actively pump Steam callbacks so invite notifications (337) are received
			scheduler::loop([]
			{
				if (!steam_client_module || !hook_installed.load()) return;

				static auto bget_fn = steam_client_module.get_proc<bool(*)(int32_t, callback_msg_t*)>("Steam_BGetCallback");
				static auto bfree_fn = steam_client_module.get_proc<void(*)(int32_t)>("Steam_FreeLastCallback");
				if (!bget_fn || !bfree_fn) return;

				auto pump = [](int32_t pipe)
				{
					if (!pipe) return;
					callback_msg_t msg{};
					while (bget_fn(pipe, &msg))
					{
						bfree_fn(pipe);
					}
				};

				pump(static_cast<int32_t>(steam_pipe));
				if (original_pipe && original_pipe != steam_pipe)
					pump(static_cast<int32_t>(original_pipe));
			}, scheduler::async, 500ms);

		}

		void pre_destroy() override
		{
			if (steam_client)
			{
				if (global_user)
				{
					steam_client->ReleaseUser(steam_pipe, global_user);
				}

				steam_client->BReleaseSteamPipe(steam_pipe);
			}
			else if (steam_client_module && steam_pipe)
			{
				if (global_user)
				{
					steam_client_module.invoke<void>("Steam_ReleaseUser", steam_pipe,
					                                 global_user);
				}

				(void)steam_client_module.invoke<bool>("Steam_BReleaseSteamPipe", steam_pipe);
			}
		}

		component_priority priority() const override
		{
			return component_priority::steam_proxy;
		}
	};

	const utils::nt::library& get_overlay_module()
	{
		return steam_overlay_module;
	}

	bool set_rich_presence(const std::string& key, const std::string& value)
	{
		if (steam_friends_real)
		{
			try { return steam_friends_real.invoke<bool>(43, key.c_str(), value.c_str()); }
			catch (...) {}
		}
		if (client_friends)
		{
			try { return client_friends.invoke<bool>("SetRichPresence", 480u, key.c_str(), value.c_str()); }
			catch (...) {}
		}
		return false;
	}

	void clear_rich_presence()
	{
		if (steam_friends_real)
		{
			try { steam_friends_real.invoke<void>(44); return; }
			catch (...) {}
		}
		if (client_friends)
		{
			try { client_friends.invoke<void>("ClearRichPresence", 480u); }
			catch (...) {}
		}
	}

	const char* get_player_name()
	{
		if (client_friends)
		{
			return client_friends.invoke<const char*>("GetPersonaName");
		}

		return "";
	}

	void initialize()
	{
		if (!steam_client_module) return;

		steam_client = steam_client_module.invoke<steam::client*>("CreateInterface", "SteamClient017", nullptr);
		if (!steam_client) return;

		steam_pipe = steam_client->CreateSteamPipe();
		global_user = steam_client->ConnectToGlobalUser(steam_pipe);

		steam::interface rc(static_cast<void*>(steam_client));
		auto* fp = rc.invoke<void*>(8, global_user, steam_pipe, "SteamFriends015");
		if (fp) steam_friends_real = steam::interface(fp);
	}

	void create_ugc()
	{
		if (!steam_client) return;

		auto* ugc = steam_client->GetISteamUGC(global_user, steam_pipe, "STEAMUGC_INTERFACE_VERSION008");
		steam_ugc = static_cast<steam::ugc*>(ugc);
	}

	void update_map_client(subscribed_item_map& map)
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

	void update_map_steam(subscribed_item_map& map)
	{
		const auto num_items = steam_ugc->GetNumSubscribedItems();

		if (!num_items)
		{
			return;
		}

		std::vector<uint64_t> ids;
		ids.resize(num_items);

		auto result = steam_ugc->GetSubscribedItems(ids.data(), num_items);
		result = std::min(num_items, result);

		for (uint32_t i = 0; i < result; ++i)
		{
			char buffer[0x1000] = {0};
			subscribed_item item{};

			item.state = steam_ugc->GetItemState(ids[i]);
			item.available = steam_ugc->GetItemInstallInfo(ids[i], &item.size_on_disk, buffer, sizeof(buffer),
			                                               &item.time_stamp);
			item.path = buffer;

			map[ids[i]] = std::move(item);
		}
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

		try
		{
			if (client_ugc)
			{
				update_map_client(map);
			}
			else if (steam_ugc)
			{
				update_map_steam(map);
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

	void access_steam_friends(const std::function<void(const std::vector<std::pair<uint64_t, std::string>>&)>& callback)
	{
		std::vector<std::pair<uint64_t, std::string>> result;
		try
		{
			if (!client_friends) { callback(result); return; }

			const auto count = client_friends.invoke<int>("GetFriendCount", 0x04);
			if (count <= 0) { callback(result); return; }

			for (int i = 0; i < count; ++i)
			{
				const auto id = client_friends.invoke<uint64_t>("GetFriendByIndex", i, 0x04);
				if (id == 0) continue;

				steam_id sid{};
				sid.bits = id;
				const auto state = client_friends.invoke<int>("GetFriendPersonaState", sid);
				if (state == 0) continue;

				const auto* name = client_friends.invoke<const char*>("GetFriendPersonaName", sid);
				result.emplace_back(id, name ? name : "Unknown");
			}
		}
		catch (...) {}
		callback(result);
	}

	void invite_friend(uint64_t xuid, const std::string& connect_string)
	{
		steam_id sid{};
		sid.bits = xuid;

		printf("[SteamProxy] Sending invite to %llu, connect=%s\n", xuid, connect_string.c_str());

		// set rich presence invite key so receiver can detect it via RP polling
		set_rich_presence("boiii_invite", utils::string::va("%llu:%s", xuid, connect_string.c_str()));

		if (steam_friends_real)
		{
			try
			{
				auto result = steam_friends_real.invoke<bool>(49, sid, connect_string.c_str());
				printf("[SteamProxy] InviteUserToGame = %s\n", result ? "true" : "false");
				return;
			}
			catch (...) {}
		}
		if (client_friends)
		{
			try
			{
				client_friends.invoke<bool>("InviteUserToGame", sid, connect_string.c_str());
			}
			catch (...) {}
		}
	}

	std::string get_pending_game_invite(uint64_t* out_friend_id)
	{
		if (out_friend_id) *out_friend_id = 0;

		// check +connect from command line (launched from Steam invite)
		static bool checked_cmdline = false;
		if (!checked_cmdline)
		{
			checked_cmdline = true;
			const auto* cmd = GetCommandLineA();
			if (cmd)
			{
				std::string cmdline(cmd);
				auto pos = cmdline.find("+connect ");
				if (pos != std::string::npos)
				{
					auto addr = cmdline.substr(pos + 9);
					auto end = addr.find(' ');
					if (end != std::string::npos) addr = addr.substr(0, end);
					if (!addr.empty()) return addr;
				}
			}
		}

		// fallback: try to install hook if not done yet (e.g. post_unpack was too early)
		if (!hook_installed.load() && steam_client_module)
		{
			auto* bget = steam_client_module.get_proc<void*>("Steam_BGetCallback");
			if (bget)
			{
				steam_bgetcallback_hook.create(bget, &steam_bgetcallback_stub);
				hook_installed.store(true);
				printf("[SteamProxy] Hooked Steam_BGetCallback (late)\n");
			}
		}

		{
			std::lock_guard lock(pending_connect_mutex);
			if (!pending_connect_addr.empty())
			{
				auto result = std::move(pending_connect_addr);
				pending_connect_addr.clear();
				if (out_friend_id) *out_friend_id = pending_connect_friend_id;
				pending_connect_friend_id = 0;
				return result;
			}
		}

		return "";
	}

	std::string get_pending_game_invite()
	{
		return get_pending_game_invite(nullptr);
	}

	std::string get_steam_friend_name(uint64_t friend_steam_id)
	{
		if (!client_friends || friend_steam_id == 0) return "";
		try
		{
			steam_id sid{};
			sid.bits = friend_steam_id;
			const auto* name = client_friends.invoke<const char*>("GetFriendPersonaName", sid);
			if (name && name[0]) return name;
		}
		catch (...) {}
		return "";
	}

	uint64_t get_own_steam_id()
	{
		// Try client_user fis better
		if (client_user)
		{
			try
			{
				auto id = client_user.invoke<uint64_t>("GetSteamID");
				if (id > 0x0110000100000000ULL) return id;
			}
			catch (...) {}
		}

		// Try SteamClient017 path
		if (steam_client && global_user && steam_pipe)
		{
			try
			{
				steam::interface rc(static_cast<void*>(steam_client));
				auto* user_iface = rc.invoke<void*>(5, global_user, steam_pipe, "SteamUser021");
				if (user_iface)
				{
					steam::interface su(user_iface);
					auto id = su.invoke<uint64_t>(0); // GetSteamID is index 0
					if (id > 0x0110000100000000ULL) return id;
				}
			}
			catch (...) {}
		}

		// fallback: use emulated steam user
		return steam::SteamUser() ? steam::SteamUser()->GetSteamID().bits : 0;
	}

	std::string get_friend_rich_presence(uint64_t friend_id, const std::string& key)
	{
		if (!steam_friends_real || friend_id == 0) return "";
		try
		{
			steam_id sid{};
			sid.bits = friend_id;
			const auto* val = steam_friends_real.invoke<const char*>(45, sid, key.c_str());
			if (val && val[0]) return val;
		}
		catch (...) {}
		return "";
	}

	void request_friend_rich_presence(uint64_t friend_id)
	{
		if (!steam_friends_real || friend_id == 0) return;
		try
		{
			steam_id sid{};
			sid.bits = friend_id;
			steam_friends_real.invoke<void>(48, sid);
		}
		catch (...) {}
	}

	void clear_invite_presence()
	{
		set_rich_presence("boiii_invite", "");
	}
}

REGISTER_COMPONENT(steam_proxy::component)
