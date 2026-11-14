#include <std_include.hpp>

#include "scheduler.hpp"
#include <loader/component_loader.hpp>

#include <utils/concurrency.hpp>
#include <utils/hook.hpp>
#include <utils/thread.hpp>

namespace scheduler {

struct task {
  conditionalSchedulerCallback_t handler;
  std::chrono::milliseconds interval{};
  std::chrono::high_resolution_clock::time_point last_call;

  task() = default;
  ~task() = default;
  task(const task &) = delete;
  task &operator=(const task &) = delete;
  task(task &&) noexcept = default;
  task &operator=(task &&) noexcept = default;
};

using task_list = std::vector<task>;

class task_pipeline {
public:
  void add(task &&new_task) {
    new_callbacks_.access([&new_task](task_list &tasks) {
      tasks.emplace_back(std::move(new_task));
    });
  }

  void execute() {
    callbacks_.access([&](task_list &tasks) {
      this->merge_callbacks();

      for (task_list::iterator i = tasks.begin(); i != tasks.end();) {
        const std::chrono::high_resolution_clock::time_point now =
            std::chrono::high_resolution_clock::now();
        const std::chrono::high_resolution_clock::duration diff =
            now - i->last_call;

        if (diff < i->interval) {
          ++i;
        } else {
          i->last_call = now;

          const bool res = i->handler();
          if (res == COND_END) {
            i = tasks.erase(i);
          } else {
            ++i;
          }
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
        new_tasks.clear();
      });
    });
  }
};

static task_pipeline pipelines[+pipeline::count];
static std::atomic_bool kill = false;
static std::thread async_thread;

namespace {
utils::hook::detour r_end_frame_hook;
utils::hook::detour main_frame_hook;

void r_end_frame_stub() {
  execute(pipeline::renderer);
  r_end_frame_hook.invoke<void>();
}

utils::hook::detour server_frame_hook;
void server_frame_stub() {
  server_frame_hook.invoke();
  execute(pipeline::server);
}

void main_frame_stub() {
  main_frame_hook.invoke<void>();
  execute(pipeline::main);
}
} // namespace

void execute(const pipeline type) {
  assert(type >= pipeline::first && type < pipeline::count);
  pipelines[+type].execute();
}

void schedule(const conditionalSchedulerCallback_t &callback, pipeline type,
              std::chrono::milliseconds delay) {
  assert(type >= pipeline::first && type < pipeline::count);

  task new_task{};
  new_task.handler = callback;
  new_task.interval = delay;
  new_task.last_call = std::chrono::high_resolution_clock::now();

  pipelines[+type].add(std::move(new_task));
}

void loop(const schedulerCallback_t &callback, const pipeline type,
          const std::chrono::milliseconds delay) {
  schedule(
      [callback]() {
        callback();
        return COND_CONTINUE;
      },
      type, delay);
}

void once(const schedulerCallback_t &callback, const pipeline type,
          const std::chrono::milliseconds delay) {
  schedule(
      [callback]() {
        callback();
        return COND_END;
      },
      type, delay);
}
} // namespace scheduler

namespace scheduler {
struct component final : generic_component {
  DEFINE_COMPONENT_NAME("scheduler");

  void post_load() override {
    async_thread = utils::thread::create_named_thread("Async Scheduler", []() {
      while (!kill.load(std::memory_order_acquire)) {
        execute(pipeline::async);
        std::this_thread::sleep_for(10ms);
      }
    });
  }

  void post_unpack() override {
    if (game::is_client()) {
      r_end_frame_hook.create(game::snd::SND_EndFrame, r_end_frame_stub);
    }

    main_frame_hook.create(game::com::Com_Frame_Try_Block_Function.get(),
                           main_frame_stub);

    server_frame_hook.create(game::G_ClearVehicleInputs, server_frame_stub);
  }

  void pre_destroy() override {
    kill.store(true, std::memory_order_release);
    if (async_thread.joinable()) {
      async_thread.join();
    }
  }
};
} // namespace scheduler

REGISTER_COMPONENT(scheduler::component)
