#pragma once

#include "game/demonware/service.hpp"

namespace demonware {
class bdTitleUtilities final : public service {
public:
  bdTitleUtilities();

private:
  void get_server_time(service_server *server, byte_buffer *buffer) const;
};
} // namespace demonware
