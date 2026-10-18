#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace commerce {
namespace marketing {
class bdMarketingComms {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
  bdNChar8 m_context[16];
};
} // namespace marketing

} // namespace commerce
} // namespace dw
} // namespace game
