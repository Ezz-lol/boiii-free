#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class html_frame;

namespace launcher::workshop
{
	void register_callbacks(html_frame* frame);
	void try_refresh_workshop_content();

	std::map<std::string, std::uint64_t> batch_get_time_updated(const std::vector<std::string>& ids);

	struct workshop_item_meta {
		std::uint64_t time_updated = 0;
		std::uint64_t file_size = 0;
		std::string description;
		std::string preview_url;
		int star_rating = 0;
		std::int64_t subs = 0;
		std::int64_t favorites = 0;
	};

	std::map<std::string, workshop_item_meta> batch_get_workshop_meta(const std::vector<std::string>& ids);
}
