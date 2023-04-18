#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "auth.hpp"
#include "party.hpp"
#include "command.hpp"
#include "network.hpp"
#include "scheduler.hpp"
#include "profile_infos.hpp"

#include <game/game.hpp>
#include <game/utils.hpp>

#include <utils/nt.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/smbios.hpp>
#include <utils/byte_buffer.hpp>
#include <utils/info_string.hpp>
#include <utils/cryptography.hpp>

#include <game/fragment_handler.hpp>

namespace auth
{
	namespace
	{
		std::array<uint64_t, 18> client_xuids{};

		std::string get_hdd_serial()
		{
			DWORD serial{};
			if (!GetVolumeInformationA("C:\\", nullptr, 0, &serial, nullptr, nullptr, nullptr, 0))
			{
				return {};
			}

			return utils::string::va("%08X", serial);
		}

		std::string get_hw_profile_guid()
		{
			HW_PROFILE_INFO info;
			if (!GetCurrentHwProfileA(&info))
			{
				return {};
			}

			return std::string{info.szHwProfileGuid, sizeof(info.szHwProfileGuid)};
		}

		std::string get_protected_data()
		{
			std::string input = "momo5502-boiii-auth";

			DATA_BLOB data_in{}, data_out{};
			data_in.pbData = reinterpret_cast<uint8_t*>(input.data());
			data_in.cbData = static_cast<DWORD>(input.size());
			if (CryptProtectData(&data_in, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_LOCAL_MACHINE,
			                     &data_out) != TRUE)
			{
				return {};
			}

			const auto size = std::min(data_out.cbData, 52ul);
			std::string result{reinterpret_cast<char*>(data_out.pbData), size};
			LocalFree(data_out.pbData);

			return result;
		}

		std::string get_key_entropy()
		{
			std::string entropy{};
			entropy.append(utils::smbios::get_uuid());
			entropy.append(get_hw_profile_guid());
			entropy.append(get_protected_data());
			entropy.append(get_hdd_serial());

			if (entropy.empty())
			{
				entropy.resize(32);
				utils::cryptography::random::get_data(entropy.data(), entropy.size());
			}

			return entropy;
		}

		utils::cryptography::ecc::key& get_key()
		{
			static auto key = utils::cryptography::ecc::generate_key(512, get_key_entropy());
			return key;
		}

		bool is_second_instance()
		{
			static const auto is_first = []
			{
				static utils::nt::handle<> mutex = CreateMutexA(nullptr, FALSE, "boiii_mutex");
				return mutex && GetLastError() != ERROR_ALREADY_EXISTS;
			}();

			return !is_first;
		}

		std::string serialize_connect_data(const char* data, const int length)
		{
			utils::byte_buffer buffer{};
			profile_infos::get_profile_info().value_or(profile_infos::profile_info{}).serialize(buffer);

			buffer.write_string(data, static_cast<size_t>(length));

			return buffer.move_buffer();
		}

		void send_fragmented_connect_packet(const game::netsrc_t sock, game::netadr_t* adr, const char* data,
		                                    const int length)
		{
			const auto connect_data = serialize_connect_data(data, length);
			game::fragment_handler::fragment_data //
			(connect_data.data(), connect_data.size(), [&](const utils::byte_buffer& buffer)
			{
				utils::byte_buffer packet_buffer{};
				packet_buffer.write("connect");
				packet_buffer.write(" ");
				packet_buffer.write(buffer);

				const auto& fragment_packet = packet_buffer.get_buffer();

				game::NET_OutOfBandData(
					sock, adr, fragment_packet.data(),
					static_cast<int>(fragment_packet.size()));
			});
		}

		int send_connect_data_stub(const game::netsrc_t sock, game::netadr_t* adr, const char* data, const int len)
		{
			try
			{
				const auto is_connect_sequence = len >= 7 && strncmp("connect", data, 7) == 0;
				if (!is_connect_sequence)
				{
					return game::NET_OutOfBandData(sock, adr, data, len);
				}

				send_fragmented_connect_packet(sock, adr, data, len);
				return true;
			}
			catch (std::exception& e)
			{
				printf("Error: %s\n", e.what());
			}

			return 0;
		}

		void distribute_player_xuid(const game::netadr_t& target, const size_t player_index, const uint64_t xuid)
		{
			if (player_index >= 18)
			{
				return;
			}

			utils::byte_buffer buffer{};
			buffer.write(static_cast<uint32_t>(player_index));
			buffer.write(xuid);

			game::foreach_connected_client([&](const game::client_s& client, const size_t index)
			{
				if (client.address.type != game::NA_BOT)
				{
					network::send(client.address, "playerXuid", buffer.get_buffer());
				}

				if (index != player_index && target.type != game::NA_BOT)
				{
					utils::byte_buffer current_buffer{};
					current_buffer.write(static_cast<uint32_t>(index));
					current_buffer.write(client.xuid);

					network::send(target, "playerXuid", current_buffer.get_buffer());
				}
			});
		}

		void handle_new_player(const game::netadr_t& target)
		{
			const command::params_sv params{};
			if (params.size() < 2)
			{
				return;
			}

			const utils::info_string info_string(params[1]);
			const auto xuid = strtoull(info_string.get("xuid").data(), nullptr, 16);

			size_t player_index = 18;
			game::foreach_connected_client([&](game::client_s& client, const size_t index)
			{
				if (client.address == target)
				{
					client.xuid = xuid;
					player_index = index;
				}
			});

			distribute_player_xuid(target, player_index, xuid);
		}

