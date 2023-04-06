#pragma once

#include <utils/byte_buffer.hpp>

namespace profile_infos
{
	struct profile_info
	{
		int32_t version{3};
		std::string ddl{};

		profile_info() = default;
		profile_info(utils::byte_buffer& buffer);
		void serialize(utils::byte_buffer& buffer) const;
	};

	void add_profile_info(uint64_t user_id, profile_info info);

	void distribute_profile_infos();

	std::optional<profile_info> get_profile_info(uint64_t user_id);
	void update_profile_info(const profile_info& info);
}
