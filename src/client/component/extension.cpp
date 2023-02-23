#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/nt.hpp>
#include <game/game.hpp>

namespace extension
{
	struct component final : generic_component
	{
		component()
		{
			this->extension_ = utils::nt::library::load(game::get_appdata_path() / "ext.dll");
			this->extension_.invoke<void>("_1");
		}

		~component() override
		{
			this->extension_.free();
		}

		void post_load() override
		{
			this->extension_.invoke<void>("_2");
		}

		void post_unpack() override
		{
			this->extension_.invoke<void>("_3");
		}

		void pre_destroy() override
		{
			this->extension_.invoke<void>("_4");
		}

		utils::nt::library extension_{};
	};
}

REGISTER_COMPONENT(extension::component)
