#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace matchmaking {
class bdMatchMaking {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
  bdNChar8 m_context[16];
};
} // namespace matchmaking
} // namespace dw
} // namespace game
