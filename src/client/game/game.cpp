#include <std_include.hpp>

#include "game.hpp"
#include <utils/nt.hpp>

namespace game
{
	namespace
	{
		const utils::nt::library& get_host_library()
		{
			static auto host_library = []
			{
				utils::nt::library host{};
				if (!host || host == utils::nt::library::get_by_address(get_base))
				{
					throw std::runtime_error("Invalid host application");
				}

				return host;
			}();

			return host_library;
		}
	}

	size_t get_base()
	{
		static auto base = reinterpret_cast<size_t>(get_host_library().get_ptr());
		return base;
	}

	bool is_server()
	{
		static auto server = get_host_library().get_optional_header()->CheckSum == 0x14C28B4;
		return server;
	}
}
