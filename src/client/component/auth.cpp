#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include "auth.hpp"
#include "party.hpp"
#include "command.hpp"
#include "network.hpp"
#include "scheduler.hpp"
#include "profile_infos.hpp"

#include <game/utils.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/smbios.hpp>
#include <utils/byte_buffer.hpp>
#include <utils/info_string.hpp>
#include <utils/cryptography.hpp>
#include <utils/io.hpp>
#include <utils/flags.hpp>
#include <utils/named_mutex.hpp>
#include <utils/properties.hpp>

#include <game/fragment_handler.hpp>

namespace auth {
game::EngineDependentDvar password;
std::array<game::XUID, 18> client_xuids{};
std::mutex client_xuids_mutex;

std::string get_hdd_serial() {
  DWORD serial{};
  if (!GetVolumeInformationA("C:\\", nullptr, 0, &serial, nullptr, nullptr,
                             nullptr, 0)) {
    return {};
  }

  return utils::string::va("%08X", serial);
}

std::string get_hw_profile_guid() {
  HW_PROFILE_INFO info{};
  if (!GetCurrentHwProfileA(&info)) {
    return {};
  }

  return info.szHwProfileGuid;
}

std::string get_machine_guid() {
  char guid[256]{};
  DWORD size = sizeof(guid);
  const auto result = RegGetValueA(
      HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid",
      RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY, nullptr, guid, &size);
  if (result != ERROR_SUCCESS || size <= 1) {
    return {};
  }

  return std::string{guid, size - 1};
}

std::string get_key_entropy(game::ControllerIndex_t controllerIndex) {
  std::string entropy =
      game::alias() ? "ezz-boiii-auth-alias-v2" : "ezz-boiii-auth-v2";
  entropy.append(utils::smbios::get_uuid());
  entropy.append(get_machine_guid());
  entropy.append(get_hw_profile_guid());
  entropy.append(get_hdd_serial());

  if (entropy == "ezz-boiii-auth-v2" || entropy == "ezz-boiii-auth-alias-v2") {
    entropy.resize(32);
    utils::cryptography::random::get_data(entropy.data(), entropy.size());
  }

  if (controllerIndex != game::ControllerIndex_t::CONTROLLER_INDEX_0) {
    uint32_t index_raw = static_cast<uint32_t>(controllerIndex);
    std::string index_str = utils::string::va("%X", index_raw);
    entropy.append(std::move(index_str));
  }

  return entropy;
}

static utils::named_mutex key_file_mutex{"ezz-boiii-auth-key-lock"};

std::filesystem::path key_file_path(game::ControllerIndex_t controllerIndex,
                                    bool isPublic = false) {
  std::string prefix = isPublic ? "ezz-public" : "ezz-private";
  if (game::alias()) {
    prefix += "-alias";
  }
  if (controllerIndex != game::CONTROLLER_INDEX_0) {
    prefix += "-" + std::to_string(static_cast<uint32_t>(controllerIndex));
  }
  std::string key_filename = prefix + ".key";
  return utils::properties::get_key_path() / key_filename;
}

bool load_key(const std::filesystem::path &key_path,
              utils::cryptography::ecc::key &key) {
  std::string data{};
  if (!utils::io::read_file(key_path, &data)) {
    return false;
  }

  key.deserialize(data);
  return key.is_valid();
}

bool write_key(const std::filesystem::path &key_path,
               const utils::cryptography::ecc::key &key) {
  auto temp_path = key_path;
  temp_path += ".new";

  const auto serialized = key.serialize();
  if (serialized.empty() || !utils::io::write_file(temp_path, serialized)) {
    utils::io::remove_file(temp_path);
    return false;
  }

  utils::cryptography::ecc::key written_key{};
  if (!load_key(temp_path, written_key) ||
      written_key.get_hash() != key.get_hash()) {
    utils::io::remove_file(temp_path);
    return false;
  }

  if (!MoveFileExW(temp_path.wstring().c_str(), key_path.wstring().c_str(),
                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    utils::io::remove_file(temp_path);
    return false;
  }

  return true;
}

utils::cryptography::ecc::key
load_or_generate_key(game::ControllerIndex_t controllerIndex) {
  const std::unique_lock lock(key_file_mutex);
  const auto key_path = key_file_path(controllerIndex);
  auto backup_path = key_path;
  backup_path += ".bak";

  utils::cryptography::ecc::key key{};
  if (load_key(key_path, key)) {
    utils::cryptography::ecc::key backup_key{};
    if (!load_key(backup_path, backup_key) ||
        backup_key.get_hash() != key.get_hash()) {
      if (!write_key(backup_path, key)) {
        printf("Failed to back up cryptographic key!\n");
      }
    }

    return key;
  }

  if (load_key(backup_path, key)) {
    printf("Restoring cryptographic key from backup.\n");
    if (!write_key(key_path, key)) {
      printf("Failed to restore cryptographic key!\n");
    }

    return key;
  }

  key = utils::cryptography::ecc::generate_key(
      512, get_key_entropy(controllerIndex));
  if (!key.is_valid()) {
    throw std::runtime_error("Failed to generate cryptographic key!");
  }

  if (!write_key(key_path, key)) {
    printf("Failed to write cryptographic key!\n");
  } else if (!write_key(backup_path, key)) {
    printf("Failed to back up cryptographic key!\n");
  }

  return key;
}

utils::cryptography::ecc::key
get_key_internal(game::ControllerIndex_t controllerIndex) {
  auto key = load_or_generate_key(controllerIndex);

  std::filesystem::path key_path = key_file_path(controllerIndex, true);
  if (!utils::io::write_file(key_path.generic_string(), key.get_public_key())) {
    printf("Failed to write public key!\n");
  }

  return key;
}

utils::cryptography::ecc::key get_key(game::ControllerIndex_t controllerIndex) {
  return get_key_internal(controllerIndex);
}

static const size_t CHALLENGE_LENGTH = 32;
static std::mutex latest_challenge_mutex;
typedef array<uint8_t, CHALLENGE_LENGTH> challenge_t;
static challenge_t latest_challenge = {0};

void set_challenge(const game::net::netadr_t &target,
                   const network::data_view &data,
                   game::LocalClientNum_t clientNum) {

  {
    std::lock_guard lock(latest_challenge_mutex);

    // We probably do not need to bother zeroing the latest_challenge bytes
    // first, as if an incomplete challenge has been received, this will cause
    // an error either way, cleanly handling this exceptional case for us.
    memcpy(latest_challenge, data.data(),

           std::min<size_t>(std::size(latest_challenge), data.size()));
  }

  for (game::LocalClientNum_t localClientIdx = game::LOCAL_CLIENT_0;
       localClientIdx < game::LOCAL_CLIENT_COUNT; localClientIdx++) {

    if (game::com::Com_LocalClient_IsBeingUsed(localClientIdx)) {
      game::connstate_t *clientConnectionState =
          &game::cg::clientUIActives->actives[localClientIdx].connectionState;
      if (*clientConnectionState == game::connstate_t::CHALLENGING) {
        *clientConnectionState = game::connstate_t::CONNECTING;
      }
    }
  }
}

std::string serialize_connect_data(game::ControllerIndex_t controllerIndex,
                                   const char *data, const int length) {

  std::string challenge;

  {
    std::lock_guard lock(latest_challenge_mutex);
    challenge = std::string(
        const_cast<const char *>(reinterpret_cast<char *>(latest_challenge)),
        CHALLENGE_LENGTH);
  }

  utils::byte_buffer buffer{};
  buffer.write_string(get_key(controllerIndex).serialize(PK_PUBLIC));
  std::string signature = utils::cryptography::ecc::sign_message(
      get_key(controllerIndex), challenge);

  buffer.write_string(std::move(signature));

  profile_infos::get_profile_info()
      .value_or(profile_infos::profile_info{})
      .serialize(buffer);

  buffer.write_string(data);

  return buffer.move_buffer();
}

bool send_fragmented_connect_packet(game::ControllerIndex_t controllerIndex,
                                    const game::net::netsrc_t sock,
                                    const game::net::netadr_t *adr,
                                    const char *data, const int length) {

  const std::string connect_data =
      serialize_connect_data(controllerIndex, data, length);
  game::fragment_handler::fragment_data //
      (connect_data.data(), connect_data.size(),
       [&](const utils::byte_buffer &buffer) {
         utils::byte_buffer packet_buffer{};
         packet_buffer.write("connect ");
         packet_buffer.write(buffer);

         const auto &fragment_packet = packet_buffer.get_buffer();

         game::net::NET_OutOfBandData(sock, adr, fragment_packet.data(),
                                      static_cast<int>(fragment_packet.size()));
       });
  return true;
}

void distribute_player_xuid(const game::net::netadr_t &target,
                            const size_t player_index, const game::XUID xuid) {
  if (player_index < 18) {
    utils::byte_buffer buffer{};
    buffer.write(static_cast<uint32_t>(player_index));
    buffer.write(xuid);

    game::XUID localXuid1 = get_guid(game::CONTROLLER_INDEX_0);
    game::XUID localXuid2 = get_guid(game::CONTROLLER_INDEX_1);

    game::foreach_connected_client(
        [&](const game::sv::client_s &client, const size_t index) {
          if (client.xuid != localXuid1 && client.xuid != localXuid2) {
            if (client.address.type != game::net::NA_BOT) {
              network::send(client.address, "playerXuid", buffer.get_buffer());
            }

            if (index != player_index && target.type != game::net::NA_BOT) {
              utils::byte_buffer current_buffer{};
              current_buffer.write(static_cast<uint32_t>(index));
              current_buffer.write(client.xuid);

              network::send(target, "playerXuid", current_buffer.get_buffer());
            }
          }
        });
  }
}

void handle_new_player(const game::net::netadr_t &target) {
  const command::params_sv params{};

  if (params.size() > 1) {
    const utils::info_string info_string(params[1]);
    const game::XUID xuid =
        strtoull(info_string.get("xuid").data(), nullptr, 16);

    size_t player_index = 18;
    game::first_connected_client(
        [&](game::sv::client_s &client, const size_t index) {
          if (client.address == target && client.xuid == 0) {
            client.xuid = xuid;
            player_index = index;
            return true;
          }
          return false;
        });

    distribute_player_xuid(target, player_index, xuid);
  }
}

inline bool is_invalid_char(const int c) {
  return c == '%' || c == '~' || c < 32 || c > 126;
}

static constexpr uint64_t CHALLENGE_EVICT_MS =
    std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::minutes(10))
        .count();
struct IssuedChallenge {
  uint64_t time;
  challenge_t challenge;
  bool initialized;

