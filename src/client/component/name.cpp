#include <mutex>
#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "name.hpp"
#include "steam_proxy.hpp"
#include "command.hpp"
#include "toast.hpp"
#include "scheduler.hpp"
#include "game/ui_scripting/execution.hpp"

#include <utils/nt.hpp>
#include <utils/string.hpp>
#include <utils/properties.hpp>
#include <utils/concurrency.hpp>
#include <fstream>

namespace name {

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
  const std::optional<std::string> stored_name =
      utils::properties::load("playerName");
  if (stored_name.has_value()) {
    std::string safe = sanitize_name(*stored_name);
    if (safe.empty())
      safe = "Unknown Soldier";
    activate_player_name(safe);
  } else {
    setup_player_name();
  }
}

std::mutex names_mutex;
game::lobby::PerPlayerOptional<std::string> name_overrides;
game::lobby::PerPlayerOptional<std::string> clan_abbrev_overrides;
game::lobby::PerPlayerOptional<std::string> orig_names;
game::lobby::PerPlayerOptional<std::string> orig_clan_abbrevs;

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
  const game::ui::lua::hks::lua_State *state = *game::ui::lua::hks::lua_state;
  try {
    game::ui::lua::hks::HashTable *globals = state->globals.v.table;
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
  } catch (...) {
  }
  return false;
}

const char *get_player_name() {
  const std::string n = player_name.copy();
  return utils::string::va("%.*s", static_cast<int>(n.size()), n.data());
}

void set_name_override(game::ClientNum_t client_num, const std::string &n) {
  if (sv::valid_client_num(client_num)) {
    name_overrides[client_num] = n;
  }
}

void set_clan_abbrev_override(game::ClientNum_t client_num,
                              const std::string &t) {
  if (!sv::valid_client_num(client_num))
    return;
  clan_abbrev_overrides[client_num] = t;
}

void clear_name_override(game::ClientNum_t client_num) {
  name_overrides[client_num].reset();
}

void clear_clan_abbrev_override(game::ClientNum_t client_num) {
  clan_abbrev_overrides[client_num].reset();
}

template <typename T>
void clear_perplayer_optional_array(game::lobby::PerPlayerOptional<T> &p) {
  for (uint32_t i = game::lobby::MIN_PLAYERS; i < game::lobby::MAX_PLAYERS;
       i++) {
    p[i].reset();
  }
}

void clear_all() {
  std::lock_guard lk(names_mutex);
  clear_perplayer_optional_array(name_overrides);
  clear_perplayer_optional_array(clan_abbrev_overrides);
  clear_perplayer_optional_array(orig_names);
  clear_perplayer_optional_array(orig_clan_abbrevs);
}

void clear_name_slot(game::ClientNum_t slot) {
  std::lock_guard lk(names_mutex);
  name_overrides[slot].reset();
  clan_abbrev_overrides[slot].reset();
  orig_names[slot].reset();
  orig_clan_abbrevs[slot].reset();
}

bool has_name_override(game::ClientNum_t client_num) {
  return name_overrides[client_num].has_value();
}

std::optional<std::string> get_name_override(game::ClientNum_t client_num) {
  return name_overrides[client_num];
}

bool has_clan_abbrev_override(game::ClientNum_t client_num) {
  return clan_abbrev_overrides[client_num].has_value();
}

std::optional<std::string>
get_clan_abbrev_override(game::ClientNum_t client_num) {
  return clan_abbrev_overrides[client_num];
}

bool has_orig_name(game::ClientNum_t client_num) {
  return orig_names[client_num].has_value();
}

bool has_orig_clan_abbrev(game::ClientNum_t client_num) {
  return orig_clan_abbrevs[client_num].has_value();
}

std::optional<std::string> get_orig_name(game::ClientNum_t client_num) {
  return orig_names[client_num];
}

std::optional<std::string> get_orig_clan_abbrev(game::ClientNum_t client_num) {
  return orig_clan_abbrevs[client_num];
}

void remove_orig_name(game::ClientNum_t client_num) {
  orig_names[client_num].reset();
}

void remove_orig_clan_abbrev(game::ClientNum_t client_num) {
  orig_clan_abbrevs[client_num].reset();
}

