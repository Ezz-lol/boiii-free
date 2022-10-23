#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

#include "utils/string.hpp"

namespace network
{
	namespace
	{
		using callback = std::function<void(const game::netadr_t&, const std::basic_string_view<uint8_t>&)>;

		std::unordered_map<std::string, callback>& get_callbacks()
		{
			static std::unordered_map<std::string, callback> callbacks{};
			return callbacks;
		}

		bool handle_command(game::netadr_t* address, const char* command, const game::msg_t* message)
		{
			/*const auto cmd_string = utils::string::to_lower(command);
			auto& callbacks = get_callbacks();
			const auto handler = callbacks.find(cmd_string);
			const auto offset = cmd_string.size() + 5;
			if (message->cursize < offset || handler == callbacks.end())
			{
			  return false;
			}
  
			const std::basic_string_view data(message->data + offset, message->cursize - offset);
  
			handler->second(*address, data);
			return true;*/

			OutputDebugStringA(command);

			return false;
		}

		void handle_command_stub(utils::hook::assembler& a)
		{
			a.pushad64();

			a.mov(r8, r12);
			a.mov(rdx, rcx); // command
			a.lea(rcx, qword_ptr(rsp, 0x30 + 0x80)); // address

			a.call_aligned(handle_command);

			a.movzx(rax, al);
			a.mov(qword_ptr(rsp, 0x78), rax);

			a.popad64();

			a.jmp(0x14134D14B_g);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// redirect dw_sendto to raw socket
			utils::hook::jump(0x14233307E_g, 0x1423330C7_g);

			utils::hook::nop(0x142333056_g, 5); // don't add sock to packet
			utils::hook::set<uint8_t>(0x14233305E_g, 0); // don't add checksum to packet

			utils::hook::set<uint8_t>(0x142332E55_g, 0); // clear local net id
			utils::hook::jump(0x142332E72_g, 0x142332E8E_g); // skip checksum parsing

			// intercept command handling
			utils::hook::jump(0x14134D146_g, utils::hook::assemble(handle_command_stub));

			/*std::thread([]
			{
				while (true)
				{
					{
						MessageBoxA(0, 0, 0, 0);

						static auto& ip_socket = *(SOCKET*)0x157E77818_g;
						if (!ip_socket)
						{
							ip_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

							const auto address = htonl(INADDR_ANY);
							const auto port = htons(28960);

							sockaddr_in server_addr{};
							server_addr.sin_family = AF_INET;
							server_addr.sin_addr.s_addr = address;
							server_addr.sin_port = port;

							if (bind(ip_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) ==
								SOCKET_ERROR)
							{
								throw std::runtime_error("Failed to bind socket");
							}
						}

						std::string data = utils::string::va("\xFF\xFF\xFF\xFF" "getservers S1 %i full empty", 1);

						game::netadr_t addr{};
						addr.type = game::NA_RAWIP;
						addr.port = 20810;
						*(unsigned long*)&addr.ipv4.a = inet_addr("116.203.183.23");
						addr.localNetID = game::NS_CLIENT1;
						game::NET_SendPacket(game::NS_CLIENT1, (int)data.size(), data.data(), &addr);
					}
				}
			}).detach();*/
		}

		void pre_destroy() override
		{
			static auto& ip_socket = *(SOCKET*)0x157E77818_g;
			if (ip_socket)
			{
				closesocket(ip_socket);
				ip_socket = 0;
			}
		}
	};
}

REGISTER_COMPONENT(network::component)
