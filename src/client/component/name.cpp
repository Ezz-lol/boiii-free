#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "name.hpp"
#include "steam_proxy.hpp"
#include "command.hpp"
#include "toast.hpp"
#include "scheduler.hpp"
#include "game/game.hpp"
#include "game/ui_scripting/execution.hpp"

#include <utils/nt.hpp>
#include <utils/string.hpp>
#include <utils/properties.hpp>
#include <utils/concurrency.hpp>
#include <fstream>

namespace name {
namespace {

utils::concurrency::container<std::string> player_name{};

std::string sanitize_name(const std::string &name) {
  std::string result;
  for (const auto c : name) {
    const auto uc = static_cast<unsigned char>(c);
    if (uc >= 32 && uc <= 126)
      result += c;
  }
  return result;
}

void store_player_name(const std::string &name) {
  utils::properties::store("playerName", name);
}

void activate_player_name(std::string new_name) {
  player_name.access([&](std::string &name) { name = std::move(new_name); });
}

void update_player_name(const std::string &new_name) {
  activate_player_name(new_name);
  store_player_name(new_name);
}

void setup_player_name() {
  std::string initial_name = sanitize_name(steam_proxy::get_player_name());
  if (initial_name.empty())
    initial_name = sanitize_name(utils::nt::get_user_name());
  if (initial_name.empty())
    initial_name = "Unknown Soldier";
  update_player_name(initial_name);
}

void load_player_name() {
  const auto stored_name = utils::properties::load("playerName");
  if (stored_name) {
    auto safe = sanitize_name(*stored_name);
    if (safe.empty())
      safe = "Unknown Soldier";
    activate_player_name(safe);
  } else {
    setup_player_name();
  }
}

std::mutex mtx;
std::unordered_map<int, std::string> name_ov;
std::unordered_map<int, std::string> tag_ov;
std::unordered_map<int, std::string> orig_names;
std::unordered_map<int, std::string> orig_tags;

constexpr size_t CS_NAME = 0x2C;
constexpr size_t CS_TAG = 0x25C;
constexpr size_t NAME_MAX = 32;
constexpr size_t TAG_MAX = 8;
constexpr size_t GCLIENT_NAME = 0x55A0;
constexpr size_t CLIENT_S_NAME = 0x55A0;
constexpr size_t CLIENT_S_TAG = 0x55C0;

std::unordered_set<int> pending_updates;

bool valid(int c) { return c >= 0 && c < 18; }

std::string encode_colors(const std::string &s) {
  std::string result;
  for (size_t i = 0; i < s.size(); ++i) {
    if (i + 1 < s.size() && s[i] == '^' && s[i + 1] >= '0' && s[i + 1] <= '9') {
      result += '`';
      result += s[i + 1];
      ++i;
    } else {
      result += s[i];
    }
  }
  return result;
}

std::string strip_color_codes(const std::string &s) {
  std::string result;
  for (size_t i = 0; i < s.size(); ++i) {
    if (i + 1 < s.size() && s[i] == '^' && s[i + 1] >= '0' && s[i + 1] <= '9') {
      ++i;
    } else {
      result += s[i];
    }
  }
  return result;
}

bool execute_lua(const std::string &code) {
  const auto state = *game::ui::lua::hks::lua_state;
  if (!state) return false;
  try {
    const auto globals = state->globals.v.table;
    const ui_scripting::table lua{globals};
    state->m_global->m_bytecodeSharingMode =
        game::ui::lua::hks::HKS_BYTECODE_SHARING_ON;
    const auto load_results = lua["loadstring"](code, "name_refresh");
    state->m_global->m_bytecodeSharingMode =
        game::ui::lua::hks::HKS_BYTECODE_SHARING_SECURE;
    if (load_results[0].is<ui_scripting::function>()) {
      const auto results = lua["pcall"](load_results);
      return results[0].as<bool>();
    }
  } catch (...) {}
  return false;
}

std::atomic<bool> scoreboard_refresh_installed{false};

void trigger_client_update_internal(int client_num) {
  auto *svs_clients_ptr = *game::sv::svs_clients;
  if (!svs_clients_ptr) return;

  auto &cl = svs_clients_ptr[client_num];
  if (cl.state != game::CA_ACTIVE) return;

  auto *cstate = game::sv::G_GetClientState(client_num);

  if (!cstate) return;

  auto *cs_b = reinterpret_cast<char *>(cstate);
  auto *cl_b = reinterpret_cast<char *>(&cl);

  game::gclient_s *gclient = nullptr;
  auto *entities = game::level::g_entities.get();
  if (entities) {
    auto *ent = reinterpret_cast<game::level::gentity_s *>(
        reinterpret_cast<uint8_t *>(entities) +
        static_cast<size_t>(client_num) * sizeof(game::level::gentity_s));
    gclient = *reinterpret_cast<game::gclient_s **>(reinterpret_cast<char *>(ent) + 0x250);
  }

  std::lock_guard lk(mtx);

  auto ni = name_ov.find(client_num);
  auto ti = tag_ov.find(client_num);
  bool has_name = ni != name_ov.end();
  bool has_tag = ti != tag_ov.end();

  if ((has_name || has_tag) && orig_names.find(client_num) == orig_names.end())
    orig_names[client_num] = std::string(cs_b + CS_NAME);
  if ((has_name || has_tag) && orig_tags.find(client_num) == orig_tags.end())
    orig_tags[client_num] = std::string(cs_b + CS_TAG);

  if (has_name || has_tag) {
    std::string name_str = has_name ? ni->second :
      (orig_names.count(client_num) ? orig_names[client_num] : std::string(cs_b + CS_NAME));

    std::string packed = encode_colors(name_str);
    if (has_tag) {
      packed += "|" + encode_colors(ti->second);
    }
    strncpy_s(cs_b + CS_NAME, NAME_MAX, packed.c_str(), _TRUNCATE);

    auto clean_name = strip_color_codes(name_str);
    if (gclient)
      strncpy_s(reinterpret_cast<char *>(gclient) + GCLIENT_NAME, NAME_MAX,
                clean_name.c_str(), _TRUNCATE);
    if (cl_b)
      strncpy_s(cl_b + CLIENT_S_NAME, NAME_MAX, clean_name.c_str(), _TRUNCATE);

    auto clean_tag = has_tag ? strip_color_codes(ti->second) : "";
    strncpy_s(cs_b + CS_TAG, TAG_MAX, clean_tag.c_str(), _TRUNCATE);
    if (cl_b)
      strncpy_s(cl_b + CLIENT_S_TAG, TAG_MAX, clean_tag.c_str(), _TRUNCATE);
  } else {
    auto orig_n = orig_names.find(client_num);
    if (orig_n != orig_names.end()) {
      strncpy_s(cs_b + CS_NAME, NAME_MAX, orig_n->second.c_str(), _TRUNCATE);
      if (gclient)
        strncpy_s(reinterpret_cast<char *>(gclient) + GCLIENT_NAME, NAME_MAX,
                  orig_n->second.c_str(), _TRUNCATE);
      if (cl_b)
        strncpy_s(cl_b + CLIENT_S_NAME, NAME_MAX, orig_n->second.c_str(),
                  _TRUNCATE);
      orig_names.erase(orig_n);
    }
    auto orig_t = orig_tags.find(client_num);
    if (orig_t != orig_tags.end()) {
      strncpy_s(cs_b + CS_TAG, TAG_MAX, orig_t->second.c_str(), _TRUNCATE);
      if (cl_b)
        strncpy_s(cl_b + CLIENT_S_TAG, TAG_MAX, orig_t->second.c_str(),
                  _TRUNCATE);
      orig_tags.erase(orig_t);
    }
  }
}

void install_scoreboard_refresh() {
  if (scoreboard_refresh_installed.exchange(true))
    return;

  scheduler::loop(
      [] {
        static bool was_in_game = false;
        bool in_game = game::com::Com_IsInGame();
        if (!in_game) {
          was_in_game = false;
          return;
        }
        if (!was_in_game) {
          was_in_game = true;
          std::ifstream file("data/ui_scripts/scoreboard/scoreboard_refresh.lua");
          if (file) {
            std::string lua_code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            execute_lua(lua_code);
          }
        }
      },
      scheduler::main, 1000ms);
}

} // namespace

struct component final : client_component {
  void post_load() override { load_player_name(); }

