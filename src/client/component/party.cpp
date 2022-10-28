#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "network.hpp"
#include "scheduler.hpp"
#include "game/game.hpp"
#include "steam/steam.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/info_string.hpp>
#include <utils/cryptography.hpp>

#include <version.hpp>

namespace party
{
	namespace
	{
		struct
		{
			game::netadr_t host{{}, {}, game::NA_BAD, {}};
			std::string challenge{};
		} connect_state{};

		void connect_stub(const char* address)
		{
			const auto target = network::address_from_string(address);
			if (target.type == game::NA_BAD)
			{
				return;
			}

			connect_state.host = target;
			connect_state.challenge = utils::cryptography::random::get_challenge();

			network::send(target, "getInfo", connect_state.challenge);
		}

		void connect_to_lobby(const game::netadr_t& addr, const std::string& mapname, const std::string& gamemode)
		{
			game::XSESSION_INFO info{};
			game::CL_ConnectFromLobby(0, &info, &addr, 1, 0, mapname.data(), gamemode.data());
		}

		void launch_mode(const game::eModes mode)
		{
			const auto* command = "";

			switch (mode)
			{
			case game::MODE_CAMPAIGN:
				command = "startCampaign;";
				break;
			case game::MODE_ZOMBIES:
				command = "startZombies;";
				break;
			case game::MODE_MULTIPLAYER:
				command = "startMultiplayer;";
				break;
			default:
				return;
			}

			game::Cbuf_AddText(0, command);
		}

		void connect_to_lobby_with_mode(const game::netadr_t& addr, const game::eModes mode, const std::string& mapname,
		                                const std::string& gametype, const bool was_retried = false)
		{
			if (game::Com_SessionMode_IsMode(mode))
			{
				connect_to_lobby(addr, mapname, gametype);
				return;
			}

			if (!was_retried)
			{
				scheduler::once([=]
				{
					connect_to_lobby_with_mode(addr, mode, mapname, gametype, true);
				}, scheduler::main, 5s);

				launch_mode(mode);
			}
		}

		std::string get_dvar_string(const char* dvar_name)
		{
			const auto dvar = game::Dvar_FindVar(dvar_name);
			if (!dvar)
			{
				return {};
			}

			return game::Dvar_GetString(dvar);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			utils::hook::jump(0x141EE6030_g, connect_stub);

			network::on("getInfo", [](const game::netadr_t& target, const network::data_view& data)
			{
				utils::info_string info{};
				info.set("challenge", std::string(data.begin(), data.end()));
				info.set("gamename", "T7");
				info.set("hostname", get_dvar_string("sv_hostname"));
				info.set("gametype", get_dvar_string("g_gametype"));
				//info.set("sv_motd", get_dvar_string("sv_motd"));
				info.set("xuid", utils::string::va("%llX", steam::SteamUser()->GetSteamID().bits));
				info.set("mapname", get_dvar_string("mapname"));
				//info.set("isPrivate", get_dvar_string("g_password").empty() ? "0" : "1");
				//info.set("clients", utils::string::va("%i", get_client_count()));
				//info.set("bots", utils::string::va("%i", get_bot_count()));
				//info.set("sv_maxclients", utils::string::va("%i", *game::mp::svs_numclients));
				info.set("protocol", utils::string::va("%i", 1/*PROTOCOL*/));
				info.set("playmode", utils::string::va("%i", game::Com_SessionMode_GetMode()));
				//info.set("sv_running", utils::string::va("%i", get_dvar_bool("sv_running")));
				//info.set("dedicated", utils::string::va("%i", get_dvar_bool("dedicated")));
				info.set("shortversion", SHORTVERSION);

				network::send(target, "infoResponse", info.build(), '\n');
			});

			network::on("infoResponse", [](const game::netadr_t& target, const network::data_view& data)
			{
				const utils::info_string info{data};

				if (connect_state.host != target)
				{
					return;
				}

				if (info.get("challenge") != connect_state.challenge)
				{
					const auto str = "Invalid challenge.";
					printf("%s\n", str);
					return;
				}

				const auto gamename = info.get("gamename");
				if (gamename != "T7"s)
				{
					const auto str = "Invalid gamename.";
					printf("%s\n", str);
					return;
				}

				const auto mapname = info.get("mapname");
				if (mapname.empty())
				{
					const auto str = "Invalid map.";
					printf("%s\n", str);
					return;
				}

				const auto gametype = info.get("gametype");
				if (gametype.empty())
				{
					const auto str = "Invalid gametype.";
					printf("%s\n", str);
					return;
				}

				const auto playmode = info.get("playmode");
				const auto mode = game::eModes(std::atoi(playmode.data()));

				scheduler::once([=]
				{
					connect_to_lobby_with_mode(target, mode, mapname, gametype);
				}, scheduler::main);
			});
		}
	};
}

REGISTER_COMPONENT(party::component)