  inline bool valid() const noexcept {
    if (initialized) {
      const uint64_t now_ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count();
      return now_ms - time < CHALLENGE_EVICT_MS;
    }
    return true;
  }

  inline bool stale() const noexcept { return !valid(); }

  inline void refresh() noexcept {
    if (stale()) {
      utils::cryptography::random::get_data(reinterpret_cast<void *>(challenge),
                                            std::size(challenge));
      time = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());
      initialized = true;
    }
  }

  inline IssuedChallenge generate() noexcept {
    IssuedChallenge result;
    result.refresh();

    return result;
  }
};
static concurrent_hash_map<game::net::netadr_t, IssuedChallenge>
    issued_challenges = {};

thread_local challenge_t challenge_buf = {0};
const challenge_t &get_challenge(const game::net::netadr_t &target) {
  if (game::no_ext()) {
    while (issued_challenges.try_emplace_l(target, [](auto &v) {
      v.second.refresh();
      memcpy(challenge_buf, &v.second.challenge, std::size(challenge_buf));
    })) {
    }
  } else {
    typedef fastcallPtr_t<void(const game::net::netadr_t *adr, void *buf,
                               size_t size)>
        get_challenge_func_t;
    const get_challenge_func_t get_challenge =
        reinterpret_cast<const get_challenge_func_t>(
            game::select(0x1412E15E0, 0x14016DDC0));
    get_challenge(&target, challenge_buf, std::size(challenge_buf));
  }
  return challenge_buf;
}

