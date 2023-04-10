#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/finally.hpp>

#include "network.hpp"

namespace network
{
	namespace
	{
		utils::hook::detour handle_packet_internal_hook{};

		std::unordered_map<std::string, callback>& get_callbacks()
		{
			static std::unordered_map<std::string, callback> callbacks{};
			return callbacks;
		}

		uint64_t handle_command(const game::netadr_t* address, const char* command, const game::msg_t* message)
		{
			const auto cmd_string = utils::string::to_lower(command);
			auto& callbacks = get_callbacks();
			const auto handler = callbacks.find(cmd_string);
			const auto offset = cmd_string.size() + 5;
			if (message->cursize < 0 || static_cast<size_t>(message->cursize) < offset || handler == callbacks.end())
			{
				return 1;
			}

			const std::basic_string_view data(message->data + offset, message->cursize - offset);

			try
			{
				handler->second(*address, data);
			}
			catch (const std::exception& e)
			{
				printf("Error: %s\n", e.what());
			}
			catch (...)
			{
			}

			return 0;
		}

		void handle_command_stub(utils::hook::assembler& a)
		{
			const auto sv = game::is_server();

			a.pushad64();

			a.mov(rdx, rcx); // command
			a.mov(r8, sv ? r15 : r12); // msg
			a.mov(rcx, sv ? r14 : r15); // address

			a.call_aligned(handle_command);

			a.mov(qword_ptr(rsp, 0x78), rax);

			a.popad64();

			a.ret();
		}

		bool socket_set_blocking(const SOCKET s, const bool blocking)
		{
			unsigned long mode = blocking ? 0 : 1;
			return ioctlsocket(s, FIONBIO, &mode) == 0;
		}

		void create_ip_socket()
		{
			auto& s = *game::ip_socket;
			if (s)
			{
				return;
			}

			s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (s == INVALID_SOCKET)
			{
				throw std::runtime_error("Unable to create socket");
			}

			constexpr char broadcast = 1;
			setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

			socket_set_blocking(s, false);

			const auto address = htonl(INADDR_ANY);
			auto port = static_cast<uint16_t>(game::Dvar_FindVar("net_port")->current.value.integer);

			sockaddr_in server_addr{};
			server_addr.sin_family = AF_INET;
			server_addr.sin_addr.s_addr = address;

			int retries = 0;
			do
			{
				server_addr.sin_port = htons(port++);
				if (++retries > 10) return;
			}
			while (bind(s, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) ==
				SOCKET_ERROR);
		}

		bool& socket_byte_missing()
		{
			static thread_local bool was_missing{false};
			return was_missing;
		}

		uint8_t read_socket_byte_stub(game::msg_t* msg)
		{
			auto& byte_missing = socket_byte_missing();
			byte_missing = msg->cursize >= 4 && *reinterpret_cast<int*>(msg->data) == -1;
			if (byte_missing)
			{
				return game::NS_SERVER | (game::NS_SERVER << 4);
			}

			const auto _ = utils::finally([msg]
			{
				++msg->data;
			});

			return game::MSG_ReadByte(msg);
		}

		int verify_checksum_stub(void* /*data*/, const int length)
		{
			return length + (socket_byte_missing() ? 1 : 0);
		}

		void con_restricted_execute_buf_stub(int local_client_num, game::ControllerIndex_t controller_index,
		                                     const char* buffer)
		{
			game::Cbuf_ExecuteBuffer(local_client_num, controller_index, buffer);
		}

		uint64_t handle_packet_internal_stub(const game::ControllerIndex_t controller_index,
		                                     const game::netadr_t from_adr, const game::XUID from_xuid,
		                                     const game::LobbyType lobby_type, const uint64_t dest_module,
		                                     game::msg_t* msg)
		{
			if (from_adr.type != game::NA_LOOPBACK)
			{
				return 0;
			}

			return handle_packet_internal_hook.invoke<bool>(controller_index, from_adr, from_xuid, lobby_type,
			                                                dest_module, msg)
				       ? 1
				       : 0;
		}


		uint64_t ret2()
		{
			return 2;
		}

		int bind_stub(SOCKET /*s*/, const sockaddr* /*addr*/, int /*namelen*/)
		{
			return 0;
		}
	}

	void on(const std::string& command, const callback& callback)
	{
		get_callbacks()[utils::string::to_lower(command)] = callback;
	}

	void send(const game::netadr_t& address, const std::string& command, const std::string& data, const char separator)
	{
		std::string packet = "\xFF\xFF\xFF\xFF";
		packet.append(command);
		packet.push_back(separator);
		packet.append(data);

		send_data(address, packet);
	}

	sockaddr_in convert_to_sockaddr(const game::netadr_t& address)
	{
		sockaddr_in to{};
		to.sin_family = AF_INET;
		to.sin_port = htons(address.port);
		to.sin_addr.S_un.S_addr = address.addr;
		return to;
	}

