#pragma once

#include <game/game.hpp>
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

	void add_profile_info(uint64_t user_id, const profile_info& info);
	void add_and_distribute_profile_info(const game::netadr_t& addr, uint64_t user_id, const profile_info& info);
	void clear_profile_infos();

	std::unique_lock<std::recursive_mutex> acquire_profile_lock();

	std::optional<profile_info> get_profile_info();
	std::optional<profile_info> get_profile_info(uint64_t user_id);
	void update_profile_info(const profile_info& info);
}
