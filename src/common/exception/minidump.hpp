#pragma once

#include "../utils/nt.hpp"

namespace exception
{
	std::string create_minidump(LPEXCEPTION_POINTERS exceptioninfo);
}