void send_challenge(const game::net::netadr_t &addr,
                    [[maybe_unused]] const network::data_view &,
                    [[maybe_unused]] game::LocalClientNum_t localClientNum) {

  constexpr char CHALLENGE_RESPONSE_COMMAND_PREFIX[] = {
      '\xFF', '\xFF', '\xFF', '\xFF', 'g', 'e', 't', 'C', 'h',
      'a',    'l',    'l',    'e',    'n', 'g', 'e', 'R', 'e',
      's',    'p',    'o',    'n',    's', 'e', ' '};
  constexpr size_t CHALLENGE_RESPONSE_BUF_LEN =
      std::size(CHALLENGE_RESPONSE_COMMAND_PREFIX) + sizeof(challenge_t);

  str<CHALLENGE_RESPONSE_BUF_LEN> challenge_response_buf;
  memcpy(challenge_response_buf, CHALLENGE_RESPONSE_COMMAND_PREFIX,
         std::size(CHALLENGE_RESPONSE_COMMAND_PREFIX));

  const challenge_t &challenge = get_challenge(addr);

#ifndef NDEBUG
  const std::string hex_challenge_resp = utils::string::hexdump(
      challenge_response_buf, std::size(challenge_response_buf));
  game::net::netadr_str_t addr_buf;
  game::trace("[Auth][Challenge] sending challenge to %s: \"%s\"",
              addr.toString(addr_buf), hex_challenge_resp.c_str());
#endif

  memcpy(&challenge_response_buf[std::size(CHALLENGE_RESPONSE_COMMAND_PREFIX)],
         challenge, std::size(challenge));

  network::send_data(addr, challenge_response_buf,
                     std::size(challenge_response_buf));
}

