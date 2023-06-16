#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "updater.hpp"
#include "game/game.hpp"

#include <utils/flags.hpp>
#include <updater/updater.hpp>

namespace updater
{
	void update()
	{
		if (utils::flags::has_flag("noupdate"))
		{
			return;
		}

		try
		{
			run(game::get_appdata_path());
		}
		catch (update_cancelled&)
		{
			TerminateProcess(GetCurrentProcess(), 0);
		}
		catch (...)
		{
		}
	}

	class component final : public generic_component
	{
	public:
		component()
		{
			this->update_thread_ = std::thread([this]
			{
				update();
			});
		}

		void pre_destroy() override
		{
			join();
		}

		void post_unpack() override
		{
			join();
		}

		component_priority priority() const override
		{
			return component_priority::updater;
		}

	private:
		std::thread update_thread_{};

		void join()
		{
			if (this->update_thread_.joinable())
			{
				this->update_thread_.join();
			}
		}
	};
}

REGISTER_COMPONENT(updater::component)
