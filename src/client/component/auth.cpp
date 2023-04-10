#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "auth.hpp"
#include "command.hpp"
#include "network.hpp"
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

namespace auth
{
	namespace
	{
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
			profile_infos::profile_info info{};
			info.version = 4; // invalid
			info.serialize(buffer);
			//profile_infos::get_profile_info().value_or(profile_infos::profile_info{}).serialize(buffer);

			buffer.write_string(data, static_cast<size_t>(length));

			printf("Serialized with size: %llX\n", buffer.get_buffer().size());

			return buffer.move_buffer();
		}

		int send_connect_data_stub(const game::netsrc_t sock, game::netadr_t* adr, const char* data, int len)
		{
			try
			{
				std::string buffer{};

				const auto is_connect_sequence = len >= 7 && strncmp("connect", data, 7) == 0;
				if (is_connect_sequence)
				{
					buffer.append("connect");
					buffer.push_back(' ');
					buffer.append(serialize_connect_data(data, len));

					data = buffer.data();
					len = static_cast<int>(buffer.size());
				}

				return reinterpret_cast<decltype(&send_connect_data_stub)>(0x142173600_g)(sock, adr, data, len);
			}
			catch (std::exception& e)
			{
				printf("Error: %s\n", e.what());
			}

			return 0;
		}

		void handle_connect_packet(const game::netadr_t& target, const network::data_view& data)
		{
			if (!game::is_server_running())
			{
				return;
			}


			printf("Deserialized with size: %llX\n", data.size());

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

			game::foreach_connected_client([&](game::client_s& client)
			{
				if (client.address == target)
				{
					client.xuid = xuid;
				}
			});
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

	struct component final : generic_component
	{
		void post_unpack() override
		{
			// Skip connect handler
			utils::hook::set<uint8_t>(game::select(0x142253EFA, 0x14053714A), 0xEB);
			network::on("connect", handle_connect_packet);

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
