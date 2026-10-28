#include <std_include.hpp>

#include "func.hpp"

#include <loader/component_loader.hpp>

#include <ctime>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace func {
namespace date {
constexpr std::string_view ALLOWED_SPECIFIERS =
    "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%";

bool get_tm(scriptInstance_t inst, uint32_t index, std::tm &out) {
  const uint32_t argc = Scr_GetNumParam(inst);
  const std::time_t time =
      argc > index ? static_cast<std::time_t>(Scr_GetInt(inst, index))
                   : std::time(nullptr);
  const bool utc = argc > index + 1 && Scr_GetInt(inst, index + 1);
  if (time < 0) {
    return false;
  }
  return (utc ? gmtime_s(&out, &time) : localtime_s(&out, &time)) == 0;
}

int32_t iso_week(const std::tm &tm) {
  char buf[4]{};
  std::strftime(buf, sizeof(buf), "%V", &tm);
  return std::atoi(buf);
}

void gscr_getdate(scriptInstance_t inst) {
  const char *field = Scr_GetString(inst, 0);
  std::tm tm{};
  if (!field || !get_tm(inst, 1, tm)) {
    push_int(inst, -1);
    return;
  }

  const std::string name = utils::string::to_lower(field);
  int32_t value = -1;
  if (name == "year")
    value = tm.tm_year + 1900;
  else if (name == "month")
    value = tm.tm_mon + 1;
  else if (name == "day")
    value = tm.tm_mday;
  else if (name == "hour")
    value = tm.tm_hour;
  else if (name == "minute")
    value = tm.tm_min;
  else if (name == "second")
    value = tm.tm_sec;
  else if (name == "weekday")
    value = tm.tm_wday;
  else if (name == "yearday")
    value = tm.tm_yday + 1;
  else if (name == "week")
    value = iso_week(tm);
  push_int(inst, value);
}

void gscr_strftime(scriptInstance_t inst) {
  const char *format = Scr_GetString(inst, 0);
  std::tm tm{};
  if (!format || !get_tm(inst, 1, tm)) {
    push_string(inst, "");
    return;
  }

  std::string safe;
  for (const char *c = format; *c && safe.size() < 256; ++c) {
    if (*c != '%') {
      safe += *c;
      continue;
    }
    const char spec = c[1];
    if (spec && ALLOWED_SPECIFIERS.find(spec) != std::string_view::npos) {
      safe += '%';
      safe += spec;
      ++c;
    } else {
      safe += "%%";
    }
  }

  char buf[1024]{};
  std::strftime(buf, sizeof(buf), safe.c_str(), &tm);
  push_string(inst, buf);
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/date");

  void post_unpack() override {
    register_builtin("getdate", gscr_getdate, 1, 3);
    register_builtin("strftime", gscr_strftime, 1, 3);
  }
};
} // namespace date
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::date::component)