void evict_stale_challenges() {
  std::vector<game::net::netadr_t> evict_addresses;

  issued_challenges.for_each([&evict_addresses](const auto &v) {
    if (v.second.stale()) {
      evict_addresses.push_back(v.first);
    }
  });

  for (const game::net::netadr_t addr : evict_addresses) {
    issued_challenges.erase_if(addr, [](const auto &v) {
      // In case of modification in the interim
      return v.second.stale();
    });
  }
}

void dispatch_connect_packet(const game::net::netadr_t &target,
                             const std::string &data,
                             game::LocalClientNum_t clientNum) {
  utils::byte_buffer buffer(data);

  utils::cryptography::ecc::key key{};
  std::string key_ser = buffer.read_string();
  std::string signature_serialized_str = buffer.read_string();
  key.deserialize(&key_ser);

  if (!utils::cryptography::ecc::verify_message<CHALLENGE_LENGTH>(
          key, get_challenge(target), signature_serialized_str)) {

    network::send(target, "error", "Bad signature");
    return;
  }

  const profile_infos::profile_info info(buffer);

  const std::string connect_data = buffer.read_string();
  const command::params_sv params(connect_data);

  if (params.size() < 2) {
    return;
  }

  const auto _ = profile_infos::acquire_profile_lock();

  const utils::info_string info_string(params[1]);
  const game::XUID xuid = strtoull(info_string.get("xuid").data(), nullptr, 16);
  if (xuid != key.get_hash()) {
    network::send(target, "error", "Bad XUID");
    return;
  }

  const std::string name = info_string.get("name");

  const auto is_name_invalid = [&name]() -> bool {
    return std::ranges::any_of(name,
                               [](const char c) { return is_invalid_char(c); });
  };

  if (name.empty() || is_name_invalid()) {
    network::send(target, "error", "Bad name");
    return;
  }

  profile_infos::add_and_distribute_profile_info(target, xuid, info);

  game::sv::SV_DirectConnect(target);
  handle_new_player(target);
}

