#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"

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
			if (!game::is_server_running())
				return;

			const auto sv_cheats_val = game::get_dvar_int("sv_cheats");
			if (sv_cheats_val != 0)
			{
				printf("[Security] sv_cheats was non-zero (%d), forcing to 0\n", sv_cheats_val);
				game::Cbuf_AddText(0, "set sv_cheats 0\n");
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
			// Instead of removing all clients from the same IP, we only remove the client that disconnected. 
			// This is useful if e.g. server is hosted behind a reverse proxy or load balancer where multiple clients share the same IP.
			game::SV_Live_RemoveClient(client, reason);
			return;
		}
	}

	struct component final : server_component
	{
		void post_unpack() override
		{
			// Sanitize chat messages on server
			g_say_hook.create(0x140299170_g, g_say_stub);

			// Rate limit connections
			sv_direct_connect_hook.create(0x14052EC60_g, sv_direct_connect_stub);

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

			// Cleanup old rate limit entries periodically
			scheduler::loop([]
			{
				cleanup_rate_limits();
			}, scheduler::pipeline::async, 30000ms);
		}
	};
}

REGISTER_COMPONENT(server_patches2::component)
