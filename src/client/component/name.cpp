#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include "name.hpp"
#include "steam_proxy.hpp"
#include "command.hpp"
#include "network.hpp"
#include "party.hpp"
#include "toast.hpp"
#include "scheduler.hpp"
#include <game/utils.hpp>

#include <utils/byte_buffer.hpp>
#include <utils/nt.hpp>
#include <utils/string.hpp>
#include <utils/properties.hpp>
#include <utils/concurrency.hpp>
#include <str.hpp>

#include "sv.hpp"
#include "game_event.hpp"

#include <mutex>
#include <optional>

namespace name {
namespace {
constexpr const str<13> sync_packet_name = "nameoverride";

enum class SyncMessageType : uint8_t {
  CLEAR_ALL = 0x0,
  SET_NAME = 0x1,
  SET_TAG = 0x2,
  CLEAR_NAME = 0x3,
  CLEAR_TAG = 0x4,
  SNAPSHOT = 0x5,
};

bool is_syncable_address(const game::net::netadr_t &address) {
  return address.type != game::net::NA_BAD && address.type != game::net::NA_BOT;
}

bool is_trusted_sync_sender(const game::net::netadr_t &address) {
  return address.type == game::net::NA_LOOPBACK || party::is_host(address);
}

bool is_syncable_client(const game::sv::client_s &client) {
  return client.state != game::net::CS_FREE &&
         client.state != game::net::CS_ZOMBIE &&
         is_syncable_address(client.address);
}

void send_override_packet(
    const game::net::netadr_t &target, const SyncMessageType type,
    const game::ClientNum_t client_num,
    const std::optional<std::string> &value = std::nullopt) {
  if (!game::server_running() || !is_syncable_address(target)) {
    return;
  }

  utils::byte_buffer buffer{};
  buffer.write(static_cast<uint8_t>(type));
  buffer.write(static_cast<int32_t>(client_num));

  if (value.has_value()) {
    buffer.write_string(*value);
  }

  network::send(target, sync_packet_name, buffer.get_buffer());
}

void broadcast_override_packet(
    const SyncMessageType type, const game::ClientNum_t client_num,
    const std::optional<std::string> &value = std::nullopt) {
  if (!game::server_running()) {
    return;
  }

  game::foreach_connected_client([&](game::sv::client_s &client, size_t) {
    if (!is_syncable_client(client)) {
      return;
    }

    send_override_packet(client.address, type, client_num, value);
  });
}
} // namespace

utils::concurrency::container<std::string> player_name{};

std::string sanitize_name(const std::string &name) {
  std::string result;
  for (const char c : name) {
    const unsigned char uc = static_cast<unsigned char>(c);
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

std::recursive_mutex names_mutex;
game::lobby::LobbyClientOptionalPool<std::string> name_overrides;
game::lobby::LobbyClientOptionalPool<std::string> clan_abbrev_overrides;
game::lobby::LobbyClientOptionalPool<std::string> orig_names;
game::lobby::LobbyClientOptionalPool<std::string> orig_clan_abbrevs;

void collect_current_overrides(
    std::vector<std::pair<game::ClientNum_t, std::string>> &names,
    std::vector<std::pair<game::ClientNum_t, std::string>> &tags) {
  std::lock_guard lk(names_mutex);

  for (game::ClientNum_t client_num = game::CLIENT_INDEX_0;
       client_num < game::CLIENT_INDEX_COUNT; client_num++) {
    if (name_overrides[client_num].has_value()) {
      names.emplace_back(client_num, *name_overrides[client_num]);
    }

    if (clan_abbrev_overrides[client_num].has_value()) {
      tags.emplace_back(client_num, *clan_abbrev_overrides[client_num]);
    }
  }
}

void send_snapshot_packet(const game::net::netadr_t &target) {
  if (!game::server_running() || !is_syncable_address(target)) {
    return;
  }

  std::vector<std::pair<game::ClientNum_t, std::string>> names{};
  std::vector<std::pair<game::ClientNum_t, std::string>> tags{};
  collect_current_overrides(names, tags);

  utils::byte_buffer buffer{};
  buffer.write(static_cast<uint8_t>(SyncMessageType::SNAPSHOT));
  buffer.write(static_cast<int32_t>(game::INVALID_CLIENT_INDEX));
  buffer.write(static_cast<uint8_t>(names.size()));

  for (const auto &[client_num, value] : names) {
    buffer.write(static_cast<int32_t>(client_num));
    buffer.write_string(value);
  }

  buffer.write(static_cast<uint8_t>(tags.size()));

  for (const auto &[client_num, value] : tags) {
    buffer.write(static_cast<int32_t>(client_num));
    buffer.write_string(value);
  }

  network::send(target, sync_packet_name, buffer.get_buffer());
}

void send_snapshot_packet_to_client(const game::ClientNum_t target_client) {
  if (!game::valid_client_num(target_client) || !game::server_running()) {
    return;
  }

  game::access_connected_client(static_cast<size_t>(target_client),
                                [&](game::sv::client_s &client) {
                                  if (!is_syncable_client(client)) {
                                    return;
                                  }

                                  send_snapshot_packet(client.address);
                                });
}

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

const char *get_player_name() {
  const std::string n = player_name.copy();
  return utils::string::va("%.*s", static_cast<int>(n.size()), n.data());
}

void set_name_override(game::ClientNum_t client_num, const std::string &n) {
  std::lock_guard lk(names_mutex);
  if (game::valid_client_num(client_num)) {
    name_overrides[client_num] = n;
  }
}

void set_clan_abbrev_override(game::ClientNum_t client_num,
                              const std::string &t) {
  std::lock_guard lk(names_mutex);
  if (!game::valid_client_num(client_num))
    return;
  clan_abbrev_overrides[client_num] = t;
}

void clear_name_override(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  name_overrides[client_num].reset();
}

void clear_clan_abbrev_override(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  clan_abbrev_overrides[client_num].reset();
}

template <typename T>
void clear_lobby_client_optional_pool(
    game::lobby::LobbyClientOptionalPool<T> &p) {
  for (game::ClientNum_t i = game::lobby::MIN_PLAYERS;
       i < game::lobby::MAX_PLAYERS; i++) {
    p[i].reset();
  }
}

void clear_all() {
  std::lock_guard lk(names_mutex);
  clear_lobby_client_optional_pool(name_overrides);
  clear_lobby_client_optional_pool(clan_abbrev_overrides);
  clear_lobby_client_optional_pool(orig_names);
  clear_lobby_client_optional_pool(orig_clan_abbrevs);
}

void clear_name_slot(game::ClientNum_t slot) {
  std::lock_guard lk(names_mutex);
  name_overrides[slot].reset();
  clan_abbrev_overrides[slot].reset();
  orig_names[slot].reset();
  orig_clan_abbrevs[slot].reset();
}

bool has_name_override(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  return name_overrides[client_num].has_value();
}

std::optional<std::string> get_name_override(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  return name_overrides[client_num];
}

bool has_clan_abbrev_override(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  return clan_abbrev_overrides[client_num].has_value();
}

std::optional<std::string>
get_clan_abbrev_override(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  return clan_abbrev_overrides[client_num];
}

void sync_name_override_to_clients(game::ClientNum_t client_num) {
  const std::optional<std::string> value = get_name_override(client_num);
  if (!value.has_value()) {
    sync_name_reset_to_clients(client_num);
    return;
  }

  broadcast_override_packet(SyncMessageType::SET_NAME, client_num, *value);
}

void sync_clan_abbrev_override_to_clients(game::ClientNum_t client_num) {
  const std::optional<std::string> value = get_clan_abbrev_override(client_num);
  if (!value.has_value()) {
    sync_clan_abbrev_reset_to_clients(client_num);
    return;
  }

  broadcast_override_packet(SyncMessageType::SET_TAG, client_num, *value);
}

void sync_name_reset_to_clients(game::ClientNum_t client_num) {
  broadcast_override_packet(SyncMessageType::CLEAR_NAME, client_num);
}

void sync_clan_abbrev_reset_to_clients(game::ClientNum_t client_num) {
  broadcast_override_packet(SyncMessageType::CLEAR_TAG, client_num);
}

void sync_all_overrides_to_client(game::ClientNum_t target_client) {
  if (!game::valid_client_num(target_client)) {
    return;
  }

  send_snapshot_packet_to_client(target_client);
}

bool has_orig_name(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  return orig_names[client_num].has_value();
}

bool has_orig_clan_abbrev(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  return orig_clan_abbrevs[client_num].has_value();
}

std::optional<std::string> get_orig_name(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  return orig_names[client_num];
}

std::optional<std::string> get_orig_clan_abbrev(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  return orig_clan_abbrevs[client_num];
}

void remove_orig_name(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  orig_names[client_num].reset();
}

void remove_orig_clan_abbrev(game::ClientNum_t client_num) {
  std::lock_guard lk(names_mutex);
  orig_clan_abbrevs[client_num].reset();
}

void client_update_internal(game::sv::client_s *cl,
                            std::optional<game::ClientNum_t> requested_client) {
  if (!game::valid_engine_ptr(cl)) {
    return;
  }

  game::level::gentity_t *ent = cl->gentity;
  if (!game::valid_engine_ptr(ent)) {
    return;
  }

  game::level::gclient_t *gclient = ent->client;
  if (!game::valid_engine_ptr(gclient)) {
    return;
  }

  std::lock_guard lk(names_mutex);
  game::level::clientState_t *client_state = &gclient->sess.cs;
  const game::ClientNum_t slot_client_num = sv::get_client_num(cl);
  const game::ClientNum_t cs_client_num = client_state->clientIndex;
  game::ClientNum_t client_num =
      requested_client.has_value() ? *requested_client : cs_client_num;

  if (!game::valid_client_num(client_num) &&
      game::valid_client_num(cs_client_num)) {
    client_num = cs_client_num;
  }

  if (!game::valid_client_num(client_num) &&
      game::valid_client_num(slot_client_num)) {
    client_num = slot_client_num;
  }

  if (!game::valid_client_num(client_num)) {
    return;
  }

  std::optional<std::string> name_override = get_name_override(client_num);
  std::optional<std::string> clan_abbrev_override =
      get_clan_abbrev_override(client_num);

  if (name_override.has_value() || clan_abbrev_override.has_value()) {
    if (!has_orig_name(client_num)) {
      orig_names[client_num] = std::string(client_state->name);
    }
    if (!has_orig_clan_abbrev(client_num)) {
      orig_clan_abbrevs[client_num] = std::string(client_state->clanAbbrev);
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
    strscpy(client_state->name, clean_name.c_str());
    strscpy(cl->name, clean_name.c_str());

    std::string clean_clan_abbrev =
        clan_abbrev_override.has_value()
            ? strip_color_codes(*clan_abbrev_override)
            : "";
    const bool clan_changed =
        std::strcmp(client_state->clanAbbrev, clean_clan_abbrev.c_str()) != 0;
    strscpy(client_state->clanAbbrev, clean_clan_abbrev.c_str());
    strscpy(cl->clanAbbrev, clean_clan_abbrev.c_str());
    client_state->clanAbbrevEV = clan_changed;
  } else {
    std::optional<std::string> orig_name = get_orig_name(client_num);
    if (orig_name.has_value()) {
      strscpy(client_state->name, orig_name->c_str());
      strscpy(cl->name, orig_name->c_str());
      remove_orig_name(client_num);
    }

    std::optional<std::string> orig_clan_abbrev =
        get_orig_clan_abbrev(client_num);
    if (orig_clan_abbrev.has_value()) {
      const bool clan_changed =
          std::strcmp(client_state->clanAbbrev, orig_clan_abbrev->c_str()) != 0;
      strscpy(client_state->clanAbbrev, orig_clan_abbrev->c_str());
      strscpy(cl->clanAbbrev, orig_clan_abbrev->c_str());
      client_state->clanAbbrevEV = clan_changed;
      remove_orig_clan_abbrev(client_num);
    }
  }
}

void client_update(game::sv::client_s *cl) {
  client_update_internal(cl, std::nullopt);
}

void client_update_post_enterworld(
    game::sv::client_s *cl, [[maybe_unused]] game::user::usercmd_t *cmd) {
  client_update(cl);

  const game::ClientNum_t client_num = sv::get_client_num(cl);
  if (game::valid_client_num(client_num)) {
    sync_all_overrides_to_client(client_num);
    scheduler::once(
        [client_num]() { sync_all_overrides_to_client(client_num); },
        scheduler::server, 250ms);
  }
}

void trigger_client_update(game::ClientNum_t client_num) {
  if (game::valid_client_num(client_num)) {
    scheduler::once(
        [client_num]() {
          game::sv::client_s *cl = sv::get_client(client_num);
          client_update_internal(cl, client_num);
        },
        scheduler::server, 50ms);
  }
}

void reset_client_name_slot(game::sv::client_s *client,
                            [[maybe_unused]] const char *reason) {
  if (client) {
    game::ClientNum_t client_num = sv::get_client_num(client);
    if (client_num != game::INVALID_CLIENT_INDEX) {
      sync_name_reset_to_clients(client_num);
      sync_clan_abbrev_reset_to_clients(client_num);
      clear_name_slot(client_num);
    }
  }
}

void initialize() {
  for (game::ClientNum_t clientNum = game::CLIENT_INDEX_0;
       clientNum < game::CLIENT_INDEX_COUNT; clientNum++) {
    trigger_client_update(clientNum);
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

      network::on(
          sync_packet_name,
          [](const game::net::netadr_t &server, const network::data_view &data,
             [[maybe_unused]] game::LocalClientNum_t local_client_num) {
            if (!is_trusted_sync_sender(server)) {
              return;
            }

            try {
              utils::byte_buffer buffer(data);
              const SyncMessageType type =
                  static_cast<SyncMessageType>(buffer.read<uint8_t>());
              const game::ClientNum_t client_num =
                  static_cast<game::ClientNum_t>(buffer.read<int32_t>());

              switch (type) {
              case SyncMessageType::CLEAR_ALL:
                clear_all();
                break;

              case SyncMessageType::SNAPSHOT: {
                clear_all();

                const uint8_t name_count = buffer.read<uint8_t>();
                for (uint8_t i = 0; i < name_count; ++i) {
                  const game::ClientNum_t snapshot_client_num =
                      static_cast<game::ClientNum_t>(buffer.read<int32_t>());
                  const std::string value = buffer.read_string();
                  if (game::valid_client_num(snapshot_client_num)) {
                    set_name_override(snapshot_client_num, value);
                  }
                }

                const uint8_t tag_count = buffer.read<uint8_t>();
                for (uint8_t i = 0; i < tag_count; ++i) {
                  const game::ClientNum_t snapshot_client_num =
                      static_cast<game::ClientNum_t>(buffer.read<int32_t>());
                  const std::string value = buffer.read_string();
                  if (game::valid_client_num(snapshot_client_num)) {
                    set_clan_abbrev_override(snapshot_client_num, value);
                  }
                }
                break;
              }

              case SyncMessageType::SET_NAME:
                if (!game::valid_client_num(client_num)) {
                  return;
                }
                set_name_override(client_num, buffer.read_string());
                break;

              case SyncMessageType::SET_TAG:
                if (!game::valid_client_num(client_num)) {
                  return;
                }
                set_clan_abbrev_override(client_num, buffer.read_string());
                break;

              case SyncMessageType::CLEAR_NAME:
                if (!game::valid_client_num(client_num)) {
                  return;
                }
                clear_name_override(client_num);
                break;

              case SyncMessageType::CLEAR_TAG:
                if (!game::valid_client_num(client_num)) {
                  return;
                }
                clear_clan_abbrev_override(client_num);
                break;
              }
            } catch (...) {
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