	void send_data(const game::netadr_t& address, const void* data, const size_t length)
	{
		//game::NET_SendPacket(game::NS_CLIENT1, static_cast<int>(size), data, &address);

		const auto to = convert_to_sockaddr(address);
		sendto(*game::ip_socket, static_cast<const char*>(data), static_cast<int>(length), 0,
		       reinterpret_cast<const sockaddr*>(&to), sizeof(to));
	}

	void send_data(const game::netadr_t& address, const std::string& data)
	{
		send_data(address, data.data(), data.size());
	}

	game::netadr_t address_from_string(const std::string& address)
	{
		game::netadr_t addr{};
		addr.localNetID = game::NS_SERVER;

		if (!game::NET_StringToAdr(address.data(), &addr))
		{
			addr.type = game::NA_BAD;
			return addr;
		}

		if (addr.type == game::NA_IP)
		{
			addr.type = game::NA_RAWIP;
		}

		return addr;
	}

	game::netadr_t address_from_ip(const uint32_t ip, const uint16_t port)
	{
		game::netadr_t addr{};
		addr.localNetID = game::NS_SERVER;
		addr.type = game::NA_RAWIP;
		addr.port = port;
		addr.addr = ip;

		return addr;
	}

	bool are_addresses_equal(const game::netadr_t& a, const game::netadr_t& b)
	{
		if (a.type != b.type)
		{
			return false;
		}

		if (a.type != game::NA_RAWIP && a.type != game::NA_IP)
		{
			return true;
		}

		return a.port == b.port && a.addr == b.addr;
	}

	int net_sendpacket_stub(const game::netsrc_t sock, const int length, const char* data, const game::netadr_t* to)
	{
		//printf("Sending packet of size: %X\n", length);

		if (to->type != game::NA_RAWIP)
		{
			printf("NET_SendPacket: bad address type\n");
			return 0;
		}

		const auto s = *game::ip_socket;
		if (!s || sock > game::NS_MAXCLIENTS)
		{
			return 0;
		}

		sockaddr_in address{};
		address.sin_family = AF_INET;
		address.sin_port = htons(to->port);
		address.sin_addr.s_addr = htonl(((to->ipv4.c | ((to->ipv4.b | (to->ipv4.a << 8)) << 8)) << 8) | to->ipv4.d);

		const auto size = static_cast<size_t>(length);

		std::vector<char> buffer{};
		buffer.resize(size + 1);
		buffer[0] = static_cast<char>((static_cast<uint32_t>(sock) & 0xF) | ((to->localNetID & 0xF) << 4));
		memcpy(buffer.data() + 1, data, size);

		return sendto(s, buffer.data(), static_cast<int>(buffer.size()), 0, reinterpret_cast<sockaddr*>(&address),
		              sizeof(address));
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			utils::hook::nop(game::select(0x1423322B6, 0x140596DF6), 4);
			// don't increment data pointer to optionally skip socket byte
			utils::hook::call(game::select(0x142332283, 0x140596DC3), read_socket_byte_stub);
			// optionally read socket byte
			utils::hook::call(game::select(0x1423322C1, 0x140596E01), verify_checksum_stub);
			// skip checksum verification
			utils::hook::set<uint8_t>(game::select(0x14233249E, 0x140596F2E), 0); // don't add checksum to packet

			// Recreate NET_SendPacket to increase max packet size
			//utils::hook::jump(game::select(0x1423323B0, 0x140596E40), net_sendpacket_stub);

			utils::hook::set<uint32_t>(game::select(0x14134C6E0, 0x14018E574), 5);
			// set initial connection state to challenging

			// intercept command handling
			utils::hook::call(game::select(0x14134D146, 0x14018EED0), utils::hook::assemble(handle_command_stub));

			utils::hook::set<uint8_t>(game::select(0x14224DEAD, 0x1405315F9), 0xEB);
			// don't kick clients without dw handle

			// Skip DW stuff in NetAdr_ToString
			utils::hook::set<uint8_t>(game::select(0x142172EF2, 0x140515881), 0xEB);
			// NA_IP -> NA_RAWIP in NetAdr_ToString
			utils::hook::set<uint8_t>(game::select(0x142172ED4, 0x140515864), game::NA_RAWIP);

			if (game::is_server())
			{
				// Remove restrictions for rcon commands
				utils::hook::call(0x140538D5C_g, &con_restricted_execute_buf_stub); // SVC_RemoteCommand
			}

			// TODO: Fix that
			scheduler::once(create_ip_socket, scheduler::main);

			// Kill lobby system
			handle_packet_internal_hook.create(game::select(0x141EF7FE0, 0x1404A5B90), &handle_packet_internal_stub);

			// Kill voice chat
			utils::hook::set<uint32_t>(game::select(0x141359310, 0x14018FE40), 0xC3C03148);

			// Don't let the game bind sockets anymore
			utils::hook::set(game::select(0x15AAE9344, 0x14B4BD828), bind_stub);

			// Set cl_maxpackets to 100
			utils::hook::set<uint8_t>(game::select(0x1412FF342, 0x140177A32), 100 - 15);
		}
	};
}

REGISTER_COMPONENT(network::component)
