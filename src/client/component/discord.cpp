#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "scheduler.hpp"
#include "discord.hpp"
#include "party.hpp"

#include <discord_rpc.h>
#include <ctime>
#include <unordered_map>
#include <regex>

namespace discord
{
	namespace
	{
		constexpr auto DISCORD_APP_ID = "967371125573177474";
		constexpr auto UPDATE_INTERVAL = 5s;
		constexpr size_t MAX_SERVER_NAME_LENGTH = 32;

		time_t start_timestamp = 0;
		time_t match_timestamp = 0;
		bool was_in_game = false;
		std::atomic_bool engine_ready = false;
		std::atomic_int player_score = 0;
		std::atomic_int enemy_score = 0;

		const std::unordered_map<std::string, std::string> map_display_names = {
			{"mp_combine", "Combine"},
			{"mp_biodome", "Aquarium"},
			{"mp_redwood", "Redwood"},
			{"mp_stronghold", "Stronghold"},
			{"mp_nuketown_x", "Nuk3town"},
			{"mp_apartments", "Evac"},
			{"mp_havoc", "Havoc"},
			{"mp_ethiopia", "Hunted"},
			{"mp_infection", "Infection"},
			{"mp_metro", "Metro"},
			{"mp_exodus", "Exodus"},
			{"mp_sector", "Breach"},
			{"mp_spire", "Spire"},
			{"mp_veiled", "Rift"},
			{"mp_crucible", "Verge"},
			{"mp_rise", "Rise"},
			{"mp_skyjacked", "Skyjacked"},
			{"mp_waterpark", "Splash"},
			{"mp_chinatown", "Knockout"},
			{"mp_kung_fu", "Empire"},
			{"mp_arena", "Gauntlet"},
			{"mp_ruins", "Ruins"},
			{"mp_rome", "Citadel"},
			{"mp_miniature", "Micro"},
			{"mp_banzai", "Berserk"},
			{"mp_platform", "Cryogen"},
			{"mp_conduit", "Rumble"},
			{"mp_aerospace2", "Outlaw"},
			{"zm_zod", "Shadows of Evil"},
			{"zm_factory", "The Giant"},
			{"zm_castle", "Der Eisendrache"},
			{"zm_island", "Zetsubou No Shima"},
			{"zm_stalingrad", "Gorod Krovi"},
			{"zm_genesis", "Revelations"},
			{"zm_prototype", "Nacht Der Untoten"},
			{"zm_asylum", "Verruckt"},
			{"zm_sumpf", "Shi No Numa"},
			{"zm_theater", "Kino Der Toten"},
			{"zm_cosmodrome", "Ascension"},
			{"zm_temple", "Shangri-La"},
			{"zm_moon", "Moon"},
			{"zm_tomb", "Origins"},
		};

		const std::unordered_map<std::string, std::string> gametype_display_names = {
			{"tdm", "Team Deathmatch"},
			{"dm", "Free-For-All"},
			{"ffa", "Free-For-All"},
			{"dom", "Domination"},
			{"sd", "Search & Destroy"},
			{"hp", "Hardpoint"},
			{"ctf", "Capture The Flag"},
			{"kc", "Kill Confirmed"},
			{"conf", "Kill Confirmed"},
			{"gun", "Gun Game"},
			{"sas", "Safeguard"},
			{"snipe", "One Shot"},
			{"oic", "One in the Chamber"},
			{"sharp", "Sharpshooter"},
			{"prop", "Prop Hunt"},
			{"ball", "Uplink"},
			{"infect", "Infected"},
			{"dem", "Demolition"},
			{"clean", "Search & Rescue"},
			{"zom", "Zombies"},
		};

		std::string strip_color_codes(const std::string& str)
		{
			std::string result;
			result.reserve(str.length());

			for (size_t i = 0; i < str.length(); ++i)
			{
				if (str[i] == '^' && i + 1 < str.length())
				{
					const char next = str[i + 1];
					if ((next >= '0' && next <= '9') || next == ':' || next == ';')
					{
						++i;
						continue;
					}
				}
				result += str[i];
			}

			return result;
		}

