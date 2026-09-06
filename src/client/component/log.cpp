#ifndef NDEBUG
#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/log.hpp>

namespace logger {

class component final : public generic_component {
public:
  void post_load() override { game::logger::thread::startup(); }
  void pre_destroy() override { game::logger::thread::shutdown(); }
  component_priority priority() const override {
    return component_priority::log;
  }
};
} // namespace logger
REGISTER_COMPONENT(logger::component);
#endif