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
}
