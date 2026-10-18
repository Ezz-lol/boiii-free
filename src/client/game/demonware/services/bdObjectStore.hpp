#pragma once

#include "game/demonware/service.hpp"

namespace demonware {
class bdObjectStore final : public service {
public:
  bdObjectStore();

private:
  void getUserObjects(service_server *server, byte_buffer *buffer) const;
};
} // namespace demonware
