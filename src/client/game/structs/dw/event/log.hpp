#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace event {
namespace log {
class bdEventLog {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
  bdBool m_filteringEnabled;
  bdBool m_receivedCategoryData;
  bdUInt m_numFilteredCategories;
  bdUInt m_filteredCategories[128];
  bdUInt64 m_numFilteredLogs;
};
} // namespace log
} // namespace event
} // namespace dw
} // namespace game
