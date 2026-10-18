#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace presence {

class bdPresenceService {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
};

} // namespace presence
} // namespace dw
} // namespace game
