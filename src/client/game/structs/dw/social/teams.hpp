#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace social {
namespace teams {
class bdTeams {
public:
  // TODO
  struct VTbl {};

  VTbl *vtbl;
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
  bdNChar8 m_context[16];
};

} // namespace teams
} // namespace social
} // namespace dw
} // namespace game
