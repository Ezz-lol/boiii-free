#pragma once

namespace profile_infos
{
	struct profile_info
	{
		int32_t version;
		std::string ddl;
	};

	std::optional<profile_info> get_profile_info(uint64_t user_id);
	void update_profile_info(const profile_info& info);
}
