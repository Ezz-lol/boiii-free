#include <std_include.hpp>

#include "game/utils.hpp"
#include "sv.hpp"

#include <game/impl/sl/sl.hpp>

namespace game {
namespace sv {

void SV_SetConfigString_Impl(const int32_t index, const char *val) {
  // TODO: define SL string users as a discrete enumeration, use the enumeration
  // inline instead.
  constexpr uint32_t SL_STRING_USER = 0;
  // TODO: define SL string types as a discrete enumeration, use the enumeration
  // inline instead.
  constexpr int32_t SL_STRING_TYPE = 5;

  if (static_cast<size_t>(index) >= std::size(sv->configstrings)) {
    // The original error line number has to be an arbitrary magic number
    // (matches the line number displayed in dedicated server engine), and this
    // particular line number will inherently never be reused, so it does not
    // need to be defined separately.
    // clang-format off
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    // clang-format on
    com::Com_Error_(R"(q:\t7\pc\code\src\server_mp\sv_init_mp.cpp)", 220,
                    // clang-format off
                    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
                    // clang-format on
                    errorParm::SERVERDISCONNECT,
                    "SV_SetConfigString: bad index %i\n", index);
  } else {
    volatile scr::ScrString_t &cfgstr = sv::sv->configstrings[index];
    if (cfgstr != 0) {
      const char *set_val = val == nullptr ? "" : val;
      if (strcmp(set_val, sl::SL_ConvertToString(cfgstr)) != 0) {
        sl::SL_RemoveRefToStringFree_ThreadSafe(cfgstr);
        cfgstr = index > 0xE2C
                     ? sl::SL_GetString(set_val, SL_STRING_USER, SL_STRING_TYPE)
                     : sl::SL_GetLowercaseString(set_val, SL_STRING_USER,
                                                 SL_STRING_TYPE);

        if (sv->running()) {
          const size_t val_len = strlen(set_val);
          constexpr size_t CFGSTR_PART_BUF_LEN = 0x400;
          char cfgstr_part_buf[CFGSTR_PART_BUF_LEN];
          snprintf(cfgstr_part_buf, sizeof(cfgstr_part_buf), "%i", index);
          const size_t index_len = strlen(cfgstr_part_buf);
          const size_t max_per_packet_msg_len =
              MAXIMUM_RELIABLE_COMMAND_DATA_LEN - index_len -
              1 /* the space (" ") following index */
              ;
          if (val_len <= max_per_packet_msg_len) {
            const char *packet = utils::string::va(
                "%c %i %s", ReliableCommand::CONFIG_STRING_MODIFIED, index,
                set_val);
            foreach_active_client(
                [packet](client_t &cl,
                         [[maybe_unused]] size_t client_index) -> void {
                  SV_AddServerCommand(&cl, net::svscmd_type::RELIABLE, packet);
                });
          } else {
            size_t sent = 0;
            while (sent < val_len) {
              size_t chunk_len = 0;

              ReliableCommand cmd = ReliableCommand::NOP;
              if (sent != 0) {
                if (sent + max_per_packet_msg_len < val_len) {
                  chunk_len = max_per_packet_msg_len;
                  cmd = ReliableCommand::BCS_APPEND;

                  const size_t init_chunk_len = chunk_len;
                  while (set_val[sent + chunk_len] != ' ') {
                    --chunk_len;
                    if (chunk_len == 0) {
                      com::Com_Error_(
                          // The original error line number has to be an
                          // arbitrary magic number (matches the line number
                          // displayed in dedicated server engine)
                          // clang-format off
                          // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
                          // clang-format on
                          R"(q:\t7\pc\code\src\server_mp\sv_init_mp.cpp)", 280,
                          // clang-format off
                          // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
                          // clang-format on
                          errorParm::SERVERDISCONNECT,
                          "SV_SetConfigstring: big config string with %d empty "
                          "spaces\n",
                          init_chunk_len);
                      return;
                    }
                  }
                } else {
                  chunk_len = val_len - sent;
                  cmd = ReliableCommand::BCS_FINALIZE;
                }
              } else {
                /*
                   Must have length that requires > 1 packet to send
                   if we have reached this branch, so first packet length is
                   guaranteed to not overflow length of config string value.
                */
                chunk_len = max_per_packet_msg_len;
                cmd = ReliableCommand::BCS_INIT;
              }

              strscpy(cfgstr_part_buf, &set_val[sent], chunk_len);

              const char *packet =
                  utils::string::va("%c %i %s", cmd, index, cfgstr_part_buf);
              foreach_active_client(
                  [packet](client_t &cl,
                           [[maybe_unused]] size_t client_index) -> void {
                    SV_AddServerCommand(&cl, net::svscmd_type::RELIABLE,
                                        packet);
                  });

              sent += chunk_len;
            }
          }
        }
      }
    }
  }
}

int32_t SV_GetUniqueClientMachineCount_Impl() {
  const client_s_cl *clients = *svs_clients_cl;
  int32_t count = 0;
  const int32_t maxClients = static_cast<int32_t>(get_max_client_count());
  for (int32_t index = 0; index < maxClients; ++index) {
    const client_s &client = clients[index];
    if (!client.bIsTestClient && client.state >= net::clientState_t::ACTIVE &&
        std::none_of(clients, clients + index, [&](const client_s &other) {
          return SV_IsSameMachine(other.address, client.address);
        })) {
      ++count;
    }
  }
  return count;
}
} // namespace sv
} // namespace game
