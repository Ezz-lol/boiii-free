#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include "command.hpp"
#include "name.hpp"
#include "network.hpp"
#include "party.hpp"
#include "scheduler.hpp"
#include "steam_proxy.hpp"
#include "toast.hpp"
#include <game/utils.hpp>

#include <str.hpp>
#include <utils/byte_buffer.hpp>
#include <utils/concurrency.hpp>
#include <utils/nt.hpp>
#include <utils/properties.hpp>
#include <utils/string.hpp>

#include "game_event.hpp"
#include "sv.hpp"

namespace name {
namespace {
constexpr const str<13> sync_packet_name = "nameoverride";
constexpr size_t max_override_length = 256;

enum class SyncMessageType : uint8_t {
  SET_NAME = 0x1,
  SET_TAG = 0x2,
  CLEAR_NAME = 0x3,
  CLEAR_TAG = 0x4,
  SNAPSHOT = 0x5,
};

struct override_slot {
  std::optional<std::string> name;
  std::optional<std::string> tag;
  std::optional<std::string> original_name;
  std::optional<std::string> original_tag;
};

std::mutex overrides_mutex;
std::array<override_slot, game::CLIENT_INDEX_COUNT> overrides{};

utils::concurrency::container<std::string> player_name{};

override_slot &slot(const game::ClientNum_t client_num) {
  return overrides[static_cast<size_t>(client_num)];
}

std::string sanitize_name(const std::string &name) {
  std::string result;
  for (const char c : name) {
    const unsigned char uc = static_cast<unsigned char>(c);
    if (uc >= 32 && uc <= 126)
      result += c;
  }
  return result;
}

void update_player_name(const std::string &new_name) {
  player_name.access([&](std::string &name) { name = new_name; });
  utils::properties::store("playerName", new_name);
}

void load_player_name() {
  std::string name =
      sanitize_name(utils::properties::load("playerName").value_or(""));
  if (name.empty())
    name = sanitize_name(steam_proxy::get_player_name());
  if (name.empty())
    name = sanitize_name(utils::nt::get_user_name());
  if (name.empty())
    name = "Unknown Soldier";
  update_player_name(name);
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

bool is_syncable(const game::net::netadr_t &address) {
  return address.type != game::net::NA_BAD && address.type != game::net::NA_BOT;
}

void send(const game::net::netadr_t &target, const std::string &data) {
  if (is_syncable(target)) {
    network::send(target, sync_packet_name, data);
  }
}

void broadcast(const SyncMessageType type, const game::ClientNum_t client_num,
               const std::optional<std::string> &value = std::nullopt) {
  utils::byte_buffer buffer{};
  buffer.write(static_cast<uint8_t>(type));
  buffer.write(static_cast<int32_t>(client_num));
  if (value) {
    buffer.write_string(*value);
  }

  game::foreach_connected_client([&](game::sv::client_s &client, size_t) {
    send(client.address, buffer.get_buffer());
  });
}

void send_snapshot(const game::net::netadr_t &target) {
  utils::byte_buffer names{};
  utils::byte_buffer tags{};
  uint8_t name_count = 0;
  uint8_t tag_count = 0;
  {
    std::lock_guard lock(overrides_mutex);
    for (size_t i = 0; i < overrides.size(); ++i) {
      if (overrides[i].name) {
        names.write(static_cast<int32_t>(i));
        names.write_string(*overrides[i].name);
        ++name_count;
      }
      if (overrides[i].tag) {
        tags.write(static_cast<int32_t>(i));
        tags.write_string(*overrides[i].tag);
        ++tag_count;
      }
    }
  }

  utils::byte_buffer buffer{};
  buffer.write(static_cast<uint8_t>(SyncMessageType::SNAPSHOT));
  buffer.write(static_cast<int32_t>(game::INVALID_CLIENT_INDEX));
  buffer.write(name_count);
  buffer.write(names);
  buffer.write(tag_count);
  buffer.write(tags);
  send(target, buffer.get_buffer());
}

void notify_ui() {
  if (!game::is_client())
    return;
  for (const auto delay : {0ms, 500ms}) {
    scheduler::once(
        [] {
          if (const auto global = game::ui::UI_Model_GetGlobalModel()) {
            if (const auto model = game::ui::UI_Model_CreateModelFromPath(
                    global, "boiiiNames.update"))
              game::ui::UI_Model_ForceNotify(model);
          }
        },
        scheduler::main, delay);
  }
}

void write_if_changed(char *dest, size_t size, const std::string &value,
                      bool *changed = nullptr) {
  if (std::strcmp(dest, value.c_str()) != 0) {
    utils::string::copy(dest, size, value.c_str());
    if (changed)
      *changed = true;
  }
}

void apply(const game::ClientNum_t client_num) {
  game::sv::client_s *cl = sv::get_client(client_num);
  if (!game::valid_engine_ptr(cl) || !game::valid_engine_ptr(cl->gentity) ||
      !game::valid_engine_ptr(cl->gentity->client)) {
    return;
  }

  game::level::clientState_t &cs = cl->gentity->client->sess.cs;
  std::lock_guard lock(overrides_mutex);
  override_slot &entry = slot(client_num);

  if (entry.name && !entry.original_name)
    entry.original_name = cs.name;
  if (entry.tag && !entry.original_tag)
    entry.original_tag = cs.clanAbbrev;

  const std::optional<std::string> name =
      entry.name ? strip_color_codes(*entry.name) : entry.original_name;
  const std::optional<std::string> tag =
      entry.tag ? strip_color_codes(*entry.tag) : entry.original_tag;

  if (name) {
    write_if_changed(cs.name, sizeof(cs.name), *name);
    write_if_changed(cl->name, sizeof(cl->name), *name);
  }
  if (tag) {
    bool changed = false;
    write_if_changed(cs.clanAbbrev, sizeof(cs.clanAbbrev), *tag, &changed);
    write_if_changed(cl->clanAbbrev, sizeof(cl->clanAbbrev), *tag);
    if (changed)
      cs.clanAbbrevEV = true;
  }

  if (!entry.name)
    entry.original_name.reset();
  if (!entry.tag)
    entry.original_tag.reset();
}

void enforce_overrides() {
  if (!game::server_running())
    return;
  for (auto client_num = game::CLIENT_INDEX_0;
       client_num < game::CLIENT_INDEX_COUNT;
       client_num = static_cast<game::ClientNum_t>(client_num + 1)) {
    bool active;
    {
      std::lock_guard lock(overrides_mutex);
      active = slot(client_num).name || slot(client_num).tag;
    }
    if (active)
      apply(client_num);
  }
}

void update_override(const game::ClientNum_t client_num, const bool is_tag,
                     const std::optional<std::string> &value) {
  if (!game::valid_client_num(client_num))
    return;
  {
    std::lock_guard lock(overrides_mutex);
    auto &field = is_tag ? slot(client_num).tag : slot(client_num).name;
    field = value;
  }
  apply(client_num);
  broadcast(
      value
          ? (is_tag ? SyncMessageType::SET_TAG : SyncMessageType::SET_NAME)
          : (is_tag ? SyncMessageType::CLEAR_TAG : SyncMessageType::CLEAR_NAME),
      client_num, value);
  notify_ui();
}

void clear_all() {
  std::lock_guard lock(overrides_mutex);
  overrides = {};
}

void store_received(const game::ClientNum_t client_num, const bool is_tag,
                    std::optional<std::string> value) {
  if (!game::valid_client_num(client_num) ||
      (value && value->size() > max_override_length))
    return;
  std::lock_guard lock(overrides_mutex);
  (is_tag ? slot(client_num).tag : slot(client_num).name) = std::move(value);
}

void read_snapshot_list(utils::byte_buffer &buffer, const bool is_tag) {
  const auto count = buffer.read<uint8_t>();
  for (uint8_t i = 0; i < count && i < game::CLIENT_INDEX_COUNT; ++i) {
    const auto client_num =
        static_cast<game::ClientNum_t>(buffer.read<int32_t>());
    store_received(client_num, is_tag, buffer.read_string());
  }
}

void receive_override(const game::net::netadr_t &server,
                      const network::data_view &data, game::LocalClientNum_t) {
  if (game::server_running() || !party::is_host(server))
    return;

  try {
    utils::byte_buffer buffer(data);
    const auto type = static_cast<SyncMessageType>(buffer.read<uint8_t>());
    const auto client_num =
        static_cast<game::ClientNum_t>(buffer.read<int32_t>());

    switch (type) {
    case SyncMessageType::SNAPSHOT:
      clear_all();
      read_snapshot_list(buffer, false);
      read_snapshot_list(buffer, true);
      break;
    case SyncMessageType::SET_NAME:
    case SyncMessageType::SET_TAG:
      store_received(client_num, type == SyncMessageType::SET_TAG,
                     buffer.read_string());
      break;
    case SyncMessageType::CLEAR_NAME:
    case SyncMessageType::CLEAR_TAG:
      store_received(client_num, type == SyncMessageType::CLEAR_TAG,
                     std::nullopt);
      break;
    }
    notify_ui();
  } catch (...) {
  }
}

void on_enter_world(game::sv::client_s *cl, game::user::usercmd_t *) {
  apply(sv::get_client_num(cl));
  send_snapshot(cl->address);
}

void on_remove_client(game::sv::client_s *cl, const char *) {
  const game::ClientNum_t client_num = sv::get_client_num(cl);
  if (!game::valid_client_num(client_num))
    return;
  {
    std::lock_guard lock(overrides_mutex);
    slot(client_num) = {};
  }
  broadcast(SyncMessageType::CLEAR_NAME, client_num);
  broadcast(SyncMessageType::CLEAR_TAG, client_num);
  notify_ui();
}

void clear_on_disconnect() {
  static bool was_connected = false;
  const bool connected =
      game::cg::clientUIActives->actives[0].connectionState ==
      game::connstate_t::ACTIVE;
  if (was_connected && !connected && !game::server_running())
    clear_all();
  was_connected = connected;
}
} // namespace

const char *get_player_name() {
  const std::string n = player_name.copy();
  return utils::string::va("%.*s", static_cast<int>(n.size()), n.data());
}

void set_name_override(const game::ClientNum_t client_num,
                       const std::string &name) {
  update_override(client_num, false, name);
}

void set_clan_abbrev_override(const game::ClientNum_t client_num,
                              const std::string &tag) {
  update_override(client_num, true, tag);
}

void clear_name_override(const game::ClientNum_t client_num) {
  update_override(client_num, false, std::nullopt);
}

void clear_clan_abbrev_override(const game::ClientNum_t client_num) {
  update_override(client_num, true, std::nullopt);
}

std::optional<std::string> get_name_override(game::ClientNum_t client_num) {
  if (!game::valid_client_num(client_num))
    return std::nullopt;
  std::lock_guard lock(overrides_mutex);
  return slot(client_num).name;
}

std::optional<std::string>
get_clan_abbrev_override(game::ClientNum_t client_num) {
  if (!game::valid_client_num(client_num))
    return std::nullopt;
  std::lock_guard lock(overrides_mutex);
  return slot(client_num).tag;
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("name");

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

      network::on(sync_packet_name, receive_override);
      scheduler::loop(clear_on_disconnect, scheduler::main, 1s);
    }

    sv::on_cliententerworld(on_enter_world);
    sv::on_removeclient(on_remove_client);
    scheduler::loop(enforce_overrides, scheduler::server);

    game_event::on_any(clear_all);
  }

  component_priority priority() const override {
    return component_priority::name;
  }
};
} // namespace name

REGISTER_COMPONENT(name::component)
