#include <std_include.hpp>

#include "game.hpp"
#include <utils/nt.hpp>

namespace game
{
	size_t get_base()
	{
		static auto base = []
		{
			const utils::nt::library host{};
			if (!host || host == utils::nt::library::get_by_address(get_base))
			{
				throw std::runtime_error("Invalid host application");
			}

			return size_t(host.get_ptr());
		}();
		return base;
	}
}