void handle_connect_packet_fragment(const game::net::netadr_t &target,
                                    const network::data_view &data,
                                    game::LocalClientNum_t clientNum) {
  if (game::server_running()) {
    utils::byte_buffer buffer(data);

    std::string final_packet{};
    if (game::fragment_handler::handle(target, buffer, final_packet)) {
      scheduler::once(
          [t = target, p = std::move(final_packet), clientNum = clientNum] {
            dispatch_connect_packet(t, p, clientNum);
          },
          scheduler::server);
    }
  }
}

void handle_player_xuid_packet(const game::net::netadr_t &target,
                               const network::data_view &data,
                               game::LocalClientNum_t clientNum) {
  if (!game::server_running() && party::is_host(target)) {
    utils::byte_buffer buffer(data);

    const uint32_t player_id = buffer.read<uint32_t>();
    const game::XUID xuid = buffer.read<game::XUID>();

    std::lock_guard lock(client_xuids_mutex);
    if (player_id < client_xuids.size()) {
      client_xuids[player_id] = xuid;
    }
  }
}

void direct_connect_bots_stub(const game::net::netadr_t address) {
  game::sv::SV_DirectConnect(address);
  handle_new_player(address);
}

game::XUID get_client_guid(game::ControllerIndex_t controllerIndex) {
  static const std::array<game::XUID, 2> guids = {
      get_key(game::CONTROLLER_INDEX_0).get_hash(),
      get_key(game::CONTROLLER_INDEX_1).get_hash()};
  controllerIndex = game::valid_controller_index(controllerIndex)
                        ? controllerIndex
                        : game::CONTROLLER_INDEX_0;
  return guids[static_cast<uint32_t>(controllerIndex)];
}

game::XUID get_guid(game::ControllerIndex_t controllerIndex) {
  if (!game::is_server())
    return get_client_guid(controllerIndex);

  static const game::XUID server_guid = static_cast<game::XUID>(
      0x110000100000000 |
      (::utils::cryptography::random::get_integer() & ~0x80000000));
  return controllerIndex == game::CONTROLLER_INDEX_0 ? server_guid : 0;
}

game::XUID get_guid(const size_t client_num) {
  if (client_num >= 18) {
    return 0;
  }

  if (!game::server_running()) {
    std::lock_guard lock(client_xuids_mutex);
    return client_xuids[client_num];
  }

  game::XUID xuid = 0;
  const auto callback = [&xuid](const game::sv::client_s &client) {
    xuid = client.xuid;
  };

  if (!game::access_connected_client(client_num, callback)) {
    return 0;
  }

  return xuid;
}

void clear_stored_guids() {
  std::lock_guard lock(client_xuids_mutex);
  for (game::XUID &xuid : client_xuids) {
    xuid = 0;
  }
}

void clear_stored_challenge() {
  std::lock_guard lock(latest_challenge_mutex);
  if (game::cg::clientUIActives->actives[0].connectionState !=
      game::connstate_t::CHALLENGING) {
    memset(latest_challenge, 0, CHALLENGE_LENGTH);
  }
}

utils::hook::detour LiveUser_UserGetXuid_hook;
bool LiveUser_UserGetXuid_stub(int64_t controllerIndex, game::XUID *xuid) {
  *xuid = get_guid(static_cast<game::ControllerIndex_t>(
      static_cast<int32_t>(controllerIndex)));
  return true;
}

utils::hook::detour CL_DisconnectPacket_hook;
void CL_DisconnectPacket_ClearStoredChallenge(
    game::LocalClientNum_t localClientNum, game::net::netadr_t *from,
    const char *reason) {
  if (localClientNum == game::LOCAL_CLIENT_0) {
    clear_stored_challenge();
  }
  CL_DisconnectPacket_hook.invoke(localClientNum, from, reason);
}
utils::hook::detour CL_Disconnect_hook;
void CL_Disconnect_ClearStoredChallenge(game::LocalClientNum_t localClientNum,
                                        bool deactivateClient) {
  if (localClientNum == game::LOCAL_CLIENT_0) {
    clear_stored_challenge();
  }
  CL_Disconnect_hook.invoke(localClientNum, deactivateClient);
}