		std::string truncate_string(const std::string& str, size_t max_length)
		{
			if (str.length() <= max_length)
			{
				return str;
			}
			return str.substr(0, max_length - 3) + "...";
		}

		std::string get_display_map_name(const std::string& raw_name)
		{
			if (const auto it = map_display_names.find(raw_name); it != map_display_names.end())
			{
				return it->second;
			}

			if (raw_name.starts_with("mp_") || raw_name.starts_with("zm_"))
			{
				auto name = raw_name.substr(3);
				if (!name.empty())
				{
					name[0] = static_cast<char>(std::toupper(name[0]));
				}
				return name;
			}

			return raw_name.empty() ? "Unknown" : raw_name;
		}

		std::string get_display_gametype(const std::string& raw_type)
		{
			if (const auto it = gametype_display_names.find(raw_type); it != gametype_display_names.end())
			{
				return it->second;
			}

			return raw_type.empty() ? "Unknown" : raw_type;
		}

		const char* get_map_image_key(const std::string& raw_name)
		{
			return map_display_names.contains(raw_name) ? raw_name.c_str() : "logo";
		}

		std::string get_dvar_string(const char* dvar_name)
		{
			if (const auto* dvar = game::Dvar_FindVar(dvar_name))
			{
				return game::Dvar_GetString(dvar);
			}
			return {};
		}

		bool get_dvar_bool(const char* dvar_name)
		{
			if (const auto* dvar = game::Dvar_FindVar(dvar_name))
			{
				return dvar->current.value.enabled;
			}
			return false;
		}

		int get_player_count()
		{
			return game::LobbyHost_GetClientCount(game::LOBBY_TYPE_GAME, game::LOBBY_CLIENT_TYPE_ALL);
		}

		void reset_match_state()
		{
			was_in_game = false;
			player_score = 0;
			enemy_score = 0;
			party::clear_server_info();
		}

		void update_presence_loading()
		{
			DiscordRichPresence presence{};
			presence.instance = 1;
			presence.startTimestamp = start_timestamp;
			presence.details = "Playing BO3 via Ezz!";
			presence.state = "Starting...";
			presence.largeImageKey = "logo";
			presence.smallImageKey = "sexy";
			presence.button1_label = "Join our Discord \xF0\x9F\x91\x8D";
			presence.button1_url = "https://discord.gg/ezz";
			presence.button2_label = "Ezz Forum \xF0\x9F\xA4\x96";
			presence.button2_url = "https://forum.ezz.lol";

			Discord_UpdatePresence(&presence);
		}

		void update_presence_menu()
		{
			DiscordRichPresence presence{};
			presence.instance = 1;
			presence.startTimestamp = start_timestamp;
			presence.details = game::Com_IsRunningUILevel() ? "In Menus" : "Playing BO3 via Ezz!";
			presence.state = "Main Menu";
			presence.largeImageKey = "logo";
			presence.largeImageText = "Playing BO3 via Ezz!";
			presence.smallImageKey = "sexy";
			presence.button1_label = "Join our Discord \xF0\x9F\x91\x8D";
			presence.button1_url = "https://discord.gg/ezz";
			presence.button2_label = "Ezz Forum \xF0\x9F\xA4\x96";
			presence.button2_url = "https://forum.ezz.lol";

			Discord_UpdatePresence(&presence);
		}