  void post_unpack() override {
    command::add("name", [](const command::params &params) {
      if (params.size() != 2)
        return;
      update_player_name(params[1]);
      toast::success("Name Changed", params[1]);
    });

    install_scoreboard_refresh();

    scheduler::loop(
        [] {
          if (!game::is_server()) return;
          auto *svs_clients_ptr = *game::sv::svs_clients;
          if (!svs_clients_ptr) return;

          std::vector<int> ready;
          {
            std::lock_guard lk(mtx);
            for (auto it = pending_updates.begin(); it != pending_updates.end();) {
              int cn = *it;
              if (svs_clients_ptr[cn].state == game::CA_ACTIVE) {
                ready.push_back(cn);
                it = pending_updates.erase(it);
              } else {
                ++it;
              }
            }
          }
          for (int cn : ready) {
            trigger_client_update_internal(cn);
          }
        },
        scheduler::server, 500ms);
  }

  component_priority priority() const override {
    return component_priority::name;
  }
};

const char *get_player_name() {
  const auto n = player_name.copy();
  return utils::string::va("%.*s", static_cast<int>(n.size()), n.data());
}

void set_name_override(int client_num, const std::string &n) {
  if (!valid(client_num)) return;
  std::lock_guard lk(mtx);
  name_ov[client_num] = n;
}

void set_tag_override(int client_num, const std::string &t) {
  if (!valid(client_num)) return;
  std::lock_guard lk(mtx);
  tag_ov[client_num] = t;
}

void clear_name_override(int client_num) {
  std::lock_guard lk(mtx);
  name_ov.erase(client_num);
}

void clear_tag_override(int client_num) {
  std::lock_guard lk(mtx);
  tag_ov.erase(client_num);
}

void clear_all_overrides() {
  std::lock_guard lk(mtx);
  name_ov.clear();
  tag_ov.clear();
  orig_names.clear();
  orig_tags.clear();
}

bool has_name_override(int client_num) {
  std::lock_guard lk(mtx);
  return name_ov.find(client_num) != name_ov.end();
}

std::string get_name_override(int client_num) {
  std::lock_guard lk(mtx);
  auto it = name_ov.find(client_num);
  return it != name_ov.end() ? it->second : std::string{};
}

bool has_tag_override(int client_num) {
  std::lock_guard lk(mtx);
  return tag_ov.find(client_num) != tag_ov.end();
}

std::string get_tag_override(int client_num) {
  std::lock_guard lk(mtx);
  auto it = tag_ov.find(client_num);
  return it != tag_ov.end() ? it->second : std::string{};
}

void trigger_client_update(int client_num) {
  if (!valid(client_num) || !game::is_server()) return;

  auto *svs_clients_ptr = *game::sv::svs_clients;
  if (!svs_clients_ptr || svs_clients_ptr[client_num].state != game::CA_ACTIVE) {
    std::lock_guard lk(mtx);
    pending_updates.insert(client_num);
    return;
  }

  trigger_client_update_internal(client_num);
}

} // namespace name

REGISTER_COMPONENT(name::component)
