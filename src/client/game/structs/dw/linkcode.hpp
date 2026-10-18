#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace linkcode {
class bdLinkCode {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
  bdNChar8 m_context[16];
};
} // namespace linkcode
} // namespace dw
} // namespace game
