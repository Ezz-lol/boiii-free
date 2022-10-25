#include <std_include.hpp>
#include "network.hpp"
#include "loader/component_loader.hpp"

#include "scheduler.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

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

	void send_data(const game::netadr_t& address, const void* data, const size_t size)
	{
		game::NET_SendPacket(game::NS_CLIENT1, static_cast<int>(size), data, &address);
	}

	void send_data(const game::netadr_t& address, const std::string& data)
	{
		send_data(address, data.data(), data.size());
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// redirect dw_sendto to raw socket
			//utils::hook::jump(0x14233307E_g, 0x1423330C7_g);

			utils::hook::nop(0x142333056_g, 5); // don't add sock to packet
			utils::hook::set<uint8_t>(0x14233305E_g, 0); // don't add checksum to packet

			utils::hook::nop(0x142332E43_g, 5); // don't read local net id
			utils::hook::set<uint8_t>(0x142332E55_g, 0); // clear local net id
			utils::hook::jump(0x142332E72_g, 0x142332E8E_g); // skip checksum parsing

			utils::hook::set<uint32_t>(0x14134C6E0_g, 4); // set initial connection state to "connecting"

			// Fix netadr types
			//utils::hook::set<uint32_t>(0x142332DE5_g, game::NA_RAWIP); // raw socket type
			//utils::hook::set<uint32_t>(0x142332E00_g, game::NA_IP); // dw socket type

			// intercept command handling
			utils::hook::jump(0x14134D146_g, utils::hook::assemble(handle_command_stub));

			on("getServersResponse", [](const game::netadr_t& /*source*/, const data_view& data)
			{
				OutputDebugStringA(utils::string::va("%.*s", static_cast<int>(data.size()), data.data()));
			});

			// TODO: Fix that
			scheduler::once(create_ip_socket, scheduler::main);

			/*std::thread([]
			{
				while (true)
				{
					MessageBoxA(0, 0, 0, 0);

					game::netadr_t addr{};
					addr.type = game::NA_RAWIP;
					addr.port = 20810;
					*reinterpret_cast<unsigned long*>(&addr.ipv4.a) = inet_addr("116.203.183.23");
					addr.localNetID = game::NS_CLIENT1;

					send(addr, "getservers", utils::string::va("T7 %i full empty", 1));
				}
			}).detach();*/
		}
	};
}

REGISTER_COMPONENT(network::component)
