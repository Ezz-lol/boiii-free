#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "profile_infos.hpp"
#include "network.hpp"
#include "party.hpp"

#include <utils/nt.hpp>
#include <utils/properties.hpp>
#include <utils/concurrency.hpp>

#include "../steam/steam.hpp"
#include <utils/io.hpp>

namespace profile_infos
{
	namespace
	{
		using profile_map = std::unordered_map<uint64_t, profile_info>;
		utils::concurrency::container<profile_map> profile_mapping;

		std::optional<profile_info> load_profile_info()
		{
			std::string data{};
			if (!utils::io::read_file("players/user/profile_info", &data))
			{
				return {};
			}

			profile_info info{};
			constexpr auto version_size = sizeof(info.version);

			if (data.size() < sizeof(version_size))
			{
				return {};
			}

			memcpy(&info.version, data.data(), version_size);
			info.ddl.assign(data.begin() + version_size, data.end());

			return {std::move(info)};
		}
	}

	profile_info::profile_info(utils::byte_buffer& buffer)
	{
		this->version = buffer.read<int32_t>();
		this->ddl = buffer.read_string();
	}

	void profile_info::serialize(utils::byte_buffer& buffer) const
	{
		buffer.write(this->version);
		buffer.write_string(this->ddl);
	}

	void add_profile_info(const uint64_t user_id, profile_info info)
	{
		if (user_id == steam::SteamUser()->GetSteamID().bits)
		{
			return;
		}

		profile_mapping.access([&](profile_map& profiles)
		{
			profiles[user_id] = std::move(info);
		});
	}

	void distribute_profile_infos()
	{
		// TODO
	}

	std::optional<profile_info> get_profile_info(uint64_t user_id)
	{
		if (user_id == steam::SteamUser()->GetSteamID().bits)
		{
			return load_profile_info();
		}

		return profile_mapping.access<std::optional<profile_info>>([user_id](const profile_map& profiles)
		{
			std::optional<profile_info> result{};

			const auto profile_entry = profiles.find(user_id);
			if (profile_entry != profiles.end())
			{
				result = profile_entry->second;
			}

			return result;
		});
	}

	void update_profile_info(const profile_info& info)
	{
		std::string data{};
		data.reserve(4 + info.ddl.size());

		data.append(reinterpret_cast<const char*>(&info.version), sizeof(info.version));
		data.append(info.ddl);

		utils::io::write_file("players/user/profile_info", data);
	}

	struct component final : client_component
	{
		void post_unpack() override
		{
			network::on("profileInfo", [](const game::netadr_t& server, const network::data_view& data)
			{
				if (party::get_connected_server() != server)
				{
					return;
				}

				utils::byte_buffer buffer(data);
				const auto user_id = buffer.read<uint64_t>();
				const profile_info info(buffer);

				add_profile_info(user_id, info);
			});
		}
	};
}

REGISTER_COMPONENT(profile_infos::component)
