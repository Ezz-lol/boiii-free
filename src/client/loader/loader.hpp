#pragma once
#include <utils/nt.hpp>

namespace loader
{
	utils::nt::library load_binary(const std::string& filename);
}
