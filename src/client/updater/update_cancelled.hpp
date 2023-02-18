#pragma once

#include <stdexcept>

namespace updater
{
	struct update_cancelled : public std::runtime_error
	{
		update_cancelled()
			: std::runtime_error("Update was cancelled")
		{
		}
	};
}
