#pragma once

#include <game/game.hpp>

namespace settings {
class flag_setting {
public:
  flag_setting(const char *dvar_name, const char *flag, bool flag_value,
               bool default_value, const char *description);

  [[nodiscard]] bool enabled() const;

  const char *dvar_name;
  const char *flag;
  bool flag_value;
  bool default_value;
  const char *description;
  mutable game::EngineDependentDvarMut dvar{};
};
} // namespace settings
