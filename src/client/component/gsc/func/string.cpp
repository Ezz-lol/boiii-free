#include <std_include.hpp>

#include "func.hpp"
#include "script_value.hpp"

#include <loader/component_loader.hpp>

#include <utils/string.hpp>

#include <regex>

namespace gsc {
namespace func {
namespace string {
namespace {
constexpr size_t MAX_REGEX_PATTERN = 256;
constexpr size_t MAX_REGEX_INPUT = 4096;

std::string string_arg(scriptInstance_t inst, const uint32_t index) {
  if (index >= Scr_GetNumParam(inst)) {
    return {};
  }
  rapidjson::Document document;
  return script_value::to_string(
      script_value::read(inst, index, document.GetAllocator()));
}

std::optional<std::regex> make_regex(scriptInstance_t inst,
                                     const std::string &pattern,
                                     const std::string &input) {
  if (pattern.size() > MAX_REGEX_PATTERN || input.size() > MAX_REGEX_INPUT) {
    Scr_ParamError(inst, 0, "regex pattern or input is too long");
    return std::nullopt;
  }
  try {
    return std::regex(pattern, std::regex::ECMAScript);
  } catch (const std::regex_error &e) {
    Scr_ParamError(inst, 1, "invalid regex: %s", e.what());
    return std::nullopt;
  }
}

void gscr_strreplace(scriptInstance_t inst) {
  std::string text = string_arg(inst, 0);
  const std::string from = string_arg(inst, 1);
  const std::string to = string_arg(inst, 2);
  if (!from.empty()) {
    text = utils::string::replace(text, from, to);
  }
  push_string(inst, text);
}

void gscr_toupper(scriptInstance_t inst) {
  push_string(inst, utils::string::to_upper(string_arg(inst, 0)));
}

void gscr_tolower(scriptInstance_t inst) {
  push_string(inst, utils::string::to_lower(string_arg(inst, 0)));
}

void gscr_strstartswith(scriptInstance_t inst) {
  push(inst, string_arg(inst, 0).starts_with(string_arg(inst, 1)));
}

void gscr_strendswith(scriptInstance_t inst) {
  push(inst, string_arg(inst, 0).ends_with(string_arg(inst, 1)));
}

void gscr_regexmatch(scriptInstance_t inst) {
  const std::string input = string_arg(inst, 0);
  const std::optional<std::regex> regex =
      make_regex(inst, string_arg(inst, 1), input);
  if (regex) {
    try {
      push(inst, std::regex_match(input, *regex));
    } catch (const std::regex_error &) {
      push(inst, false);
    }
  }
}

void gscr_regexreplace(scriptInstance_t inst) {
  const std::string input = string_arg(inst, 0);
  const std::optional<std::regex> regex =
      make_regex(inst, string_arg(inst, 1), input);
  if (regex) {
    try {
      push_string(inst, std::regex_replace(input, *regex, string_arg(inst, 2)));
    } catch (const std::regex_error &) {
      push_string(inst, input);
    }
  }
}

void gscr_regexsearch(scriptInstance_t inst) {
  const std::string input = string_arg(inst, 0);
  const std::optional<std::regex> regex =
      make_regex(inst, string_arg(inst, 1), input);
  if (!regex) {
    return;
  }
  std::vector<std::string> groups;
  try {
    std::smatch match;
    if (std::regex_search(input, match, *regex)) {
      for (const auto &group : match) {
        groups.emplace_back(group.str());
      }
    }
  } catch (const std::regex_error &) {
    groups.clear();
  }
  push(inst, std::move(groups));
}

void gscr_va(scriptInstance_t inst) {
  const std::string format = string_arg(inst, 0);
  std::string result;
  uint32_t next = 1;
  for (size_t i = 0; i < format.size(); ++i) {
    if (format[i] != '%' || i + 1 >= format.size()) {
      result += format[i];
      continue;
    }
    const char type = format[++i];
    if (type == '%') {
      result += '%';
      continue;
    }
    rapidjson::Document document;
    const rapidjson::Value value =
        script_value::read(inst, next++, document.GetAllocator());
    switch (type) {
    case 'd':
    case 'i':
      result += std::to_string(
          value.IsNumber()
              ? static_cast<int64_t>(value.GetDouble())
              : std::strtoll(script_value::to_string(value).c_str(), nullptr,
                             10));
      break;
    case 'f':
      result += utils::string::va(
          "%f",
          value.IsNumber()
              ? value.GetDouble()
              : std::strtod(script_value::to_string(value).c_str(), nullptr));
      break;
    case 'x':
      result += utils::string::va(
          "%llx",
          static_cast<unsigned long long>(
              value.IsNumber() ? static_cast<int64_t>(value.GetDouble()) : 0));
      break;
    default:
      result += script_value::to_string(value);
      break;
    }
  }
  push_string(inst, result);
}

template <typename... Args>
void register_missing(const char *name, BuiltinFunction func, Args... args) {
  for (const scriptInstance_t inst :
       {scriptInstance_t::SERVER, scriptInstance_t::CLIENT}) {
    if (!builtin_function(inst, name)) {
      register_builtin(inst, name, func, args...);
    }
  }
}
} // namespace

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/string");

  void post_unpack() override {
    register_missing("strreplace", gscr_strreplace, 3u);
    register_missing("toupper", gscr_toupper, 1u);
    register_missing("tolower", gscr_tolower, 1u);
    register_missing("strstartswith", gscr_strstartswith, 2u);
    register_missing("strendswith", gscr_strendswith, 2u);
    register_missing("regexmatch", gscr_regexmatch, 2u);
    register_missing("regexreplace", gscr_regexreplace, 3u);
    register_missing("regexsearch", gscr_regexsearch, 2u);
    register_missing("va", gscr_va, 1u, MAX_BUILTIN_ARGS);
  }
};
} // namespace string
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::string::component)
