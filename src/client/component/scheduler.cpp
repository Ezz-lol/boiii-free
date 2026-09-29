#include <std_include.hpp>

#include "scheduler.hpp"
#include <loader/component_loader.hpp>

#include <game/game.hpp>

#include <utils/concurrency.hpp>
#include <utils/hook.hpp>
#include <utils/thread.hpp>

namespace scheduler {
namespace {
struct task {
  std::function<bool()> handler{};
  std::chrono::milliseconds interval{};
  std::chrono::high_resolution_clock::time_point last_call{};
};

using task_list = std::vector<task>;

class task_pipeline {
public:
  void add(task &&task) {
    new_callbacks_.access(
        [&task](task_list &tasks) { tasks.emplace_back(std::move(task)); });
  }

  void execute() {
    callbacks_.access([&](task_list &tasks) {
      this->merge_callbacks();

      for (task_list::iterator i = tasks.begin(); i != tasks.end();) {
        const std::chrono::high_resolution_clock::time_point now =
            std::chrono::high_resolution_clock::now();
        const auto diff = now - i->last_call;

        if (diff < i->interval) {
          ++i;
          continue;
        }

        i->last_call = now;

        const auto res = i->handler();
        if (res == cond_end) {
          i = tasks.erase(i);
        } else {
          ++i;
        }
      }
    });
  }

private:
  utils::concurrency::container<task_list> new_callbacks_;
  utils::concurrency::container<task_list, std::recursive_mutex> callbacks_;

  void merge_callbacks() {
    callbacks_.access([&](task_list &tasks) {
      new_callbacks_.access([&](task_list &new_tasks) {
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

void r_end_frame_stub() {
  execute(renderer);
  r_end_frame_hook.invoke<void>();
}

utils::hook::detour server_frame_hook;
void server_frame_stub() {
  server_frame_hook.invoke();
  execute(server);
}

void main_frame_stub() {
  main_frame_hook.invoke<void>();
  execute(main);
}
} // namespace

void execute(const pipeline type) {
  assert(type >= 0 && type < pipeline::count);
  pipelines[type].execute();
}

void schedule(const std::function<bool()> &callback, const pipeline type,
              const std::chrono::milliseconds delay) {
  assert(type >= 0 && type < pipeline::count);

  task task;
  task.handler = callback;
  task.interval = delay;
  task.last_call = std::chrono::high_resolution_clock::now();

  pipelines[type].add(std::move(task));
}

void loop(const std::function<void()> &callback, const pipeline type,
          const std::chrono::milliseconds delay) {
  schedule(
      [callback]() {
        callback();
        return cond_continue;
      },
      type, delay);
}

void once(const std::function<void()> &callback, const pipeline type,
          const std::chrono::milliseconds delay) {
  schedule(
      [callback]() {
        callback();
        return cond_end;
      },
      type, delay);
}
} // namespace scheduler

namespace scheduler {
struct component final : generic_component {
#ifndef NDEBUG
  std::string name() override { return "scheduler"; }
#endif

  void post_load() override {
    async_thread = utils::thread::create_named_thread("Async Scheduler", []() {
      while (!kill) {
        execute(async);
        std::this_thread::sleep_for(10ms);
      }
    });
  }

  void post_unpack() override {
    if (!game::is_server()) {
      r_end_frame_hook.create(game::snd::SND_EndFrame, r_end_frame_stub);
    }

    main_frame_hook.create(game::com::Com_Frame_Try_Block_Function.get(),
                           main_frame_stub);

    server_frame_hook.create(game::G_ClearVehicleInputs, server_frame_stub);
  }

  void pre_destroy() override {
    kill = true;
    if (async_thread.joinable()) {
      async_thread.join();
    }
  }
};
} // namespace scheduler

REGISTER_COMPONENT(scheduler::component)
