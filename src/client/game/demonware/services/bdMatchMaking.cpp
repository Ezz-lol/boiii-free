#include <memory>
#include <std_include.hpp>

#include "bdMatchMaking.hpp"
#include <steam/steam.hpp>

namespace demonware {
bdMatchMaking::bdMatchMaking() : service(21, "bdMatchMaking") {
  this->register_task(1, &bdMatchMaking::create_session);
  this->register_task(2, &bdMatchMaking::update_session);
  this->register_task(3, &bdMatchMaking::delete_session);
  this->register_task(5, &bdMatchMaking::find_sessions);
  this->register_task(10, &bdMatchMaking::get_performance);
}

void bdMatchMaking::create_session(service_server *server,
                                   [[maybe_unused]] byte_buffer *buffer) const {
  std::unique_ptr<bdSessionID> id = std::make_unique<bdSessionID>();
  id->session_id = steam::SteamUser()->GetSteamID().bits;

  service_reply reply = server->create_reply(this->task_id());
  reply.add(id);
  reply.send();
}

void bdMatchMaking::update_session(service_server *server,
                                   byte_buffer *buffer) const {
  /*MatchMakingInfo mmInfo;
  mmInfo.session_id.deserialize(buffer);
  mmInfo.deserialize(buffer);

  byte_buffer out_data;
  mmInfo.symmetric = true;
  mmInfo.serialize(&out_data);

  byte_buffer addr_buf(mmInfo.host_addr);
  bdCommonAddr addr;
  addr.deserialize(&addr_buf);*/

  service_reply reply = server->create_reply(this->task_id());
  reply.send();
}

void bdMatchMaking::delete_session(service_server *server,
                                   byte_buffer *buffer) const {
  bdSessionID id;
  id.deserialize(buffer);

  byte_buffer out_data;
  id.serialize(&out_data);

  service_reply reply = server->create_reply(this->task_id());
  reply.send();
}

void bdMatchMaking::get_performance(service_server *server,
                                    byte_buffer * /*buffer*/) const {
  std::unique_ptr<bdPerformanceValue> result =
      std::make_unique<bdPerformanceValue>();
  result->user_id = steam::SteamUser()->GetSteamID().bits;
  result->performance = 10;

  service_reply reply = server->create_reply(this->task_id());
  reply.add(result);
  reply.send();
}

void bdMatchMaking::find_sessions(service_server *server,
                                  byte_buffer * /*buffer*/) const {
  service_reply reply = server->create_reply(this->task_id());

  // for (auto& session : sessions)
  {
    // reply.add(session.second);
  }

  reply.send();
}
} // namespace demonware
