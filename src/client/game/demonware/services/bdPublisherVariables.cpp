#include <std_include.hpp>

#include "bdPublisherVariables.hpp"

namespace demonware {
bdPublisherVariables::bdPublisherVariables()
    : service(95, "bdPublisherVariables") {
  this->register_task(1, &bdPublisherVariables::retrievePublisherVariables);
}

void bdPublisherVariables::retrievePublisherVariables(
    service_server *server, byte_buffer * /*buffer*/) const {
  // TODO:
  service_reply reply = server->create_reply(this->task_id());
  reply.send();
}
} // namespace demonware
