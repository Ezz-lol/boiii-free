#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "command.hpp"
#include "scheduler.hpp"
#include "toast.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

namespace binds {
namespace {
enum class phase { init, loading, ready };
std::atomic<phase> current_phase{phase::init};

game::cmd::xcommand_t original_bind_fn{nullptr};
game::cmd::xcommand_t original_unbind_fn{nullptr};
game::cmd::xcommand_t original_unbindall_fn{nullptr};

std::mutex binds_mutex;
std::map<std::string, std::string> custom_binds;
std::atomic_bool save_scheduled{false};

struct pending_op {
  enum type_t { BIND, UNBIND, UNBINDALL } type;
  std::string key;
  std::string cmd;
};
std::mutex pending_mutex;
std::vector<pending_op> pending_ops;
std::atomic<int> pending_version{0};
constexpr int BATCH_THRESHOLD = 3;

std::string get_binds_path() { return "boiii_players/user/binds.cfg"; }

// Convert raw operator characters to their numpad key names
std::string normalize_key(const std::string &key) {
  if (key.size() == 1) {
    switch (key[0]) {
    case '+':
      return "KP_PLUS";
    case '-':
      return "KP_MINUS";
    case '*':
      return "KP_STAR";
    case '/':
      return "KP_SLASH";
    default:
      break;
    }
  }
  return key;
}

void save_binds() {
  std::lock_guard lock(binds_mutex);
  std::string buffer;
  for (const auto &[key, cmd] : custom_binds) {
    buffer += utils::string::va("bind %s \"%s\"\n", key.c_str(), cmd.c_str());
  }
  utils::io::write_file(get_binds_path(), buffer);
  printf("[Binds] Saved %zu custom bind(s)\n", custom_binds.size());
}

void schedule_save() {
  if (save_scheduled.exchange(true))
    return;
  scheduler::once(
      [] {
        save_scheduled = false;
        save_binds();
      },
      scheduler::main, 3s);
}

void flush_pending() {
  std::vector<pending_op> ops;
  {
    std::lock_guard lock(pending_mutex);
    ops = std::move(pending_ops);
    pending_ops.clear();
  }

  if (static_cast<int>(ops.size()) > BATCH_THRESHOLD) {
    return;
  }

  const bool show_toasts = static_cast<int>(ops.size()) <= BATCH_THRESHOLD;

  bool changed = false;
  std::lock_guard lock(binds_mutex);
  for (const auto &op : ops) {
    switch (op.type) {
    case pending_op::BIND: {
      auto it = custom_binds.find(op.key);
      if (it != custom_binds.end() && it->second != op.cmd) {
        printf("[Binds] Rebind: %s -> %s (was: %s)\n", op.key.c_str(),
               op.cmd.c_str(), it->second.c_str());
        it->second = op.cmd;
        if (show_toasts)
          toast::show("Bind Updated", op.key + " -> " + op.cmd,
                      "t7_icon_options_overlays");
        changed = true;
      } else if (it == custom_binds.end()) {
        custom_binds[op.key] = op.cmd;
        changed = true;
        printf("[Binds] Bind: %s -> %s\n", op.key.c_str(), op.cmd.c_str());
        if (show_toasts)
          toast::show("Bind Saved", op.key + " -> " + op.cmd,
                      "t7_icon_save_overlays");
      }
      break;
    }
    case pending_op::UNBIND:
      if (custom_binds.erase(op.key)) {
        printf("[Binds] Unbind: %s\n", op.key.c_str());
        if (show_toasts)
          toast::info("Bind Removed", op.key);
        changed = true;
      }
      break;
    case pending_op::UNBINDALL:
      if (!custom_binds.empty()) {
        printf("[Binds] Unbindall: cleared %zu custom bind(s)\n",
               custom_binds.size());
        custom_binds.clear();
        changed = true;
      }
      break;
    }
  }

  if (changed)
    schedule_save();
}

void queue_op(pending_op op) {
  {
    std::lock_guard lock(pending_mutex);
    pending_ops.push_back(std::move(op));
  }
  const int ver = ++pending_version;
  scheduler::once(
      [ver] {
        if (ver != pending_version.load())
          return;
        flush_pending();
      },
      scheduler::main, 100ms);
}

game::cmd::cmd_function_s *get_command_list_head() {
  auto *raw =
      static_cast<game::cmd::cmd_function_s *>(game::cmd::cmd_functions);

  auto *as_pointer = *reinterpret_cast<game::cmd::cmd_function_s **>(raw);

  if (as_pointer &&
      !IsBadReadPtr(as_pointer, sizeof(game::cmd::cmd_function_s))) {
    if (as_pointer->name && !IsBadReadPtr(as_pointer->name, 1)) {
      return as_pointer;
    }
  }

  return raw;
}

game::cmd::cmd_function_s *find_command(const char *name) {
  game::cmd::cmd_function_s *cmd = get_command_list_head();
  int safety = 0;
  while (cmd && safety < 2000) {
    safety++;
    if (!IsBadReadPtr(cmd, sizeof(game::cmd::cmd_function_s)) && cmd->name &&
        !IsBadReadPtr(cmd->name, 1) && _stricmp(cmd->name, name) == 0) {
      return cmd;
    }
    cmd = cmd->next;
  }
  return nullptr;
}

void bind_wrapper() {
  if (original_bind_fn)
    original_bind_fn();

  if (current_phase != phase::ready)
    return;

  const command::params params{};
  if (params.size() < 3)
    return;

  const auto key = normalize_key(utils::string::to_lower(params[1]));
  const auto cmd = params.join(2);
  if (key.empty() || cmd.empty())
    return;

  queue_op({pending_op::BIND, key, cmd});
}

void unbind_wrapper() {
  const command::params params{};
  std::string key;
  if (params.size() >= 2)
    key = normalize_key(utils::string::to_lower(params[1]));

  if (original_unbind_fn)
    original_unbind_fn();

  if (current_phase != phase::ready)
    return;
  if (key.empty())
    return;

  queue_op({pending_op::UNBIND, key, ""});
}

void unbindall_wrapper() {
  if (original_unbindall_fn)
    original_unbindall_fn();

  if (current_phase != phase::ready)
    return;

  queue_op({pending_op::UNBINDALL, "", ""});
}

bool patch_command(const char *name, game::cmd::xcommand_t wrapper,
                   game::cmd::xcommand_t *original_out) {
  auto *cmd = find_command(name);
  if (!cmd || !cmd->function) {
    printf("[Binds] WARNING: Could not find engine command '%s'\n", name);
    return false;
  }

  *original_out = cmd->function;
  cmd->function = wrapper;
  return true;
}

void parse_binds_file(const std::string &data) {
  std::lock_guard lock(binds_mutex);
  custom_binds.clear();

  std::istringstream stream(data);
  std::string line;
  while (std::getline(stream, line)) {
    if (line.size() < 7)
      continue;
    if (_strnicmp(line.c_str(), "bind ", 5) != 0)
      continue;

    auto rest = line.substr(5);
    auto space = rest.find(' ');
    if (space == std::string::npos)
      continue;

    auto key = normalize_key(utils::string::to_lower(rest.substr(0, space)));
    auto cmd = rest.substr(space + 1);
    if (cmd.size() >= 2 && cmd.front() == '"' && cmd.back() == '"')
      cmd = cmd.substr(1, cmd.size() - 2);

    if (!key.empty() && !cmd.empty()) {
      custom_binds[key] = cmd;
    }
  }
}

void init_hooks_and_load() {
  patch_command("bind", bind_wrapper, &original_bind_fn);
  patch_command("unbind", unbind_wrapper, &original_unbind_fn);
  patch_command("unbindall", unbindall_wrapper, &original_unbindall_fn);

  if (!original_bind_fn) {
    printf("[Binds] FATAL: Could not hook 'bind' command — custom binds "
           "disabled\n");
    current_phase = phase::ready;
    return;
  }

  std::string data;
  if (!utils::io::read_file(get_binds_path(), &data) || data.empty()) {
    printf("[Binds] No custom binds file found\n");
    current_phase = phase::ready;
    return;
  }

  parse_binds_file(data);
  printf("[Binds] Loading %zu custom bind(s)...\n", custom_binds.size());

  current_phase = phase::loading;
  game::cbuf::Cbuf_AddText(0, data.c_str());
  game::cbuf::Cbuf_AddText(0, "binds_loaded\n");
}
} // namespace

struct component final : client_component {
  void post_unpack() override {
    command::add("binds_loaded", [] {
      current_phase = phase::ready;
      std::lock_guard lock(binds_mutex);
      printf("[Binds] Ready (%zu custom binds active)\n", custom_binds.size());
    });

    // List current custom binds
    command::add("bindlist_custom", [](const command::params &params) {
      std::lock_guard lock(binds_mutex);
      if (custom_binds.empty()) {
        printf("[Binds] No custom binds saved.\n");
        return;
      }
      printf("--- Custom Binds (%zu) ---\n", custom_binds.size());
      for (const auto &[key, cmd] : custom_binds) {
        printf("  %s -> %s\n", key.c_str(), cmd.c_str());
      }
      printf("--- End ---\n");
    });

    scheduler::once(
        [] {
          init_hooks_and_load();
          if (!original_bind_fn) {
            scheduler::once(init_hooks_and_load, scheduler::main, 3s);
          }
        },
        scheduler::pipeline::dvars_loaded);
  }
};
} // namespace binds

REGISTER_COMPONENT(binds::component)
