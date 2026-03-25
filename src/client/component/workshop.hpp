#pragma once

#include <string>
#include <atomic>
#include <cstdint>
#include <filesystem>

namespace workshop
{
extern std::atomic<bool> downloading_workshop_item;

	extern std::atomic<bool> launcher_downloading;

	bool is_any_download_active();

	// Configurable via dvars (workshop_retry_attempts, workshop_timeout)
	int get_workshop_retry_attempts();

	std::string get_usermap_publisher_id(const std::string& folder_name);
	std::string get_mod_publisher_id();
	std::string get_mod_resized_name();
	bool check_valid_usermap_id(const std::string& mapname, const std::string& pub_id, const std::string& workshop_id, const std::string& base_url = {});
	bool check_valid_mod_id(const std::string& pub_id, const std::string& workshop_id);
	bool mod_switch_requires_fs_reinitialization(const std::string& current_mod, const std::string& new_mod);
	bool mod_load_requires_fs_reinitialization(std::string& mod_name);
	void setup_same_mod_as_host(const std::string& usermap, const std::string& mod);


	void set_pending_mod_reconnect(const std::string& address);
	std::string get_pending_mod_reconnect();

	void set_pending_download_reconnect(const std::string& address);
	std::string get_pending_download_reconnect();

	std::uint64_t compute_folder_size_bytes(const std::filesystem::path& folder);
	std::string human_readable_size(std::uint64_t bytes);

	struct workshop_info
	{
		std::uint64_t file_size = 0;
		std::string title;
	};
	workshop_info get_steam_workshop_info(const std::string& workshop_id);
}
