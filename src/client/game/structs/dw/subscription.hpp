#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace subscription {

class bdSubscription {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
};

} // namespace subscription
} // namespace dw
} // namespace game
