#pragma once

#include <game/structs/dw/core.hpp>
#include <game/structs/dw/hash.hpp>
#include <game/structs/dw/lobby/core.hpp>

namespace game {
namespace dw {
namespace task {
namespace manage {
class bdRemoteTaskManager {
public:
  // TODO
  struct VTbl {};

  VTbl *vtbl;
  bdLinkedList<bdRemoteTaskRef> m_tasks;
  hash::bdHashMap<bdUInt64, bdRemoteTaskRef> m_asyncTasks;
  hash::bdHashMap<bdUInt64, bdByteBufferRef> m_asyncResults;
  lobby::bdLobbyConnectionRef m_lobbyConnection;
  bdBool m_encryptedConnection;
  bdUInt64 m_connectionID;
};
} // namespace manage
} // namespace task
} // namespace dw
} // namespace game
