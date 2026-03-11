#pragma once

#include <std_include.hpp>

namespace steamcmd
{
	int setup_steamcmd();
	void initialize_download(std::string workshop_id, std::string modtype);
}
