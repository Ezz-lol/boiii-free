#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"

#include <game/game.hpp>
#include <game/utils.hpp>

#include <string>
#include <utils/hook.hpp>

namespace script {
std::string resolve_hash(uint32_t hash);
int resolve_hash_line(uint32_t hash, int num_params = -1);
std::string get_source_line(const std::string &file, int line_num);
} // namespace script

namespace patches {
namespace {
const game::dvar_t *lobby_min_players;
utils::hook::detour com_error_hook;

std::string try_resolve_hex_token(const std::string &token) {
  if (token.empty() || token.size() > 8 ||
      token.find_first_not_of("0123456789ABCDEFabcdef") != std::string::npos)
    return {};
  uint32_t hash =
      static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 16));
  return script::resolve_hash(hash);
}

std::string resolve_quoted_hashes(const std::string &input) {
  std::string result = input;
  size_t pos = 0;
  while (pos < result.size()) {
    auto q1 = result.find('"', pos);
    if (q1 == std::string::npos)
      break;
    auto q2 = result.find('"', q1 + 1);
    if (q2 == std::string::npos)
      break;

    auto token = result.substr(q1 + 1, q2 - q1 - 1);
    auto name = try_resolve_hex_token(token);
    if (!name.empty()) {
      result.replace(q1 + 1, q2 - q1 - 1, name);
      pos = q1 + 1 + name.size() + 1;
      continue;
    }
    pos = q2 + 1;
  }
  return result;
}

std::string resolve_bare_hashes(const std::string &input) {
  std::string result = input;
  size_t pos = 0;
  while (pos < result.size()) {
    if (result[pos] == '"') {
      auto close = result.find('"', pos + 1);
      pos = (close != std::string::npos) ? close + 1 : pos + 1;
      continue;
    }

    if (std::isxdigit(static_cast<unsigned char>(result[pos]))) {
      size_t start = pos;
      while (pos < result.size() &&
             std::isxdigit(static_cast<unsigned char>(result[pos])))
        pos++;
      size_t len = pos - start;

      bool preceded_by_alnum =
          (start > 0 &&
           (std::isalnum(static_cast<unsigned char>(result[start - 1])) ||
            result[start - 1] == '_'));
      bool followed_by_alnum =
          (pos < result.size() &&
           (std::isalnum(static_cast<unsigned char>(result[pos])) ||
            result[pos] == '_'));

      if (len >= 1 && len <= 8 && !preceded_by_alnum && !followed_by_alnum) {
        auto token = result.substr(start, len);
        auto name = try_resolve_hex_token(token);
        if (!name.empty()) {
          result.replace(start, len, name);
          pos = start + name.size();
          continue;
        }
      }
      continue;
    }
    pos++;
  }
  return result;
}

std::string resolve_hashes_in_string(const std::string &input) {
  return resolve_bare_hashes(resolve_quoted_hashes(input));
}

#define MS 1ms
#define SECOND 1000 * MS
#define MINUTE 60 * SECOND
#define HOUR 60 * MINUTE

