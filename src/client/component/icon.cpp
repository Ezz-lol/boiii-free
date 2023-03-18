#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "resource.hpp"

#include <utils/nt.hpp>
#include <utils/hook.hpp>

namespace icon
{
	namespace
	{
		utils::hook::detour load_icon_a_hook;

		HINSTANCE get_current_module()
		{
			return utils::nt::library::get_by_address(get_current_module);
		}

		HICON WINAPI load_icon_a_stub(HINSTANCE module, LPCSTR icon_name)
		{
			static const utils::nt::library game{};
			if (game == module && icon_name == MAKEINTRESOURCEA(1))
			{
				module = get_current_module();
				icon_name = MAKEINTRESOURCEA(ID_ICON);
			}

			return load_icon_a_hook.invoke<HICON>(module, icon_name);
		}
	}

	struct component final : generic_component
	{
		void post_load() override
		{
			load_icon_a_hook.create(LoadIconA, load_icon_a_stub);
		}
	};
}

REGISTER_COMPONENT(icon::component)