utils::hook::detour LiveUser_GetXuid_hook;

struct component final : generic_component {
  void post_unpack() override {

    if (game::no_ext()) {
      scheduler::loop(evict_stale_challenges, scheduler::pipeline::async, 5min);
    }

    // Skip connect handler
    utils::hook::set<uint8_t>(game::select(0x142253EFA, 0x14053714A), 0xEB);
    network::on("connect", handle_connect_packet_fragment);
    network::on("playerXuid", handle_player_xuid_packet);
    network::on("getChallengeResponse", set_challenge);
    if (game::no_ext()) {
      network::on("getchallenge", send_challenge);
    }

    // Intercept SV_DirectConnect in SV_AddTestClient
    utils::hook::call(game::select(0x1422490DC, 0x14052E582),
                      direct_connect_bots_stub);

    scheduler::once(
        [] {
          password = game::register_dvar_string(
              "password", "", game::DVAR_USERINFO, "password");
        },
        scheduler::pipeline::main);

    // Patch steam id bit check
    std::vector<std::pair<size_t, size_t>> patches{};
    const auto p = [&patches](const size_t a, const size_t b) {
      patches.emplace_back(a, b);
    };

    if (game::is_server()) {
      p(0x1404747C6_g, 0x140474806_g);
      p(0x140474A24_g, 0x140474A68_g);
      p(0x140474A85_g, 0x140474AC6_g);
      p(0x140457ED0_g, 0x140457F26_g);
      p(0x140473DD8_g, 0x140473E19_g);
      p(0x1404743D5_g, 0x140474423_g);
      p(0x1404744FD_g, 0x140474553_g);
      p(0x14047462D_g, 0x140474677_g);
      p(0x140475057_g, 0x14047509F_g); // ?
      p(0x140475672_g, 0x1404756B5_g);
      p(0x140477322_g, 0x140477365_g); // ?
    } else {
      p(0x141E19CED_g, 0x141E19D3B_g);
      p(0x141EB2C76_g, 0x141EB2CB6_g);
      p(0x141EB2DAD_g, 0x141EB2DF2_g);
      p(0x141EB3C35_g, 0x141EB3C76_g);
      p(0x141E19AD0_g, 0x141E19B26_g);
      //
      p(0x141EB0EE8_g, 0x141EB0F29_g);
      p(0x141EB0FA8_g, 0x141EB0FE9_g);
      p(0x141EB2525_g, 0x141EB2573_g);
      p(0x141EB264D_g, 0x141EB26A3_g);
      p(0x141EB277D_g, 0x141EB27C7_g);

      p(0x141EB2AEA_g, 0x141EB2AFA_g);
      p(0x141EB2B01_g, 0x141EB2B33_g);

      p(0x141EB3137_g, 0x141EB3147_g);
      p(0x141EB314E_g, 0x141EB317F_g);

      p(0x141EB5377_g, 0x141EB53BF_g); // ?
      p(0x141EB5992_g, 0x141EB59D5_g);
      p(0x141EB74D2_g, 0x141EB7515_g); // ?

      LiveUser_UserGetXuid_hook.create(
          game::live::user::LiveUser_UserGetXuid.get(),
          LiveUser_UserGetXuid_stub);
      LiveUser_GetXuid_hook.create(game::live::user::LiveUser_GetXuid.get(),
                                   get_guid);
      CL_DisconnectPacket_hook.create(game::cl::CL_DisconnectPacket.get(),
                                      CL_DisconnectPacket_ClearStoredChallenge);
      CL_Disconnect_hook.create(game::cl::CL_Disconnect.get(),
                                CL_Disconnect_ClearStoredChallenge);
    }

    for (const auto &patch : patches) {
      utils::hook::jump(patch.first, patch.second);
    }
  }
};
} // namespace auth

REGISTER_COMPONENT(auth::component)