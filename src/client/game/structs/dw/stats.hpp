#pragma once

#include <game/structs/core.hpp>
#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace stats {

typedef bdUInt32 bdLeaderBoardID;

class bdStatsInfo : public bdTaskResult, public bdSerializable {
public:
  enum class bdWriteType : uint32_t {
    REPLACE = 0x0,
    ADD = 0x1,
    MAX = 0x2,
    MIN = 0x3,
    REPLACE_WHEN_RATING_INCREASE = 0x4,
    ADD_WHEN_RATING_INCREASE = 0x5,
    MAX_WHEN_RATING_INCREASE = 0x6,
    MIN_WHEN_RATING_INCREASE = 0x7,
  };

  bdLeaderBoardID m_leaderboardID;
  bdEntityID m_entityID;
  bdWriteType m_writeType;
  bdInt64 m_rating;
  bdUInt64 m_rank;
  bdNChar8 m_entityName[65];
  bdUInt32 m_secondsSinceUpdate;
};

class bdStats {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
};

} // namespace stats
} // namespace dw
} // namespace game
