#include <std_include.hpp>
#include "console.hpp"
#include "loader/component_loader.hpp"

#include <utils/thread.hpp>
#include <utils/hook.hpp>

namespace console
{
        namespace
        {
	  void create_game_console()
	  {
	    reinterpret_cast<void(*)()>(utils::nt::library{}.get_ptr() + 0x2333F80)();
	  }
        }

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			this->terminate_runner_ = false;

			this->console_runner_ = utils::thread::create_named_thread("Console IO", [this]
			{
			    create_game_console();

			    MSG msg{};
			    while (!this->terminate_runner_)
			    {
			      if (PeekMessageA(&msg, nullptr, NULL, NULL, PM_REMOVE))
			      {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			      }
			      else
			      {
				std::this_thread::sleep_for(1ms);
			      }
			    }
			});
		}

		void pre_destroy() override
		{
			this->terminate_runner_ = true;

			if (this->console_runner_.joinable())
			{
				this->console_runner_.join();
			}
		}

	private:
		std::atomic_bool terminate_runner_{false};
		std::thread console_runner_;
	};
}

REGISTER_COMPONENT(console::component)