void com_error_stub(const char *file, int line, int code, const char *fmt,
                    ...) {
  void *callerAddr = _ReturnAddress();
  va_list ap;
  va_start(ap, fmt);
  int32_t len = vsnprintf(nullptr, 0, fmt, ap);
  va_end(ap);
  va_start(ap, fmt);
  std::vector<char> infoBuf(len + 1);
  vsnprintf(infoBuf.data(), infoBuf.size(), fmt, ap);
  va_end(ap);
  char *msg = infoBuf.data();
  if (msg == nullptr || msg[0] == '\0') {
    msg = "No message provided!";
  }
  printf("[Com_Error] Called from 0x%p with message: \"%s\", code: %d\n",
         callerAddr, msg, code);
  game::com::Com_Printf(
      0, 0, "ComError called from 0x%p with message: \"%s\", code: %d\n",
      callerAddr, msg, code);
  static bool suppress_next_lua_error = false;
  static bool client_script_error_pending = false;

  char buffer[0x1000];

  {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf_s(buffer, _TRUNCATE, fmt, ap);
    va_end(ap);
  }

  if (game::is_server() && server_restart::restart_pending.load()) {

    return;
  }

  // Suppress cascading Lua error (code 512) after a script error
  if (suppress_next_lua_error && code == 512) {
    suppress_next_lua_error = false;
    return;
  }

  const bool is_script_error = strstr(buffer, "script error") != nullptr;
  const bool is_link_error = strstr(buffer, "linking") != nullptr ||
                             strstr(buffer, "Linking") != nullptr;
  const bool is_script_not_found =
      strstr(buffer, "Script file not found") != nullptr;

  if (is_script_error || is_link_error || is_script_not_found) {
    if ((!game::is_server() && client_script_error_pending) ||
        (game::is_server() && server_restart::restart_pending.load()))
      return;

    if (!game::is_server())
      suppress_next_lua_error = true;
    std::string resolved = resolve_hashes_in_string(buffer);
    const bool is_csc = resolved.find(".csc") != std::string::npos;
    const char *script_type = is_csc ? "CSC" : "GSC";

    std::string formatted_error;
    formatted_error.reserve(2048);

    printf("^1************* %s SCRIPT ERROR *************\n", script_type);
    formatted_error += script_type;
    formatted_error += " SCRIPT ERROR\n";

    std::istringstream stream(resolved);
    std::string err_line;
    while (std::getline(stream, err_line)) {
      if (!err_line.empty() && err_line.back() == '\r')
        err_line.pop_back();
      if (err_line.empty())
        continue;

      if (err_line.find("Unresolved external") != std::string::npos) {
        auto q1 = err_line.find('"');
        auto q2 = (q1 != std::string::npos) ? err_line.find('"', q1 + 1)
                                            : std::string::npos;
        std::string func = (q1 != std::string::npos && q2 != std::string::npos)
                               ? err_line.substr(q1 + 1, q2 - q1 - 1)
                               : "?";

        std::string params;
        auto wp = err_line.find("with ");
        auto pp = (wp != std::string::npos) ? err_line.find(" parameters", wp)
                                            : std::string::npos;
        if (wp != std::string::npos && pp != std::string::npos)
          params = err_line.substr(wp + 5, pp - wp - 5);

        auto fq1 = err_line.find("in \"");
        auto fq2 = (fq1 != std::string::npos) ? err_line.find('"', fq1 + 4)
                                              : std::string::npos;
        std::string script_file =
            (fq1 != std::string::npos && fq2 != std::string::npos)
                ? err_line.substr(fq1 + 4, fq2 - fq1 - 4)
                : "";

        uint32_t func_hash = 0;
        bool func_is_hex = !func.empty() && func.size() <= 8 &&
                           func.find_first_not_of("0123456789ABCDEFabcdef") ==
                               std::string::npos;

        if (func_is_hex) {
          // func is still a raw hex hash - parse it directly
          func_hash =
              static_cast<uint32_t>(std::strtoul(func.c_str(), nullptr, 16));
          auto resolved_name = script::resolve_hash(func_hash);
          if (!resolved_name.empty())
            func = resolved_name;
        } else {
          // func was already resolved to a name - hash it back
          uint32_t h = 0x4B9ACE2F;
          for (char c : func)
            h = (static_cast<uint32_t>(
                     std::tolower(static_cast<unsigned char>(c))) ^
                 h) *
                0x1000193;
          h *= 0x1000193;
          func_hash = h;
        }
        int num_params_int = params.empty() ? -1 : std::atoi(params.c_str());
        int src_line = script::resolve_hash_line(func_hash, num_params_int);

        printf("^1  Function:  ^5%s^1(%s)\n", func.c_str(), params.c_str());
        printf("^1  Reason:    ^1Unresolved external (function not found)\n");
        formatted_error += "Function: " + func + "(" + params + ")\n";
        formatted_error += "Reason: Unresolved external (function not found)\n";
        if (!script_file.empty()) {
          printf("^1  File:      ^5%s\n", script_file.c_str());
          formatted_error += "File: " + script_file + "\n";
        }
        if (src_line > 0) {
          printf("^1  Line:      ^2%d\n", src_line);
          formatted_error += "Line: " + std::to_string(src_line) + "\n";
          auto src = script::get_source_line(script_file, src_line);
          if (!src.empty()) {
            printf("^1  Source:    ^7%s\n", src.c_str());
            formatted_error += "Source: " + src + "\n";
          }
        }
      } else {
        printf("^1  %s\n", err_line.c_str());
        formatted_error += err_line + "\n";
      }
    }
    printf("^1*********************************************\n");

    if (game::is_server()) {
      server_restart::last_error_is_link.store(is_link_error);

      if (is_link_error) {
        server_restart::schedule("Link error detected");
        server_restart::abort_game_frame();
        return;
      } else {
        server_restart::schedule("Script error detected");
        RaiseException(server_restart::SCRIPT_ERROR_EXCEPTION, 0, 0, nullptr);
        return;
      }
    } else {
      // No script errors popups for ingame menu , just logs in console since
      // most are harmless csc erros anyway
      if (!game::com::Com_IsInGame()) {
        return;
      }

      client_script_error_pending = true;
      auto deferred_error = formatted_error;
      scheduler::once(
          [deferred_error]() {
            client_script_error_pending = false;
            if (game::com::Com_IsInGame())
              game::cbuf::Cbuf_AddText(0, "disconnect\n");
            scheduler::once(
                [deferred_error]() {
                  game::ui::UI_OpenErrorPopupWithMessage(
                      0, 0, deferred_error.c_str());
                },
                scheduler::pipeline::main, 500ms);
          },
          scheduler::pipeline::main);
      return;
    }
  } else {
    printf("[Com_Error] Code=%d, File=%s, Line=%d: %s\n", code,
           file ? file : "unknown", line, buffer);
  }

  // Suppress Clientfield Mismatch errors - convert to a recoverable ERR_DROP
  if (strstr(buffer, "Clientfield Mismatch")) {
    printf("[Com_Error] Suppressing Clientfield Mismatch error, converting to "
           "ERR_DROP\n");
    com_error_hook.invoke<void>(file, line, game::ERR_DROP,
                                "Mod compatibility issue: %s\nThis mod may "
                                "require additional patches for boiii.",
                                buffer);
    return;
  }

  if (!game::is_server() && code == game::ERR_DROP) {
    auto deferred_error = std::string(buffer);
    scheduler::once(
        [deferred_error]() {
          game::ui::UI_OpenErrorPopupWithMessage(0, 0, deferred_error.c_str());
        },
        scheduler::pipeline::main, 500ms);
    return;
  }

  // removing this will ruin stuff
  com_error_hook.invoke<void>(file, line, code, "%s", buffer);
}

