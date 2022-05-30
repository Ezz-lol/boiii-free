#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/thread.hpp>
#include <utils/hook.hpp>

namespace console
{
	namespace
	{
		volatile bool g_started = false;

		void create_game_console()
		{
			reinterpret_cast<void(*)()>(0x142333F80_g)();
		}

		void print_message(const char* message)
		{
			if (g_started)
			{
				reinterpret_cast<void(*)(int, int, const char*, ...)>(0x1421499C0_g)(0, 0, "%s", message);
			}
		}

		void print_stub(const char* fmt, ...)
		{
			va_list ap;
			va_start(ap, fmt);

			char buffer[1024]{0};
			const int res = vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, ap);
			print_message(buffer);

			va_end(ap);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			utils::hook::jump(printf, print_stub);

			this->terminate_runner_ = false;

			this->console_runner_ = utils::thread::create_named_thread("Console IO", [this]
			{
				{
					utils::hook::detour d;
					d.create(0x142333B40_g, utils::hook::assemble([](utils::hook::assembler& a)
					{
						a.mov(r8, "BOIII Console");
						a.mov(r9d, 0x80CA0000);
						a.sub(eax, edx);
						a.jmp(0x142333B4F_g);
					}));
					create_game_console();
					g_started = true;
				}

				MSG msg{};
				while (!this->terminate_runner_)
				{
					if (PeekMessageW(&msg, nullptr, NULL, NULL, PM_REMOVE))
					{
						TranslateMessage(&msg);
						DispatchMessageW(&msg);
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