		void update_presence_ingame()
		{
			if (!was_in_game)
			{
				match_timestamp = time(nullptr);
				was_in_game = true;
				player_score = 0;
				enemy_score = 0;
			}

			const auto map_name = get_dvar_string("mapname");
			const auto gametype = get_dvar_string("g_gametype");
			const auto display_map = get_display_map_name(map_name);
			const auto display_type = get_display_gametype(gametype);

			const bool is_zombies = game::Com_SessionMode_IsMode(game::MODE_ZOMBIES);
			const bool is_mp = game::Com_SessionMode_IsMode(game::MODE_MULTIPLAYER);
			const bool is_campaign = game::Com_SessionMode_IsMode(game::MODE_CAMPAIGN);

			auto raw_server_name = party::get_server_hostname();
			const int max_clients = party::get_server_max_clients();
			const bool on_server = !raw_server_name.empty();

			std::string server_name;
			if (on_server)
			{
				server_name = strip_color_codes(raw_server_name);
				server_name = truncate_string(server_name, MAX_SERVER_NAME_LENGTH);
			}

			const std::string details = on_server ? server_name : "Private Game";

			std::string state;
			std::string mode_str;
			if (is_zombies) mode_str = "Zombies";
			else if (is_mp) mode_str = "Multiplayer";
			else if (is_campaign) mode_str = "Campaign";

			if (!mode_str.empty() && !map_name.empty())
			{
				state = mode_str + " on " + display_map;
			}
			else if (!map_name.empty())
			{
				state = "Playing " + display_map;
			}
			else
			{
				state = "In Game";
			}

			if (is_mp)
			{
				state += " | " + std::to_string(player_score.load()) + " - " + std::to_string(enemy_score.load());
			}
			else if (is_zombies)
			{
				const int round = *game::level_rounds_played;
				if (round > 0)
				{
					state += " | Round " + std::to_string(round);
				}
			}

			std::string large_text;
			if (!mode_str.empty())
			{
				large_text = mode_str + " - " + display_map;
			}
			else
			{
				large_text = display_map;
			}

			DiscordRichPresence presence{};
			presence.instance = 1;
			presence.startTimestamp = match_timestamp;
			presence.details = details.c_str();
			presence.state = state.c_str();
			presence.largeImageKey = get_map_image_key(map_name);
			presence.largeImageText = large_text.c_str();
			presence.smallImageKey = "logo";
			presence.smallImageText = "Playing BO3 via Ezz!";
			presence.button1_label = "Join our Discord \xF0\x9F\x91\x8D";
			presence.button1_url = "https://discord.gg/ezz";
			presence.button2_label = "Ezz Forum \xF0\x9F\xA4\x96";
			presence.button2_url = "https://forum.ezz.lol";

			{
				int party_max = max_clients;
				if (party_max <= 0)
				{
					const auto* maxclients_dvar = game::Dvar_FindVar("com_maxclients");
					if (maxclients_dvar) party_max = maxclients_dvar->current.value.integer;
				}

				int party_size = get_player_count();
				if (party_size <= 0) party_size = 1;

				if (party_max > 0)
				{
					presence.partySize = party_size;
					presence.partyMax = party_max;
				}
			}

			Discord_UpdatePresence(&presence);
		}

		void update_presence()
		{
			if (!engine_ready)
			{
				update_presence_loading();
				return;
			}

			if (game::Com_IsInGame())
			{
				update_presence_ingame();
			}
			else
			{
				reset_match_state();
				update_presence_menu();
			}
		}

		void ready(const DiscordUser* request)
		{
			SetEnvironmentVariableA("discord_user", request->userId);
			printf("Discord: Ready: %s - %s\n", request->userId, request->username);
			update_presence();
		}

		void errored(const int error_code, const char* message)
		{
			printf("Discord: Error (%i): %s\n", error_code, message);
		}
	}

	void set_player_score(const int score)
	{
		player_score = score;
	}

	void set_enemy_score(const int score)
	{
		enemy_score = score;
	}

	class component final : public client_component
	{
	public:
		void post_load() override
		{
			start_timestamp = time(nullptr);

			DiscordEventHandlers handlers{};
			handlers.ready = ready;
			handlers.errored = errored;
			handlers.disconnected = errored;

			Discord_Initialize(DISCORD_APP_ID, &handlers, 1, nullptr);
			initialized_ = true;

			scheduler::loop([]
			{
				Discord_RunCallbacks();
				update_presence();
			}, scheduler::pipeline::async, UPDATE_INTERVAL);
		}

		void post_unpack() override
		{
			engine_ready = true;
		}

		void pre_destroy() override
		{
			if (initialized_)
			{
				Discord_Shutdown();
			}
		}

	private:
		bool initialized_ = false;
	};
}

REGISTER_COMPONENT(discord::component)