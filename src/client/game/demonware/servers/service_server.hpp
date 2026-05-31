#pragma once

#include "../reply.hpp"

namespace demonware {
class service_server {
public:
  virtual ~service_server() = default;

  virtual remote_reply create_message(uint8_t type) {
    return remote_reply{this, type};
  }

  virtual service_reply create_reply(
      uint8_t type,
      game::dw::bdLobbyErrorCode error = game::dw::bdLobbyErrorCode::BD_NO_ERROR) {
    return service_reply{this, type, error};
  }

  virtual void send_reply(reply *data) = 0;
};
} // namespace demonware