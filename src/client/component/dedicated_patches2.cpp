#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"

#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "scheduler.hpp"
#include "command.hpp"

namespace server_patches2
{
	namespace
	{
		// Connection rate limiting per IP
		struct connect_attempt
		{
			uint32_t ip;
			std::chrono::steady_clock::time_point last_attempt;
			int attempts;
		};

		std::mutex rate_limit_mutex;
		std::unordered_map<uint32_t, connect_attempt> connect_attempts;
		constexpr int MAX_CONNECT_ATTEMPTS = 5;
		constexpr auto RATE_LIMIT_WINDOW = std::chrono::seconds(10);

		bool is_rate_limited(uint32_t ip)
		{
			std::lock_guard lock(rate_limit_mutex);
			const auto now = std::chrono::steady_clock::now();

			auto it = connect_attempts.find(ip);
			if (it == connect_attempts.end())
			{
				connect_attempts[ip] = {ip, now, 1};
				return false;
			}

			auto& attempt = it->second;
			if ((now - attempt.last_attempt) > RATE_LIMIT_WINDOW)
			{
				attempt.attempts = 1;
				attempt.last_attempt = now;
				return false;
			}

			attempt.attempts++;
			attempt.last_attempt = now;

			if (attempt.attempts > MAX_CONNECT_ATTEMPTS)
			{
				printf("[Security] Rate limited connection from IP 0x%08X (%d attempts)\n",
				       ip, attempt.attempts);
				return true;
			}

			return false;
		}

		void cleanup_rate_limits()
		{
			std::lock_guard lock(rate_limit_mutex);
			const auto now = std::chrono::steady_clock::now();

			for (auto it = connect_attempts.begin(); it != connect_attempts.end();)
			{
				if ((now - it->second.last_attempt) > std::chrono::seconds(60))
					it = connect_attempts.erase(it);
				else
					++it;
			}
		}

		// Chat message sanitization
		std::string sanitize_chat_message(const std::string& msg)
		{
			std::string result;
			result.reserve(msg.size());

			for (size_t i = 0; i < msg.size(); i++)
			{
				const unsigned char c = static_cast<unsigned char>(msg[i]);

				// Block control characters (except newline and tab)
				if (c < 0x20 && c != '\n' && c != '\t')
					continue;

				// Block DEL
				if (c == 0x7F)
					continue;

				// Block format string exploits (all dangerous C format specifiers)
				if (c == '%' && (i + 1) < msg.size())
				{
					const unsigned char next = static_cast<unsigned char>(msg[i + 1]);
					if (next == 'n' || next == 's' || next == 'x' || next == 'p' || next == 'd' ||
					    next == 'i' || next == 'u' || next == 'o' || next == 'f' || next == 'e' ||
					    next == 'g' || next == 'c' || next == 'X')
						continue;
				}

				// Block extended ASCII control chars (can cause rendering issues)
				if (c >= 0x80 && c <= 0x9F)
					continue;

				result += static_cast<char>(c);
			}

			// Limit message length
			if (result.size() > 512)
				result.resize(512);

			return result;
		}

		// sv_cheats enforcement
		void enforce_sv_cheats()
		{
			if (game::is_server_running())
			{
				const bool sv_cheats_val = game::get_dvar_bool("sv_cheats");
				if (sv_cheats_val)
				{
					printf("[Security] sv_cheats was non-zero (%d), forcing to 0\n", sv_cheats_val);
					game::set_dvar_bool("sv_cheats", false);
				}
			}
		}


		// Hook for G_Say to sanitize messages
		utils::hook::detour g_say_hook;

		void g_say_stub(game::gentity_s* ent, game::gentity_s* target, int mode, const char* chatText)
		{
			if (chatText)
			{
				const auto sanitized = sanitize_chat_message(chatText);
				g_say_hook.invoke(ent, target, mode, sanitized.data());
			}
			else
			{
				g_say_hook.invoke(ent, target, mode, chatText);
			}
		}

		// Hook for SV_DirectConnect to rate limit connections
		utils::hook::detour sv_direct_connect_hook;

		void sv_direct_connect_stub(game::netadr_t adr)
		{
			if (is_rate_limited(adr.addr))
			{
				printf("[Security] Rejected connection from rate-limited IP\n");
				return;
			}

			sv_direct_connect_hook.invoke(adr);
		}

		utils::hook::detour sv_removeallclientsfromaddress_hook;
		void sv_live_removeallclientsfromaddress_stub(game::client_s* client, const char* reason) {
			// Skip disconnecting other clients from the same IP -
			// just free the disconnected client's slot, and return. 
			game::SV_Live_RemoveClient(client, reason);
			return;
		}
		
		std::mutex reliable_cmd_mutex;
		// Map of reliable command string -> Map of xuid -> svs->time of last sequencing
		std::unordered_map<std::string, std::unordered_map<uint64_t, uint32_t>> client_openmenu_cmd_last_sequence_time;	
		// Map of xuid -> last sequenced reliable command string
		std::unordered_map<uint64_t, std::string> client_last_cmd;	
		
		utils::hook::detour g_init_game_hook;
		void g_init_game_stub(uint32_t levelTime, uint32_t randomSeed, game::qboolean restart, game::qboolean registerDvars, game::qboolean savegame) {
			std::lock_guard lock(reliable_cmd_mutex);

			// Reset tracked openmenu reliable cmds on starting a new game.
			for (auto& [cmd, client_map] : client_openmenu_cmd_last_sequence_time) {
				client_map.clear();
				client_openmenu_cmd_last_sequence_time.erase(cmd);

			}

			client_openmenu_cmd_last_sequence_time.clear();
			client_last_cmd.clear();

			g_init_game_hook.invoke(levelTime, randomSeed, restart, registerDvars, savegame);
		}
		
