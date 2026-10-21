#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace social {
namespace profile {
class bdProfileInfo : public bdTaskResult, bdSerializable {
public:
  bdEntityID m_entityID;
};

class bdProfiles {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
};
} // namespace profile
} // namespace social
} // namespace dw
} // namespace game
