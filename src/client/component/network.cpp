#include <std_include.hpp>
#include "network.hpp"
#include "loader/component_loader.hpp"

#include "scheduler.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/finally.hpp>

namespace network
{
	namespace
	{
		std::unordered_map<std::string, callback>& get_callbacks()
		{
			static std::unordered_map<std::string, callback> callbacks{};
			return callbacks;
		}

		bool handle_command(const game::netadr_t* address, const char* command, const game::msg_t* message)
		{
			const auto cmd_string = utils::string::to_lower(command);
			auto& callbacks = get_callbacks();
			const auto handler = callbacks.find(cmd_string);
			const auto offset = cmd_string.size() + 5;
			if (message->cursize < 0 || static_cast<size_t>(message->cursize) < offset || handler == callbacks.end())
			{
				return false;
			}

			const std::basic_string_view data(message->data + offset, message->cursize - offset);

			handler->second(*address, data);
			return true;
		}

		void handle_command_stub(utils::hook::assembler& a)
		{
			a.pushad64();

			a.mov(r8, r12); // msg
			a.mov(rdx, rcx); // command
			a.mov(rcx, r15); // address

			a.call_aligned(handle_command);

			a.movzx(rax, al);
			a.mov(qword_ptr(rsp, 0x78), rax);

			a.popad64();

			a.jmp(0x14134D14B_g);
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

	void send_data(const game::netadr_t& address, const void* data, const size_t size)
	{
		//game::NET_SendPacket(game::NS_CLIENT1, static_cast<int>(size), data, &address);

		const auto to = convert_to_sockaddr(address);
		sendto(*game::ip_socket, static_cast<const char*>(data), static_cast<int>(size), 0,
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

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			//utils::hook::jump(0x14143CAB0_g, ret2); // patch dwGetConnectionStatus
			//utils::hook::jump(0x14233307E_g, 0x1423330C7_g);

			utils::hook::nop(0x142332E76_g, 4); // don't increment data pointer to optionally skip socket byte
			utils::hook::call(0x142332E43_g, read_socket_byte_stub); // optionally read socket byte
			utils::hook::call(0x142332E81_g, verify_checksum_stub); // skip checksum verification
			utils::hook::set<uint8_t>(0x14233305E_g, 0); // don't add checksum to packet

			utils::hook::set<uint32_t>(0x14134C6E0_g, 5); // set initial connection state to challenging

			// intercept command handling
			utils::hook::jump(0x14134D146_g, utils::hook::assemble(handle_command_stub));

			utils::hook::set<uint8_t>(0x14224E90D_g, 0xEB); // don't kick clients without dw handle

			// TODO: Fix that
			scheduler::once(create_ip_socket, scheduler::main);
		}
	};
}

REGISTER_COMPONENT(network::component)
