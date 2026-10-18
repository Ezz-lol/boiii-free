#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace storage {
class bdStorage {
public:
  // TODO
  struct VTbl {};

  VTbl *vtbl;
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
  bdNChar8 m_context[16];
};

} // namespace storage
} // namespace dw
} // namespace game
