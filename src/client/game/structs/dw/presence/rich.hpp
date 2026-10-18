#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace presence {
namespace rich {

class bdRichPresenceInfo : public bdTaskResult, public bdSerializable {
public:
  bdBool m_online;
  bdUByte8 m_richPresence[128];
  bdUInt m_count;
};

class bdRichPresenceService {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
};

} // namespace rich
} // namespace presence
} // namespace dw
} // namespace game
