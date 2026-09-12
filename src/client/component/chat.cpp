#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include "chat.hpp"

#include <game/game.hpp>
#include <game/utils.hpp>

#include <game/impl/game/game.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "command.hpp"
#include "client_command.hpp"
#include "console_command.hpp"
#include "scheduler.hpp"
#include "sv.hpp"

#include <array>
#include <atomic>
#include <charconv>

namespace chat {
namespace {
game::EngineDependentDvar g_deadChat;
game::EngineDependentDvar sv_sayname;
std::array<std::atomic_bool, game::CLIENT_INDEX_COUNT> muted_clients{};

std::optional<game::ClientNum_t> parse_client_num(const char *value) {
  const std::string_view text(value);
  int client_num{};
  const auto [ptr, error] =
      std::from_chars(text.data(), text.data() + text.size(), client_num);
  const auto result = static_cast<game::ClientNum_t>(client_num);

  if (error != std::errc{} || ptr != text.data() + text.size() ||
      !game::valid_client_num(result)) {
    return std::nullopt;
  }

  return result;
}

bool is_muted(const game::level::gentity_s *ent) {
  if (!ent) {
    return false;
  }

  const auto client_num = static_cast<game::ClientNum_t>(ent->s.number);
  return game::valid_client_num(client_num) &&
         muted_clients[client_num].load(std::memory_order_relaxed);
}

void set_muted(const command::params &params, const bool muted) {
  const char *command_name = muted ? "muteclient" : "unmuteclient";
  if (params.size() != 2) {
    printf("Usage: %s <client number>\n", command_name);
    return;
  }

  const auto client_num = parse_client_num(params[1]);
  if (!client_num) {
    printf("Invalid client number. Expected a value from 0 to %d.\n",
           game::CLIENT_INDEX_COUNT - 1);
    return;
  }

  if (muted && !game::access_connected_client(*client_num, [](auto &) {})) {
    printf("Client %d is not connected.\n", *client_num);
    return;
  }

  muted_clients[*client_num].store(muted, std::memory_order_relaxed);
  printf("Client %d is now %s.\n", *client_num,
         muted ? "muted" : "unmuted");
}

void cmd_say_f(game::level::gentity_s *ent, const command::params_sv &params) {
  if (params.size() < 2) {
    return;
  }

  if (is_muted(ent)) {
    return;
  }

  int mode = 0;
  if (params[0] == "say_team"s) {
    mode = 1;
  }

  const auto p = params.join(1);
  game::scr::Scr_AddString(game::scr::SCRIPTINSTANCE_SERVER,
                           p.data() + 1); // Skip special char
  game::scr::Scr_Notify_Canon(ent, game::CanonHash(params[0]), 1);

  game::G_Say(ent, nullptr, mode, p.data());
}

void cmd_chat_f(game::level::gentity_s *ent, const command::params_sv &params) {
  if (is_muted(ent)) {
    return;
  }

  auto p = params.join(1);

  // Not a mistake! + 2 is necessary for the GSC script to receive only the
  // actual chat text
  game::scr::Scr_AddString(game::scr::SCRIPTINSTANCE_SERVER, p.data() + 2);
  game::scr::Scr_Notify_Canon(ent, game::CanonHash(params[0]), 1);

  utils::hook::invoke<void>(0x140298E70_g, ent, p.data());
}

uint64_t *
divert_xuid_to_client_num_stub(int, const game::ClientNum_t client_num, int) {
  thread_local uint64_t value;
  // zero xuid is invalid, so increase the clientnum to prevent 0 values
  value = static_cast<uint64_t>(client_num) + 1;
  return &value;
}

void send_chat_message(game::ClientNum_t client_num, const std::string &text) {
  game::sv::SV_GameSendServerCommand(
      client_num, game::net::SV_CMD_CAN_IGNORE,
      utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, text.data()));
}

// This function has probably a different name
void g_say_to_stub(utils::hook::assembler &a) {
  const auto no_dead_chat = a.newLabel();

  // game's code
  a.mov(rax, qword_ptr(rbx));

  a.push(rax);

  a.mov(rax, qword_ptr(reinterpret_cast<std::uintptr_t>(&g_deadChat)));
  a.mov(al, byte_ptr(rax, 0x28)); // dvar_t.current.value.enabled
  a.test(al, al);

  a.pop(rax);

  a.je(no_dead_chat);

  a.jmp(0x140299061_g);

  a.bind(no_dead_chat);
  a.cmp(dword_ptr(rax, 0x16AE0), 0x0); // game's code
  a.jmp(0x14029905B_g);
}

void cl_handle_chat(char *dest, size_t dest_size, const char *src) {
  game::I_strcpy(dest, dest_size, src);
  printf("%s\n", dest);
}

inline const char *sv_sayname_val() {
  if (sv_sayname) {
    return sv_sayname.get_cstring();
  }

  return nullptr;
}
} // namespace

