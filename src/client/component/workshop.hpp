#pragma once

namespace workshop
{
	const std::string get_usermap_publisher_id(const std::string& mapname);
	bool check_valid_publisher_id(const std::string& mapname, const std::string& pub_id);
	void load_usermap_mod_if_needed(const std::string& pub_id);
}
