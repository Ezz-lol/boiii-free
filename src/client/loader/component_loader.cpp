#include <std_include.hpp>

#include "component_loader.hpp"

#include <utils/nt.hpp>

#include <game/game.hpp>

namespace component_loader {
namespace {
std::vector<std::unique_ptr<generic_component>> &get_components() {
  using component_vector = std::vector<std::unique_ptr<generic_component>>;
  using component_vector_container =
      std::unique_ptr<component_vector,
                      std::function<void(component_vector *)>>;

  static component_vector_container components(
      new component_vector, [](const component_vector *component_vector) {
        pre_destroy();
        delete component_vector;
      });

  return *components;
}

std::vector<std::pair<registration_functor, component_type>> &
get_registration_functors() {
  static std::vector<std::pair<registration_functor, component_type>> functors;
  return functors;
}

void activate_component(std::unique_ptr<generic_component> component) {
  std::vector<std::unique_ptr<generic_component>> &components =
      get_components();
  components.push_back(std::move(component));

  std::ranges::stable_sort(components,
                           [](const std::unique_ptr<generic_component> &a,
                              const std::unique_ptr<generic_component> &b) {
                             return a->priority() > b->priority();
                           });
}
} // namespace

void register_component(registration_functor functor, component_type type) {
  if (!get_components().empty()) {
    throw std::runtime_error("Registration is too late");
  }

  get_registration_functors().emplace_back(std::move(functor), type);
}

bool activate(bool server) {
  static const bool res = [server] {
    try {
      for (std::pair<registration_functor, component_type> &functor :
           get_registration_functors()) {
        if (functor.second == component_type::any ||
            server == (functor.second == component_type::server)) {
          activate_component(functor.first());
        }
      }
    } catch (premature_shutdown_trigger &) {
      return false;
    } catch (const std::exception &e) {
      game::show_error(e.what());
      return false;
    }

    return true;
  }();

  return res;
}

bool post_load() {
  static const bool res = [] {
    try {
#ifndef NDEBUG
      game::trace("[component_loader] Executing component_loader::post_load");
#endif
      for (const std::unique_ptr<generic_component> &component :
           get_components()) {

#ifndef NDEBUG
        game::trace("[component_loader] Executing post_load() in component "
                    "with name: {}",
                    component->name());
#endif
        component->post_load();
#ifndef NDEBUG
        game::trace(
            "Successfully executed post_load in component with name: {}",
            component->name());
#endif
      }
    } catch (premature_shutdown_trigger &) {
      return false;
    } catch (const std::exception &e) {
      game::show_error(e.what());
#ifndef NDEBUG
      game::trace("[component_loader] Failed to execute "
                  "component_loader::post_load. Error: {}",
                  e.what());
#endif
      return false;
    }

#ifndef NDEBUG
    game::trace(
        "[component_loader] Successfully executed component_loader::post_load");
#endif
    return true;
  }();

  return res;
}

void post_unpack() {
  static const bool res = [] {
    try {
#ifndef NDEBUG
      game::trace("[component_loader] Executing component_loader::post_unpack");
#endif
      for (const std::unique_ptr<generic_component> &component :
           get_components()) {
#ifndef NDEBUG
        game::trace("[component_loader] Executing post_unpack() in component "
                    "with name: {}",
                    component->name());
#endif
        component->post_unpack();
#ifndef NDEBUG
        game::trace(
            "Successfully executed post_unpack in component with name: {}",
            component->name());
#endif
      }
    } catch (const std::exception &e) {
      game::show_error(e.what());
#ifndef NDEBUG
      game::trace("[component_loader] Failed to execute "
                  "component_loader::post_unpack. Error: {}",
                  e.what());
#endif
      return false;
    }

#ifndef NDEBUG
    game::trace("[component_loader] Successfully executed "
                "component_loader::post_unpack");
#endif
    return true;
  }();

  if (!res) {
    TerminateProcess(GetCurrentProcess(), 1);
  }
}

void pre_destroy() {
  static const bool res = [] {
    try {
#ifndef NDEBUG
      game::trace("[component_loader] Executing component_loader::pre_destroy");
#endif
      for (const std::unique_ptr<generic_component> &component :
           get_components()) {
#ifndef NDEBUG
        game::trace("[component_loader] Executing "
                    "component_loader::pre_destroy in component with "
                    "name: {}",
                    component->name());
#endif
        component->pre_destroy();
#ifndef NDEBUG
        game::trace(
            "Successfully executed pre_destroy in component with name: {}",
            component->name());
#endif
      }
    } catch (const std::exception &e) {
      game::show_error(e.what());
#ifndef NDEBUG
      game::trace("[component_loader] Failed to execute "
                  "component_loader::pre_destroy. Error: {}",
                  e.what());
#endif
      return false;
    }

    return true;
  }();

  if (!res) {
    TerminateProcess(GetCurrentProcess(), 1);
  }
}

void trigger_premature_shutdown() { throw premature_shutdown_trigger(); }
} // namespace component_loader
