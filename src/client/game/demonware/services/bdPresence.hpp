#pragma once

#include "game/demonware/service.hpp"

namespace demonware {
class bdPresence final : public service {
public:
  bdPresence();

private:
  void unk1(service_server *server, byte_buffer *buffer) const;
  void unk3(service_server *server, byte_buffer *buffer) const;
};
} // namespace demonware
