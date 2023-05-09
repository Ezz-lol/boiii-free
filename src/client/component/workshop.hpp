#pragma once

namespace workshop
{
	std::string get_usermap_publisher_id(const std::string& folder_name);
	std::string get_mod_publisher_id();
	std::string get_mod_resized_name();
	bool check_valid_usermap_id(const std::string& mapname, const std::string& pub_id);
	bool check_valid_mod_id(const std::string& pub_id);
	void setup_same_mod_as_host(const std::string& usermap, const std::string& mod);
}
