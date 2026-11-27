#include <std_include.hpp>

#include "scheduler.hpp"

#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/flags.hpp>
#include <utils/string.hpp>

namespace startup_flags {
namespace {
std::vector<std::string> get_command_line_args() {
  const std::string command_line = GetCommandLineA();

  std::vector<std::string> args{};
  std::string current{};
  auto in_quotes = false;
  auto has_arg = false;

  for (const auto character : command_line) {
    if (character == '"') {
      in_quotes = !in_quotes;
      has_arg = true;
    } else if (!in_quotes &&
               std::isspace(static_cast<unsigned char>(character))) {
      if (has_arg) {
        args.emplace_back(std::move(current));
        current.clear();
        has_arg = false;
      }
    } else {
      current.push_back(character);
      has_arg = true;
    }
  }

  if (has_arg) {
    args.emplace_back(std::move(current));
  }

  return args;
}

bool is_flag(const std::string &arg) {
  return arg.size() > 1 && arg[0] == '-' &&
         std::isalpha(static_cast<unsigned char>(arg[1]));
}

void apply_command_line_dvars() {
  const auto args = get_command_line_args();
  std::string buffer{};

  for (size_t i = 0; i < args.size(); ++i) {
    const auto command = utils::string::to_lower(args[i]);
    if ((command != "+set" && command != "+seta") || i + 1 >= args.size()) {
      continue;
    }

    std::string value{};
    auto end = i + 2;
    for (; end < args.size() && !args[end].starts_with('+') &&
           !is_flag(args[end]);
         ++end) {
      if (!value.empty()) {
        value.push_back(' ');
      }
      value.append(args[end]);
    }

    buffer.append(
        std::format("{} {} \"{}\"\n", command.substr(1), args[i + 1], value));
    i = end - 1;
  }

  if (!buffer.empty()) {
    game::cbuf::Cbuf_ExecuteBuffer(game::LOCAL_CLIENT_0,
                                   game::ControllerIndex_t::CONTROLLER_INDEX_0,
                                   buffer.c_str());
  }
}

void apply_startup_flags() {
  apply_command_line_dvars();

  if (utils::flags::has_flag("borderless")) {
    game::r_fullscreen->set(2);
  } else if (utils::flags::has_flag("windowed")) {
    game::r_fullscreen->set(false);
  }

  if (utils::flags::has_flag("safe")) {
    game::r_fullscreen->set(false);
    game::r_vsync->set(false);
  }
}
} // namespace

class component final : public client_component {
  DEFINE_COMPONENT_NAME("startup_flags");

public:
  void post_unpack() override {
    // Schedule after dvars are loaded from config so we can override them
    scheduler::once(apply_startup_flags, scheduler::pipeline::dvars_loaded);
  }
};
} // namespace startup_flags

REGISTER_COMPONENT(startup_flags::component)