const char *get_client_name(const uint64_t xuid) {
  if (xuid == 0xFFFFFFFF || xuid == 0xFFFFFFFFFFFFFFFF) {
    const char *val = sv_sayname_val();
    return val ? val : "Server";
  }

  if (xuid > 0 && xuid < 19 && !game::is_server()) {
    char buffer[256]{};
    game::cl::CL_GetClientName(game::LOCAL_CLIENT_0, static_cast<int>(xuid - 1),
                               buffer, sizeof(buffer), true);
    std::string name(buffer);
    const size_t pipe = name.find('|');
    if (pipe != std::string::npos)
      name = name.substr(0, pipe);
    return utils::string::va("%s", name.c_str());
  }

  return "Unknown Soldier";
}

class component final : public generic_component {
public:
  void post_unpack() override {
    utils::hook::call(game::select(0x141974B04, 0x14029908A),
                      divert_xuid_to_client_num_stub);

    if (game::is_server()) {
      client_command::add("say", cmd_say_f);
      client_command::add("say_team", cmd_say_f);

      client_command::add("chat", cmd_chat_f);

      // Overwrite say command
      utils::hook::jump(
          0x14052A6C0_g, +[] {
            if (!game::server_running()) {
              printf("Server is not running\n");
              return;
            }

            const command::params params{};
            const std::string text = params.join(1);

            send_chat_message(game::INVALID_CLIENT_INDEX, text);

            const char *val = sv_sayname_val();
            const char *say_prefix = val ? val : "Server";
            printf("%s: %s\n", say_prefix, text.data());
          });

      // Overwrite tell command
      utils::hook::jump(
          0x14052A7E0_g, +[] {
            if (!game::server_running()) {
              printf("Server is not running\n");
              return;
            }

            const command::params params{};
            if (params.size() < 2) {
              return;
            }

            const game::ClientNum_t client =
                static_cast<game::ClientNum_t>(atoi(params[1]));
            const std::string text = params.join(2);

            send_chat_message(client, text);
            printf("Server -> %i: %s\n", client, text.data());
          });

      // Kill say fallback
      utils::hook::set<uint8_t>(0x1402FF987_g, 0xEB);

      console_command::add_console(
          "muteclient",
          [](const command::params &params) { set_muted(params, true); });
      console_command::add_console(
          "unmuteclient",
          [](const command::params &params) { set_muted(params, false); });

      sv::on_removeclient([](game::sv::client_s *client, const char *) {
        const auto client_num = sv::get_client_num(client);
        if (game::valid_client_num(client_num)) {
          muted_clients[client_num].store(false, std::memory_order_relaxed);
        }
      });

      scheduler::once(
          [] {
            sv_sayname = game::register_dvar_string(
                "sv_sayname", "Server", game::DVAR_SERVERINFO,
                "Custom name for server chat messages");
          },
          scheduler::pipeline::main);

      g_deadChat = game::register_dvar_bool(
          "g_deadChat", false, game::DVAR_NONE,
          "Allow dead players to chat with living players");

      utils::hook::jump(0x140299051_g, utils::hook::assemble(g_say_to_stub));
    } else {
      scheduler::once(
          [] {
            sv_sayname = game::register_dvar_string(
                "sv_sayname", "", game::DVAR_SERVERINFO,
                "Custom name for server chat messages");
          },
          scheduler::pipeline::main);

      utils::hook::call(0x141DEAA0F_g, cl_handle_chat);
    }
  }
};
} // namespace chat

REGISTER_COMPONENT(chat::component)
