#pragma once

#include "macros.hpp"
#include <string_view>

enum class component_priority {
  min = 0,
  // must run after the steam_proxy
  name,
  // must run after the updater
  steam_proxy,
  updater,
#ifndef NDEBUG
  // Logger thread startup
  log,
#endif
  // must have the highest priority
  arxan,
};

enum class component_type {
  client,
  server,
  any,
};

struct generic_component {
  static constexpr component_type type = component_type::any;

#ifndef NDEBUG
  virtual const std::string_view &name() {
    static constexpr std::string_view name = "generic";
    return name;
  }
#endif

  virtual ~generic_component() = default;

  virtual void post_load() {}

  virtual void pre_destroy() {}

  virtual void post_unpack() {}

  virtual component_priority priority() const {
    return component_priority::min;
  }
};

struct client_component : generic_component {
  DEFINE_COMPONENT_NAME("generic_client");

  static constexpr component_type type = component_type::client;
};

struct server_component : generic_component {
  DEFINE_COMPONENT_NAME("generic_server");

  static constexpr component_type type = component_type::server;
};
