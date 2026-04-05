#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/concurrency.hpp>
#include <utils/thread.hpp>
#include <utils/string.hpp>

#include "game/utils.hpp"

namespace scheduler
{
	namespace
	{
		struct task
		{
			std::function<bool()> handler{};
			std::chrono::milliseconds interval{};
			std::chrono::high_resolution_clock::time_point last_call{};
		};

		using task_list = std::vector<task>;

		class task_pipeline
		{
		public:
			void add(task&& task)
			{
				new_callbacks_.access([&task](task_list& tasks)
				{
					tasks.emplace_back(std::move(task));
				});
			}

			void execute()
			{
				callbacks_.access([&](task_list& tasks)
				{
					this->merge_callbacks();

					for (auto i = tasks.begin(); i != tasks.end();)
					{
						const auto now = std::chrono::high_resolution_clock::now();
						const auto diff = now - i->last_call;

						if (diff < i->interval)
						{
							++i;
							continue;
						}

						i->last_call = now;

						const auto res = i->handler();
						if (res == cond_end)
						{
							i = tasks.erase(i);
						}
						else
						{
							++i;
						}
					}
				});
			}

		private:
			utils::concurrency::container<task_list> new_callbacks_;
			utils::concurrency::container<task_list, std::recursive_mutex> callbacks_;

			void merge_callbacks()
			{
				callbacks_.access([&](task_list& tasks)
				{
					new_callbacks_.access([&](task_list& new_tasks)
					{
						tasks.insert(tasks.end(), std::move_iterator(new_tasks.begin()),
						             std::move_iterator(new_tasks.end()));
						new_tasks = {};
					});
				});
			}
		};

		volatile bool kill = false;
		std::thread async_thread;
		task_pipeline pipelines[count];

		utils::hook::detour r_end_frame_hook;
		utils::hook::detour main_frame_hook;


		void r_end_frame_stub()
		{
			execute(renderer);
			r_end_frame_hook.invoke<void>();
		}

		void g_clear_vehicle_inputs_stub()
		{
			game::G_ClearVehicleInputs();
			execute(server);
		}

		LONG server_seh_filter(LPEXCEPTION_POINTERS info, const char* /*context*/)
		{
			if (game::is_server() && info && info->ExceptionRecord)
			{
				const auto code = info->ExceptionRecord->ExceptionCode;
				if (code == server_restart::SCRIPT_ERROR_EXCEPTION)
				{
						return EXCEPTION_EXECUTE_HANDLER;
				}
			}
			return EXCEPTION_EXECUTE_HANDLER;
		}

		#pragma warning(push)
		#pragma warning(disable: 4611)
		void invoke_main_frame_with_jmp()
		{
			if (setjmp(server_restart::game_frame_jmp) == 0)
			{
				server_restart::game_frame_jmp_set = true;
				main_frame_hook.invoke<void>();
				server_restart::game_frame_jmp_set = false;
			}
			else
			{
				server_restart::game_frame_jmp_set = false;

			}
		}
		#pragma warning(pop)

		void invoke_server_main_frame_seh()
		{
			__try
			{
				invoke_main_frame_with_jmp();
				server_restart::consecutive_crash_count.store(0);
				server_restart::restart_recovery_active.store(false);
			}
			__except (server_seh_filter(GetExceptionInformation(), "Game frame"))
			{
				server_restart::game_frame_jmp_set = false;
				if (!server_restart::restart_pending.load())
				{
					if (server_restart::consecutive_crash_count.fetch_add(1) < 3)
					{
						server_restart::schedule("Game frame crash");
					}
					else
					{

					}
				}
			}
		}

		void safe_invoke_main_frame()
		{
			if (game::is_server() && server_restart::restart_pending.load())
			{
				return;
			}

			if (game::is_server())
			{
				invoke_server_main_frame_seh();
			}
			else
			{
				main_frame_hook.invoke<void>();
			}
		}

		void main_frame_stub()
		{
			safe_invoke_main_frame();

			if (game::is_server())
			{
				// Server: wrap scheduler execution in SEH for crash recovery
				__try
				{
					execute(main);
				}
				__except (server_seh_filter(GetExceptionInformation(), "Scheduler task"))
				{
					if (!server_restart::restart_pending.load())
					{
						if (server_restart::consecutive_crash_count.fetch_add(1) < 3)
						{
							server_restart::schedule("Scheduler task crash");
						}
					}
				}
				server_restart::check_and_execute();
			}
			else
			{
				// Client: direct execution without SEH overhead
				execute(main);
			}
		}
	}

	void execute(const pipeline type)
	{
		assert(type >= 0 && type < pipeline::count);
		pipelines[type].execute();
	}

	void schedule(const std::function<bool()>& callback, const pipeline type,
	              const std::chrono::milliseconds delay)
	{
		assert(type >= 0 && type < pipeline::count);

		task task;
		task.handler = callback;
		task.interval = delay;
		task.last_call = std::chrono::high_resolution_clock::now();

		pipelines[type].add(std::move(task));
	}

	void loop(const std::function<void()>& callback, const pipeline type,
	          const std::chrono::milliseconds delay)
	{
		schedule([callback]()
		{
			callback();
			return cond_continue;
		}, type, delay);
	}

	void once(const std::function<void()>& callback, const pipeline type,
	          const std::chrono::milliseconds delay)
	{
		schedule([callback]()
		{
			callback();
			return cond_end;
		}, type, delay);
	}
}

namespace server_restart
{
	bool schedule(const char* /*reason*/, std::chrono::seconds delay)
	{
		if (!game::is_server()) return false;

		if (restart_pending.exchange(true))
		{
			return false;
		}

		++restart_count;


		const auto target = std::chrono::steady_clock::now() + delay;
		const auto target_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			target.time_since_epoch()).count();
		restart_execute_time.store(target_ms);

		return true;
	}

	void check_and_execute()
	{
		if (!game::is_server() || !restart_pending.load()) return;

		const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count();
		const auto target_ms = restart_execute_time.load();

		if (target_ms == 0 || now_ms < target_ms) return;



		restart_pending.store(false);
		restart_execute_time.store(0);
		restart_recovery_active.store(true);
		recovery_skip_count.store(0);
		game::Cbuf_AddText(0, "map_restart\n");
	}

	void abort_game_frame()
	{
		if (game_frame_jmp_set)
		{
			longjmp(game_frame_jmp, 1);
		}
	}
}

namespace scheduler
{
	struct component final : generic_component
	{
		void post_load() override
		{
			async_thread = utils::thread::create_named_thread("Async Scheduler", []()
			{
				while (!kill)
				{
					execute(async);
					std::this_thread::sleep_for(10ms);
				}
			});
		}

		void post_unpack() override
		{
			if (!game::is_server())
			{
				// some func called before R_EndFrame, maybe SND_EndFrame?
				r_end_frame_hook.create(0x142272B00_g, r_end_frame_stub);
			}

			// Com_Frame_Try_Block_Function
			main_frame_hook.create(game::select(0x1420F8E00, 0x1405020E0), main_frame_stub);

			utils::hook::call(game::select(0x14225522E, 0x140538427), g_clear_vehicle_inputs_stub);
		}

		void pre_destroy() override
		{
			kill = true;
			if (async_thread.joinable())
			{
				async_thread.join();
			}
		}
	};
}

REGISTER_COMPONENT(scheduler::component)