		utils::hook::detour sv_addservercommand_hook;

		void sv_addservercommand_stub(game::client_s* client, game::svscmd_type type, const char* cmd) {
			
			std::string cmd_str = cmd ? std::string(cmd) : "";
			std::lock_guard lock(reliable_cmd_mutex);

			/* 
				`openmenu` reliable commands have format "D %d %d %d %d", or "D %d %d %d". 
				Note that the prefix "D " is its unique command type identifier.
			*/
			if (utils::string::starts_with(cmd_str, "D ")) {
				// If this command was sent less than 1000 ms ago, skip.
				if (client_openmenu_cmd_last_sequence_time.contains(cmd_str) && 
					client_openmenu_cmd_last_sequence_time[cmd_str].contains(client->xuid) &&
					*(game::svs_time.get()) - client_openmenu_cmd_last_sequence_time[cmd_str][client->xuid] < 1000) {
					return;
				}

				// We also do not need to send a redundant openmenu command if it was the last command sent, even if sent > 1 second ago. 
				// This is valid because we can guarantee that menu state was not modified otherwise in the interim.
				if (client_last_cmd.contains(client->xuid) && client_last_cmd[client->xuid] == cmd_str) {
					return;
				}
			}

			client_openmenu_cmd_last_sequence_time[cmd_str][client->xuid] = *(game::svs_time.get());
			client_last_cmd[client->xuid] = cmd_str;

			sv_addservercommand_hook.invoke(client, type, cmd);
		}
	}

	struct component final : server_component
	{
		void post_unpack() override
		{
			// Sanitize chat messages on server
			g_say_hook.create(game::G_Say.get(), g_say_stub);

			/*
			 Some server configurations will require this to be disabled.
			 For example, if the server operates behind a reverse proxy, all incoming connections will appear to be from the same IP.
			 As such, clients will be erroneously rate limited, despite being unique connections.

			 In this case, rate limiting should be enforced in the server which is executing the reverse proxy.
			 Enforcement of rate limiting elsewhere will be the responsibility of the server operator using this abnormal configuration.
			*/
			if (!utils::flags::has_flag("noratelimit")) {
				// Rate limit connections
				sv_direct_connect_hook.create(game::SV_DirectConnect.get(), sv_direct_connect_stub);
			}

			// RCE Prevention: Patch Cmd_ParseArgs to prevent remote code execution
			// Makes the vulnerable function immediately return, blocking crafted
			// command strings from executing arbitrary code on the server
			utils::hook::set<uint8_t>(0x1404B2E00_g, 0xC3);

			// TeamOps arbitrary write fix: NOP the inlined arbitrary write
			// that allows attackers to write to arbitrary memory via team operations
			utils::hook::nop(0x1401155D5_g, 7);

			/*
				Disable removal of all clients from an IP address if
				one client from that IP address disconnects.
				
				Useful if e.g. server is hosted behind a reverse proxy or 
				load balancer where multiple clients share the same IP.
			*/
			sv_removeallclientsfromaddress_hook.create(game::SV_Live_RemoveAllClientsFromAddress.get(), sv_live_removeallclientsfromaddress_stub);

			// Enforce sv_cheats = 0 periodically
			scheduler::loop([]
			{
				enforce_sv_cheats();
			}, scheduler::pipeline::server, 5000ms);

			if (!utils::flags::has_flag("noratelimit")) {
				// Cleanup old rate limit entries periodically
				scheduler::loop([]
				{
					cleanup_rate_limits();
				}, scheduler::pipeline::async, 30000ms);
			}

			/*
				Some custom maps, especially those with custom HUDs, update HUD state with server-side logic.
				Often, this is implemented through a spin-loop which executes `luinotify`s every 500ms to update HUD state, regardless of whether
				there has been a change in state.

				When executed client-side, in a singleplayer game, this works well.

				Unfortunately, when executed in either dedicated server or when hosting a private match, each one of these `luinotify`s results in a packet sent to each non-host client.
				
				In two extreme cases - in the custom maps Kowloon and Daybreak - this results in a constant, massive flood of redundant `openmenu` reliable commands being sent to each client.
				While inefficient, this is generally acceptable. However, when the client is completing load-in to the map, 
				in the initial blackscreen before they begin playing, reliable commands are temporarily unhandled. 
				In the case of the aforementioned examples, this results in a near-consistent inability for clients to succesfully load into the map, 
				instead resulting in an `EXE_ERR_RELIABLE_CYCLED_OUT` error, as the flood of hundreds of menu update packets are unhandled.

				Inability to reliably play these maps online has been noted many times in the steamcommunity workshop pages for these maps. 
				Neither has been updated to resolve the issue, and new maps are often created with HUD update logic which contains similarly poor design.

				To mitigate this, these duplicative menu update packets without state change can be handled and filtered out server-side.
			*/

			if (utils::flags::has_flag("mitigatepacketspam")) {
				sv_addservercommand_hook.create(game::SV_AddServerCommand.get(), sv_addservercommand_stub);
				g_init_game_hook.create(game::G_InitGame.get(), g_init_game_stub);
			}



		}
	};
}

REGISTER_COMPONENT(server_patches2::component)
