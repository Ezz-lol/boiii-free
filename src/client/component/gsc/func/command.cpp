#include <std_include.hpp>

#include "command.hpp"
#include "func.hpp"

#include <game/impl/scr/scr.hpp>
#include <game/impl/scr/var.hpp>
#include <game/impl/sv/sv.hpp>
#include <game/utils.hpp>

#include <component/command.hpp>
#include <component/game_event.hpp>
#include <component/name.hpp>
#include <component/toast.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace func {

namespace command {
std::string normalize_command_name(std::string value) {
  std::transform(
      value.begin(), value.end(), value.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

std::mutex script_cmd_mutex;
std::vector<std::string> script_cmd_names;
std::deque<std::string> script_cmd_queue;

void script_cmd_handler(const ::command::params &params) {
  std::string result;
  for (int32_t i = 0; i < params.size(); i++) {
    if (i > 0)
      result += ' ';
    result += params.get(i);
  }

  std::scoped_lock lock(script_cmd_mutex);
  script_cmd_queue.push_back(std::move(result));
}

void clear_script_commands() {
  std::scoped_lock lock(script_cmd_mutex);
  script_cmd_queue.clear();
  script_cmd_names.clear();
}

std::string extract_command_name(const std::string &value) {
  const size_t start = value.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return {};
  }

  const size_t end = value.find_first_of(" \t", start);
  const std::string name = value.substr(start, end - start);
  return normalize_command_name(name);
}

void gscr_executecommand(scriptInstance_t inst) {
  const char *cmd = Scr_GetString(inst, 0);
  if (cmd) {
    game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                             utils::string::va("%s\n", cmd));
  }
}

// addcommand("name") - registers a console command that GSC can read via
// getcommand() and you are free to to whatever you with it once you detect
// the command
void gscr_addcommand(scriptInstance_t inst) {
  const char *name = Scr_GetString(inst, 0);
  if (name && name[0]) {
    const std::string cmd_name(name);
    const std::string cmd_key = normalize_command_name(cmd_name);
    {
      std::scoped_lock lock(script_cmd_mutex);
      for (const std::string &existing : script_cmd_names) {
        if (existing == cmd_key)
          return; // Already registered
      }
      script_cmd_names.push_back(cmd_key);
    }

    ::command::add(cmd_name, [](const ::command::params &params) {
      script_cmd_handler(params);
    });
  }
}

// getcommand("name") - returns the next queued command for that name, or ""
// getcommand() - returns the next queued command string regardless of name
void gscr_getcommand(scriptInstance_t inst) {
  std::scoped_lock lock(script_cmd_mutex);
  const uint32_t argc = Scr_GetNumParam(inst);
  if (argc > 0) {
    const char *requested = Scr_GetString(inst, 0);
    const std::string requested_name =
        requested ? normalize_command_name(requested) : std::string{};

    if (!requested_name.empty()) {
      for (auto it = script_cmd_queue.begin(); it != script_cmd_queue.end();
           ++it) {
        if (extract_command_name(*it) == requested_name) {
          const std::string cmd = *it;
          script_cmd_queue.erase(it);
          push_string(inst, cmd.c_str());
          return;
        }
      }

      push_string(inst, "");
      return;
    }
  }

  if (script_cmd_queue.empty()) {
    push_string(inst, "");
    return;
  }

  std::string cmd = std::move(script_cmd_queue.front());
  script_cmd_queue.pop_front();
  push_string(inst, cmd.c_str());
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/command");

  void post_unpack() override {
    // Console commands
    register_builtin(SCRIPTINSTANCE_SERVER, "addcommand", gscr_addcommand, 1,
                     2);
    register_builtin(SCRIPTINSTANCE_SERVER, "getcommand", gscr_getcommand, 0,
                     1);
    register_builtin(SCRIPTINSTANCE_SERVER, "executecommand",
                     gscr_executecommand, 1);

    game_event::on_g_shutdown_game(clear_script_commands);
  }
};
} // namespace command
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::command::component)