		void dispatch_connect_packet(const game::netadr_t& target, const std::string& data)
		{
			utils::byte_buffer buffer(data);
			const profile_infos::profile_info info(buffer);

			const auto connect_data = buffer.read_string();
			const command::params_sv params(connect_data);

			if (params.size() < 2)
			{
				return;
			}

			const auto _ = profile_infos::acquire_profile_lock();

			const utils::info_string info_string(params[1]);
			const auto xuid = strtoull(info_string.get("xuid").data(), nullptr, 16);

			profile_infos::add_and_distribute_profile_info(target, xuid, info);

			game::SV_DirectConnect(target);
			handle_new_player(target);
		}

		void handle_connect_packet_fragment(const game::netadr_t& target, const network::data_view& data)
		{
			if (!game::is_server_running())
			{
				return;
			}

			utils::byte_buffer buffer(data);

			std::string final_packet{};
			if (game::fragment_handler::handle(target, buffer, final_packet))
			{
				scheduler::once([t = target, p = std::move(final_packet)]
				{
					dispatch_connect_packet(t, p);
				}, scheduler::server);
			}
		}

		void handle_player_xuid_packet(const game::netadr_t& target, const network::data_view& data)
		{
			if (game::is_server_running() || !party::is_host(target))
			{
				return;
			}

			utils::byte_buffer buffer(data);

			const auto player_id = buffer.read<uint32_t>();
			const auto xuid = buffer.read<uint64_t>();

			if (player_id < client_xuids.size())
			{
				client_xuids[player_id] = xuid;
			}
		}

		void direct_connect_bots_stub(const game::netadr_t address)
		{
			game::SV_DirectConnect(address);
			handle_new_player(address);
		}
	}

	uint64_t get_guid()
	{
		static const auto guid = []() -> uint64_t
		{
			if (game::is_server() || is_second_instance())
			{
				return 0x110000100000000 | (::utils::cryptography::random::get_integer() & ~0x80000000);
			}

			return get_key().get_hash();
		}();

		return guid;
	}

	uint64_t get_guid(const size_t client_num)
	{
		if (client_num >= 18)
		{
			return 0;
		}

		if (!game::is_server_running())
		{
			return client_xuids[client_num];
		}

		uint64_t xuid = 0;
		const auto callback = [&xuid](const game::client_s& client)
		{
			xuid = client.xuid;
		};

		if (!game::access_connected_client(client_num, callback))
		{
			return 0;
		}

		return xuid;
	}

	void clear_stored_guids()
	{
		for (auto& xuid : client_xuids)
		{
			xuid = 0;
		}
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			// Skip connect handler
			utils::hook::set<uint8_t>(game::select(0x142253EFA, 0x14053714A), 0xEB);
			network::on("connect", handle_connect_packet_fragment);
			network::on("playerXuid", handle_player_xuid_packet);

			// Intercept SV_DirectConnect in SV_AddTestClient
			utils::hook::call(game::select(0x1422490DC, 0x14052E582), direct_connect_bots_stub);

			// Patch steam id bit check
			std::vector<std::pair<size_t, size_t>> patches{};
			const auto p = [&patches](const size_t a, const size_t b)
			{
				patches.emplace_back(a, b);
			};

			if (game::is_server())
			{
				p(0x1404747C6_g, 0x140474806_g);
				p(0x140474A24_g, 0x140474A68_g);
				p(0x140474A85_g, 0x140474AC6_g);
				p(0x140457ED0_g, 0x140457F26_g);
				p(0x140473DD8_g, 0x140473E19_g);
				p(0x1404743D5_g, 0x140474423_g);
				p(0x1404744FD_g, 0x140474553_g);
				p(0x14047462D_g, 0x140474677_g);
				p(0x140475057_g, 0x14047509F_g); // ?
				p(0x140475672_g, 0x1404756B5_g);
				p(0x140477322_g, 0x140477365_g); // ?
			}
			else
			{
				p(0x141E19CED_g, 0x141E19D3B_g);
				p(0x141EB2C76_g, 0x141EB2CB6_g);
				p(0x141EB2DAD_g, 0x141EB2DF2_g);
				p(0x141EB3C35_g, 0x141EB3C76_g);
				p(0x141E19AD0_g, 0x141E19B26_g);
				//
				p(0x141EB0EE8_g, 0x141EB0F29_g);
				p(0x141EB0FA8_g, 0x141EB0FE9_g);
				p(0x141EB2525_g, 0x141EB2573_g);
				p(0x141EB264D_g, 0x141EB26A3_g);
				p(0x141EB277D_g, 0x141EB27C7_g);

				p(0x141EB2AEA_g, 0x141EB2AFA_g);
				p(0x141EB2B01_g, 0x141EB2B33_g);

				p(0x141EB3137_g, 0x141EB3147_g);
				p(0x141EB314E_g, 0x141EB317F_g);

				p(0x141EB5377_g, 0x141EB53BF_g); // ?
				p(0x141EB5992_g, 0x141EB59D5_g);
				p(0x141EB74D2_g, 0x141EB7515_g); // ?

				utils::hook::call(0x14134BF7D_g, send_connect_data_stub);
			}

			for (const auto& patch : patches)
			{
				utils::hook::jump(patch.first, patch.second);
			}
		}
	};
}

REGISTER_COMPONENT(auth::component)
