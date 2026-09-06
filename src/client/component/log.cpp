#ifndef NDEBUG
#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/log.hpp>

namespace log {

class component final : public generic_component {
public:
  void post_load() override { game::log::thread::startup(); }
  void pre_destroy() override { game::log::thread::shutdown(); }
  component_priority priority() const override {
    return component_priority::log;
  }
};
} // namespace log
REGISTER_COMPONENT(log::component);
#endif