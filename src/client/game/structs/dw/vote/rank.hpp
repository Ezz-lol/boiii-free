#pragma once

#include <game/structs/dw/stats.hpp>

namespace game {
namespace dw {
namespace vote {
namespace rank {

class bdVoteRankStatsInfo : public stats::bdStatsInfo {
  bdEntityID m_fileOwnerID;
  bdNChar8 m_fileOwnerName[65];
  bdUInt32 m_totalVotes;
  bdUInt32 m_avgVoteValue;
};

class bdVoteRank {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
};

} // namespace rank
} // namespace vote
} // namespace dw
} // namespace game