void client_update(game::sv::client_s *cl) {
  if (game::valid_ptr(cl)) {
    game::level::gentity_t *ent = cl->gentity;
    if (game::valid_ptr(ent)) {
      game::level::gclient_t *gclient = ent->verified_0.client;
      if (game::valid_ptr(gclient)) {
        std::lock_guard lk(names_mutex);
        game::level::clientState_t *client_state = &gclient->sess.cs;
        game::ClientNum_t client_num = client_state->clientIndex;
        if (sv::valid_client_num(client_num)) {

          std::optional<std::string> name_override =
              get_name_override(client_num);
          std::optional<std::string> clan_abbrev_override =
              get_clan_abbrev_override(client_num);

          if ((name_override.has_value() || clan_abbrev_override.has_value())) {
            if (!has_orig_name(client_num)) {
              orig_names[client_num] = std::string(client_state->name);
            }
            if (!has_orig_clan_abbrev(client_num)) {
              orig_clan_abbrevs[client_num] =
                  std::string(client_state->clanAbbrev);
            }

            std::string name_str;
            if (name_override.has_value()) {
              name_str = *name_override;
            } else if (has_orig_name(client_num)) {
              name_str = *get_orig_name(client_num);
            } else {
              name_str = std::string(client_state->name);
            }

            std::string clean_name = strip_color_codes(name_str);
            strncpy_s(client_state->name, game::PLAYER_NAME_MAX_LEN,
                      clean_name.c_str(), _TRUNCATE);
            strncpy_s(cl->name, game::PLAYER_NAME_MAX_LEN, clean_name.c_str(),
                      _TRUNCATE);

            std::string clean_clan_abbrev =
                clan_abbrev_override.has_value()
                    ? strip_color_codes(*clan_abbrev_override)
                    : "";
            strncpy_s(client_state->clanAbbrev,
                      game::PLAYER_CLAN_ABBREV_MAX_LEN,
                      clean_clan_abbrev.c_str(), _TRUNCATE);
            strncpy_s(cl->clanAbbrev, game::PLAYER_CLAN_ABBREV_MAX_LEN,
                      clean_clan_abbrev.c_str(), _TRUNCATE);
          } else {
            std::optional<std::string> orig_name = get_orig_name(client_num);
            if (orig_name.has_value()) {
              strncpy_s(client_state->name, game::PLAYER_NAME_MAX_LEN,
                        orig_name->c_str(), _TRUNCATE);
              strncpy_s(cl->name, game::PLAYER_NAME_MAX_LEN, orig_name->c_str(),
                        _TRUNCATE);
              remove_orig_name(client_num);
            }
            std::optional<std::string> orig_clan_abbrev =
                get_orig_clan_abbrev(client_num);
            if (orig_clan_abbrev.has_value()) {
              strncpy_s(client_state->clanAbbrev,
                        game::PLAYER_CLAN_ABBREV_MAX_LEN,
                        orig_clan_abbrev->c_str(), _TRUNCATE);
              strncpy_s(cl->clanAbbrev, game::PLAYER_CLAN_ABBREV_MAX_LEN,
                        orig_clan_abbrev->c_str(), _TRUNCATE);
              remove_orig_clan_abbrev(client_num);
            }
          }
        }
      }
    }
  }
}

void client_update_post_enterworld(
    game::sv::client_s *cl, [[maybe_unused]] game::user::usercmd_t *cmd) {
  client_update(cl);
}

void trigger_client_update(game::ClientNum_t client_num) {
  if (sv::valid_client_num(client_num)) {
    scheduler::once(
        [client_num]() {
          game::sv::client_s *cl;
          if (game::is_client()) {
            game::sv::client_s_cl *svs_clients = *game::sv::svs_clients_cl;
            cl = &svs_clients[client_num];
          } else {
            game::sv::client_s *svs_clients = *game::sv::svs_clients;
            cl = &svs_clients[client_num];
          }

          client_update(cl);
        },
        scheduler::server, 50ms);
  }
}

void reset_client_name_slot(game::sv::client_s *client,
                            [[maybe_unused]] const char *reason) {
  if (client) {
    game::ClientNum_t client_num = sv::get_client_num(client);
    if (client_num != game::INVALID_CLIENT_INDEX) {
      clear_name_slot(client_num);
    }
  }
}

void initialize() {
  for (game::ClientNum_t i =
           static_cast<game::ClientNum_t>(game::lobby::MIN_PLAYERS);
       i < static_cast<game::ClientNum_t>(game::lobby::MAX_PLAYERS);
       i = static_cast<game::ClientNum_t>(static_cast<int32_t>(i) + 1)) {
    trigger_client_update(i);
  }
}

struct component final : generic_component {
  void post_load() override {
    if (game::is_client()) {
      load_player_name();
    }
  }

  void post_unpack() override {
    if (game::is_client()) {
      command::add("name", [](const command::params &params) {
        if (params.size() != 2)
          return;
        update_player_name(params[1]);
        toast::success("Name Changed", params[1]);
      });
      com::on_level_load([](const char *level) {
        if (strcmp(level, "core_frontend") != 0) {
          std::ifstream file(
              "data/ui_scripts/scoreboard/scoreboard_refresh.lua");
          if (file) {
            std::string lua_code((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
            execute_lua(lua_code);
          }
        }
      });
    }
    sv::on_cliententerworld(client_update_post_enterworld);
    sv::on_removeclient(reset_client_name_slot);

    game_event::on_g_shutdown_game(clear_all);
    game_event::on_g_init_game([] {
      clear_all();
      initialize();
    });
  }

  component_priority priority() const override {
    return component_priority::name;
  }
};

} // namespace name

REGISTER_COMPONENT(name::component)