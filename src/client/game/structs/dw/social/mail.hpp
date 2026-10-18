#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace social {
namespace mail {
class bdMail {
public:
  // TODO
  struct VTbl {};
  VTbl *vtbl;
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
  bdNChar8 m_context[16];
};

} // namespace mail
} // namespace social
} // namespace dw
} // namespace game
