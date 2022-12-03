#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace dedicated
{
	namespace
	{
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
		}
	};
}

REGISTER_COMPONENT(dedicated::component)
