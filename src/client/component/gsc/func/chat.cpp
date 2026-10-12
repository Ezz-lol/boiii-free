#include <std_include.hpp>

#include "func.hpp"

#include <game/impl/scr/scr.hpp>
#include <game/impl/scr/var.hpp>
#include <game/impl/sv/sv.hpp>
#include <game/utils.hpp>

#include <component/command.hpp>
#include <component/game_event.hpp>
#include <component/name.hpp>
#include <component/toast.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace func {
namespace chat {
// say: broadcast a chat message to all players
// GSC: say("Hello world");
void gscr_say(scriptInstance_t inst) {
  const char *msg = Scr_GetString(inst, 0);
  if (msg) {
    sv::SV_GameSendServerCommand(
        game::INVALID_CLIENT_INDEX, game::net::SV_CMD_CAN_IGNORE,
        utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
  }
}

namespace gscr_tell {
void send(scriptInstance_t inst, game::ClientNum_t client_num,
          uint32_t message_index) {
  const char *msg = Scr_GetString(inst, message_index);
  if (game::valid_client_num(client_num) && msg) {
    sv::SV_GameSendServerCommand(
        client_num, game::net::SV_CMD_CAN_IGNORE,
        utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
  }
}

// Method form: player tell("Hello");
void method(scriptInstance_t inst, scr_entref_t *entref) {
  send(inst, static_cast<game::ClientNum_t>(entref->u.entnum), 0);
}

// Function form: tell(client_num, "Hello");
void func(scriptInstance_t inst) {
  send(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)), 1);
}
} // namespace gscr_tell

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/chat");

  void post_unpack() override {

    register_builtin("say", gscr_say, 1);
    register_builtin("tell", gscr_tell::func, 2);
    register_builtin("tell", gscr_tell::method, 1);
  }
};
} // namespace chat
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::chat::component)