void scr_get_num_expected_players() {
  auto expected_players = game::lobby::LobbyHost_GetClientCount(
      game::lobby::LOBBY_TYPE_GAME, game::lobby::LOBBY_CLIENT_TYPE_ALL);

  const auto mode = game::com::Com_SessionMode_GetMode();
  if ((mode == game::MODE_ZOMBIES || mode == game::MODE_CAMPAIGN)) {
    const auto min_players = lobby_min_players->current.value.integer;
    if (min_players > 0) {
      expected_players = min_players;
    } else if (!game::is_server()) {
      expected_players = 1;
    }
  }

  const auto num_expected_players = std::max(1, expected_players);
  game::scr::Scr_AddInt(game::scr::SCRIPTINSTANCE_SERVER, num_expected_players);
}

void sv_execute_client_messages_stub(game::net::client_s *client,
                                     game::net::msg_t *msg) {
  if ((client->reliableSequence - client->reliableAcknowledge) < 0) {
    client->reliableAcknowledge = client->reliableSequence;
    game::sv::SV_DropClient(client, "EXE_LOSTRELIABLECOMMANDS", true, true);
    return;
  }

  game::sv::SV_ExecuteClientMessage(client, msg);
}
} // namespace

struct component final : generic_component {
  void post_unpack() override {
    // Clientfield Mismatch -> recoverable ERR_DROP
    com_error_hook.create(game::com::Com_Error_, com_error_stub);

    // print hexadecimal xuids in chat game log command
    utils::hook::set<char>(game::select(0x142FD9362, 0x140E16FA2), 'x');

    // change 4 character min name limit to 3 characters
    utils::hook::set<uint8_t>(game::select(0x14224DA53, 0x140531143), 3);
    utils::hook::set<uint8_t>(game::select(0x14224DBB4, 0x1405312A8), 3);
    utils::hook::set<uint8_t>(game::select(0x14224DF8C, 0x1405316DC), 3);

    // make sure reliableAck is not negative or too big
    utils::hook::call(game::select(0x14225489C, 0x140537C4C),
                      sv_execute_client_messages_stub);

    lobby_min_players = game::register_dvar_int("lobby_min_players", 0, 0, 8,
                                                game::DVAR_NONE, "");
    utils::hook::jump(game::select(0x141A7BCF0, 0x1402CB900),
                      scr_get_num_expected_players, true);
  }
};
} // namespace patches

REGISTER_COMPONENT(patches::component)