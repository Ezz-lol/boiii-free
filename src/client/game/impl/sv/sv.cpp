#include <std_include.hpp>
#include <game/game.hpp>
#include "sv.hpp"
#include "game/utils.hpp"
#include <game/impl/sl/sl.hpp>

namespace game {
namespace sv {

void SV_SetConfigString_Impl(const int32_t index, const char *val) {
  if (static_cast<uint32_t>(index) > 0xE2D) {
    com::Com_Error_("q:\\t7\\pc\\code\\src\\server_mp\\sv_init_mp.cpp", 220,
                    errorParm::SERVERDISCONNECT,
                    "SV_SetConfigString: bad index %i\n", index);
    return;
  }
  volatile scr::ScrString_t *cfgstr = &sv::sv->configstrings[index];
  if (*cfgstr != 0) {
    const char *set_val = val ? val : "";
    if (strcmp(set_val, sl::SL_ConvertToString(*cfgstr)) != 0) {
      sl::SL_RemoveRefToStringFree_ThreadSafe(*cfgstr);
      *cfgstr = index > 0xE2C ? sl::SL_GetString(set_val, 0, 5)
                              : sl::SL_GetLowercaseString(set_val, 0, 5);

      if (sv->running()) {
        const size_t val_len = strlen(set_val);
        char cfgstr_part_buf[0x400];
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
                SV_AddServerCommand(&cl, net::SV_CMD_RELIABLE, packet);
              });
        } else {
          size_t sent = 0;
          while (sent < val_len) {

            size_t chunk_len;

            ReliableCommand cmd;
            if (sent) {
              if (sent + max_per_packet_msg_len < val_len) {
                chunk_len = max_per_packet_msg_len;
                cmd = ReliableCommand::BCS_APPEND;

                const size_t init_chunk_len = chunk_len;
                while (set_val[sent + chunk_len] != ' ') {
                  --chunk_len;
                  if (chunk_len == 0) {
                    com::Com_Error_(
                        "q:\\t7\\pc\\code\\src\\server_mp\\sv_init_mp.cpp", 280,
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
                  SV_AddServerCommand(&cl, net::SV_CMD_RELIABLE, packet);
                });

            sent += chunk_len;
          }
        }
      }
    }
  }
}
} // namespace sv
} // namespace game