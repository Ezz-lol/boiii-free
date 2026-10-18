#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace tencent {

class bdTencent {
public:
  bdUInt32 m_antibotSendInterval;
  bdUInt32 m_antibotLastMessageTime;
  bdNChar8 m_antibotBatchBuffer[327680];
  bdUWord m_antibotBatchTotalSize;
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
};

} // namespace tencent
} // namespace dw
} // namespace game
