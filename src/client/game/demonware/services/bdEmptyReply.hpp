#pragma once

namespace demonware {
template <uint8_t Id> class bdEmptyReply final : public service {
public:
  bdEmptyReply() : service(Id, "bdEmptyReply") {}

  void exec_task(service_server *server, const std::string &data) override {
    byte_buffer buffer(data);
    uint8_t task_id{};
    buffer.read_ubyte(&task_id);
    server->create_reply(task_id).send();
  }
};
} // namespace demonware
