#pragma once

#include <cstdint>
#include "../core.hpp"

namespace game {
namespace dw {

// Unverified. Probably still correct.
enum class taskCompleteResults : int32_t {
  TASK_NOTCOMPLETE = 0x0,
  TASK_COMPLETE = 0x1,
  TASK_ERROR = 0x2,
};

// Very likely out of date
enum class RecordEventType : int32_t {
  DW_EVENT_UNKNOWN = 0x0,
  DW_EVENT_AVI = 0x1,
  DW_EVENT_SCREENSHOT = 0x2,
  DW_EVENT_MATCH_RECORD_POINTS_SPENT = 0x3,
  DW_EVENT_EMAIL = 0x5,
  DW_EVENT_MATCH_RECORD_FINISHED = 0x6,
  DW_EVENT_INVALID_GAMERTAG = 0x7,
  DW_EVENT_CRASH_DUMP = 0x8,
  DW_EVENT_EMPTY_STATS = 0x9,
  DW_EVENT_UPLOAD_FAILURE = 0xA,
  DW_EVENT_DB_READ_ERROR = 0xB,
  DW_EVENT_NO_HELI_MESH = 0x10,
  DW_EVENT_INVALID_EMBLEM = 0x11,
  DW_EVENT_UPLOAD_SP_SESSION = 0x12,
  DW_EVENT_VALID_EMBLEM = 0x13,
  DW_EVENT_POSTMATCH_EVENTS = 0x14,
  DW_EVENT_MATCH_RECORD_FINISHED_AND_UPLOADED = 0x15,
  DW_EVENT_LOG_MATCHMAKING_DATA = 0x1E,
  DW_EVENT_LOG_XSESSION_NON_SERIALIZED_CALLS = 0x20,
  DW_EVENT_SYS_ERROR = 0x29,
  DW_EVENT_UI_ERROR = 0x2A,
  DW_EVENT_SEASON_PASS_PURCHASED = 0x2B,
  DW_EVENT_THEATER_CLIP_UPLOADED = 0x2C,
  DW_UCD_ERROR = 0x2D,
  DW_EREG_ENTRY_ERROR = 0x2E,
  DW_EREG_YOUTUBE_ERROR = 0x2F,
  DW_EVENT_UI_MP_ERROR = 0x30,
  DW_EVENT_UI_ZM_ERROR = 0x31,
  DW_EVENT_UI_CP_ERROR = 0x32,
  DW_EVENT_DEDI_NOGEOINFO = 0x33,
  DW_EVENT_CONNECTION_LOG = 0x34,
  DW_EVENT_SESSION_MODIFY_FAILURE = 0x35,
  DW_EVENT_DEDICATED_PING_LOG = 0x36,
  DW_EVENT_STATS_ACCESS_ERROR = 0x37,
  DW_EVENT_ZERO_FLAG_ERROR = 0x38,
  DW_EVENT_SESSION_LOG = 0x39,
  DW_EVENT_DOUBLE_XP_FLAG_CHANGED = 0x46, // Verified correct
  DW_EVENT_DLC_FLAG_CHANGED = 0x47,
  DW_EVENT_DEMO_UPLOAD_FAILURE = 0x48,
  DW_EVENT_LEAGUE = 0x64,
  DW_EVENT_REMOTE_CONSOLE = 0xC8,
  DW_EVENT_WATCHDOG = 0xC9,
  DW_EVENT_IGS = 0xFA,
  DW_EVENT_DLC_MENUS_VIEWED = 0xFB,
  DW_EVENT_CUSTOMCLASS_ERRORLOG = 0x12C,
  DW_EVENT_IW6_PROMO_PURCHASED = 0x145,
  DW_EVENT_IW6_PROMO_ACCESSED = 0x146,
  DW_EVENT_RATE_MATCH = 0x190,
  DW_EVENT_VOTE_MTX = 0x1A9,
  DW_EVENT_SP_REMINDER_ACTION = 0x1AA,
  DW_EVENT_DSP_PROMOTION_ACTION = 0x1AB,
  DW_EVENT_CONTENT_AVAILABLE = 0x1AE,
  DW_EVENT_REPORT_OFFENSIVE_EMBLEM = 0x1AF,
  DW_EVENT_ANTICHEAT_DEBUG = 0x1B8,
  DW_EVENT_MATCH_RECORD_MP_PUBLIC = 0x1F4,
  DW_EVENT_MATCH_RECORD_CP_PUBLIC = 0x1F5,
  DW_EVENT_MATCH_RECORD_ZM_PUBLIC = 0x1F6,
  DW_EVENT_MATCH_RECORD_MP_PRIVATE = 0x1F7,
  DW_EVENT_MATCH_RECORD_CP_PRIVATE = 0x1F8,
  DW_EVENT_MATCH_RECORD_ZM_PRIVATE = 0x1F9,
  DW_EVENT_BLACKBOX = 0x372,
  DW_EVENT_STREAM_READ_LOG = 0x384,
  DW_EVENT_KVS_FIRST = 0x1388,
  DW_EVENT_KVS_LAST = 0x176F,
  DW_EVENT_COMSCORE_FIRST = 0x2710,
  DW_EVENT_COMSCORE_LAST = 0x274C,
  DW_EVENT_NUM_EVENTS = 0x274D,
};

template <typename T> struct bdReference {
  T *m_ptr;
};

#pragma pack(push, 1)
template <typename T> struct bdReferencable {
  fastcallPtr<void, T *, int64_t> releaseFunc;
  volatile bdInt m_refCount;
  uint8_t _padding[4];
};
ASSERT_SIZE(bdReferencable<void>, 0x10);
#pragma pack(pop)

#pragma pack(push, 1)
struct bdByteBuffer : bdReferencable<bdByteBuffer> {
  bdUInt m_size;
  uint8_t _padding14[4];
  bdUByte8 *m_data;
  bdUByte8 *m_readPtr;
  bdUByte8 *m_writePtr;
  bdBool m_typeChecked;
  bdBool m_typeCheckedCopy;
  bdBool m_allocatedData;
  uint8_t _padding33[5];
};
ASSERT_SIZE(bdByteBuffer, 0x38);
typedef bdReference<bdByteBuffer> bdByteBufferRef;
#pragma pack(pop)

struct bdStopwatch {
  bdUInt64 m_start;
};

#pragma pack(push, 1)
struct bdTaskResult {
  int (**_vptr$bdTaskResult)(void);
  uint8_t _padding[16];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct bdTaskResultProcessor {
  int (**_vptr$bdTaskResultProcessor)(void);
};

#pragma pack(pop)

struct bdTask : bdReferencable<bdTask> {
  enum class bdStatus : int32_t {
    BD_EMPTY = 0x0,
    BD_PENDING = 0x1,
    BD_DONE = 0x2,
    BD_FAILED = 0x3,
    BD_TIMED_OUT = 0x4,
    BD_CANCELLED = 0x5,
    BD_MAX_STATUS = 0x6,
  };
};

#pragma pack(push, 1)
struct bdRemoteTask : bdTask {
  bdStopwatch m_timer;
  bdFloat32 m_timeout;
  bdRemoteTask::bdStatus m_status;
  bdByteBufferRef m_byteResults;
  bdTaskResult *m_taskResult;
  bdTaskResult **m_taskResultList;
  bdUInt m_numResults;
  bdUInt m_maxNumResults;
  bdUInt m_totalNumResults;
  uint8_t _padding44[4];
  bdUInt64 m_transactionID;
  bdLobbyErrorCode m_errorCode;
  uint8_t _padding54[4];
  bdTaskResultProcessor *m_taskResultProcessor;
  bdUByte8 m_serviceId;
  bdUByte8 m_taskId;
  uint8_t _padding62[6];
};
ASSERT_SIZE(bdRemoteTask, 0x68);
typedef bdReference<bdRemoteTask> bdRemoteTaskRef;
#pragma pack(pop)

} // namespace dw
} // namespace game