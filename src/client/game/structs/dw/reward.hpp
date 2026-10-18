#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace reward {
class bdReward {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
  bdNChar8 m_context[16];
};

} // namespace reward
} // namespace dw
} // namespace game
