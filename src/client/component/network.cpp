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

			handler->second(*address, data);
			return 0;
		}

		void handle_command_stub(utils::hook::assembler& a)
		{
			const auto sv = game::is_server();

			a.pushad64();

			a.mov(rdx, rcx);            // command
			a.mov(r8, sv ? r15 : r12);  // msg
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

			socket_set_blocking(s, false);

			const auto address = htonl(INADDR_ANY);
			const auto port = htons(28960);

			sockaddr_in server_addr{};
			server_addr.sin_family = AF_INET;
			server_addr.sin_addr.s_addr = address;
			server_addr.sin_port = port;

			int retries = 0;
			while (bind(s, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) ==
				SOCKET_ERROR)
			{
				if (++retries > 10) return;
				server_addr.sin_port = htons(ntohs(server_addr.sin_port) + 1);
			}
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

		void con_restricted_execute_buf_stub(int local_client_num, game::ControllerIndex_t controller_index, const char* buffer)
		{
			game::Cbuf_ExecuteBuffer(local_client_num, controller_index, buffer);
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

	uint64_t ret2()
	{
		return 2;
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			utils::hook::nop(game::select(0x142332E76, 0x140596DF6), 4); // don't increment data pointer to optionally skip socket byte
			utils::hook::call(game::select(0x142332E43, 0x140596DC3), read_socket_byte_stub); // optionally read socket byte
			utils::hook::call(game::select(0x142332E81, 0x140596E01), verify_checksum_stub); // skip checksum verification
			utils::hook::set<uint8_t>(game::select(0x14233305E, 0x140596F2E), 0); // don't add checksum to packet

			utils::hook::set<uint32_t>(game::select(0x14134C6E0, 0x14018E574), 5); // set initial connection state to challenging

			// intercept command handling
			utils::hook::call(game::select(0x14134D146, 0x14018EED0), utils::hook::assemble(handle_command_stub));

			utils::hook::set<uint8_t>(game::select(0x14224E90D, 0x1405315F9), 0xEB); // don't kick clients without dw handle

			// Skip DW stuff in NetAdr_ToString
			utils::hook::set<uint8_t>(game::select(0x142173952, 0x140515881), 0xEB);
			// NA_IP -> NA_RAWIP in NetAdr_ToString
			utils::hook::set<uint8_t>(game::select(0x142173934, 0x140515864), game::NA_RAWIP);

			if (game::is_server())
			{
				// Remove restrictions for rcon commands
				utils::hook::call(0x140538D5C_g, con_restricted_execute_buf_stub); // SVC_RemoteCommand
			}

			// TODO: Fix that
			scheduler::once(create_ip_socket, scheduler::main);
		}
	};
}

REGISTER_COMPONENT(network::component)
