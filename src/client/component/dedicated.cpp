#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace dedicated
{
	namespace
	{
	}

	struct component final : server_component
	{
		void post_unpack() override
		{
		}
	};
}

REGISTER_COMPONENT(dedicated::component)
