#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/memory.hpp>

#include <game/game.hpp>
#include <steam/steam.hpp>

#include <mutex>

namespace command {
namespace {
constexpr const char *compatibility_commands[] = {
    "ffotdversion",     "bbdisable", "bbenable",
    "bitfieldBBPrints", "bbstart",   "setliveevent"};

std::mutex &get_command_map_mutex() {
  static std::mutex mutex;
  return mutex;
}

using command_map = std::unordered_map<std::string, command_param_function>;
using sv_command_map =
    std::unordered_map<std::string, sv_command_param_function>;

command_map &get_command_map() {
  static command_map command_map{};
  return command_map;
}

sv_command_map &get_sv_command_map() {
  static sv_command_map command_map{};
  return command_map;
}

void execute_custom_command() {
  const params params{};
  const std::string command = utils::string::to_lower(params[0]);

  command_map &map = get_command_map();

  if (map.contains(command)) {
    map[command](params);
  }
}

void execute_custom_sv_command() {
  const params_sv params{};
  const std::string command = utils::string::to_lower(params[0]);

  sv_command_map &map = get_sv_command_map();

  if (map.contains(command)) {
    map[command](params);
  }
}

game::CmdArgs *get_cmd_args() { return game::sys::Sys_GetTLS()->cmdArgs; }

void update_whitelist_stub() {
  game::cmd::cmd_function_s *current_function = game::cmd::cmd_functions;
  while (current_function) {
    current_function->autoComplete = 1;
    current_function = current_function->next;
  }
}

void register_client_compatibility_commands() {
  for (const char *command_name : compatibility_commands) {
    add(command_name, [](const params &) {});
  }
}
} // namespace

params::params() : nesting_(get_cmd_args()->nesting) {
  assert(this->nesting_ < game::cmd::CMD_MAX_NESTING);
}

params::params(const std::string &text) : needs_end_(true) {
  game::CmdArgs *cmd_args = get_cmd_args();
  game::cmd::Cmd_TokenizeStringKernel(
      0, game::CONTROLLER_INDEX_FIRST, text.data(),
      512 - cmd_args->totalUsedArgvPool, false, cmd_args);

  this->nesting_ = cmd_args->nesting;
}

params::~params() {
  if (this->needs_end_) {
    game::cmd::Cmd_EndTokenizedString();
  }
}

int params::size() const { return get_cmd_args()->argc[this->nesting_]; }

params_sv::params_sv() : nesting_(game::sv::sv_cmd_args->nesting) {
  assert(this->nesting_ < game::cmd::CMD_MAX_NESTING);
}

params_sv::params_sv(const std::string &text) : needs_end_(true) {
  game::sv::SV_Cmd_TokenizeString(text.data());
  this->nesting_ = game::sv::sv_cmd_args->nesting;
}

params_sv::~params_sv() {
  if (this->needs_end_) {
    game::sv::SV_Cmd_EndTokenizedString();
  }
}

int params_sv::size() const {
  return game::sv::sv_cmd_args->argc[this->nesting_];
}

const char *params_sv::get(const int index) const {
  if (index >= this->size()) {
    return "";
  }

  return game::sv::sv_cmd_args->argv[this->nesting_][index];
}

std::string params_sv::join(const int index) const {
  std::string result;

  for (auto i = index; i < this->size(); ++i) {
    if (i > index)
      result.append(" ");
    result.append(this->get(i));
  }

  return result;
}

const char *params::get(const int index) const {
  if (index >= this->size()) {
    return "";
  }

  return get_cmd_args()->argv[this->nesting_][index];
}

std::string params::join(const int index) const {
  std::string result = {};

  for (auto i = index; i < this->size(); i++) {
    if (i > index)
      result.append(" ");
    result.append(this->get(i));
  }
  return result;
}

void add(const std::string &command, command_function function) {
  add(command, [f = std::move(function)](const params &) { f(); });
}

void add(const std::string &command, command_param_function function) {
  const std::string lower_command = utils::string::to_lower(command);

  bool is_registered;
  {
    std::lock_guard lock(get_command_map_mutex());
    command_map &map = get_command_map();
    is_registered = map.contains(lower_command);
    map[lower_command] = std::move(function);
  }

  if (is_registered) {
    return;
  }

  utils::memory::allocator &allocator = *utils::memory::get_allocator();
  game::cmd::cmd_function_s *cmd_function =
      allocator.allocate<game::cmd::cmd_function_s>();
  const char *cmd_string = allocator.duplicate_string(command);

  game::cmd::Cmd_AddCommandInternal(cmd_string, execute_custom_command,
                                    cmd_function);
  cmd_function->autoComplete = 1;
}

void add_sv(const std::string &command, sv_command_param_function function) {
  const std::string lower_command = utils::string::to_lower(command);

  sv_command_map &map = get_sv_command_map();
  const bool is_registered = map.contains(lower_command);

  map[std::move(lower_command)] = std::move(function);

  if (is_registered) {
    return;
  }

  utils::memory::allocator &allocator = *utils::memory::get_allocator();
  const char *cmd_string = allocator.duplicate_string(command);

  game::cmd::Cmd_AddCommandInternal(
      cmd_string, game::cmd::Stub,
      allocator.allocate<game::cmd::cmd_function_s>());
  game::cmd::Cmd_AddServerCommandInternal(
      cmd_string, execute_custom_sv_command,
      allocator.allocate<game::cmd::cmd_function_s>());
}

std::vector<std::string> get_registered_command_names() {
  std::lock_guard lock(get_command_map_mutex());

  std::vector<std::string> names;
  command_map &map = get_command_map();
  names.reserve(map.size());
  for (const auto &entry : map) {
    names.push_back(entry.first);
  }
  return names;
}

size_t get_registered_command_count() {
  std::lock_guard lock(get_command_map_mutex());
  return get_command_map().size();
}

struct component final : generic_component {
  void post_unpack() override {
    // Disable whitelist
    utils::hook::jump(game::select(0x1420EE860, 0x1404F9CD0),
                      update_whitelist_stub);

    if (!game::is_server()) {
      register_client_compatibility_commands();
    }
  }
};
} // namespace command

REGISTER_COMPONENT(command::component)