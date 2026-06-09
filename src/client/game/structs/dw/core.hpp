#pragma once

#include <cstdint>
#include "../core.hpp"

namespace game {
namespace dw {

// Unverified. Probably still correct.
enum class taskCompleteResults : uint32_t {
  TASK_NOTCOMPLETE = 0x0,
  TASK_COMPLETE = 0x1,
  TASK_ERROR = 0x2,
};

// Very likely out of date
enum class RecordEventType : uint32_t {
  EVENT_UNKNOWN = 0x0,
  EVENT_AVI = 0x1,
  EVENT_SCREENSHOT = 0x2,
  EVENT_MATCH_RECORD_POINTS_SPENT = 0x3,
  EVENT_EMAIL = 0x5,
  EVENT_MATCH_RECORD_FINISHED = 0x6,
  EVENT_INVALID_GAMERTAG = 0x7,
  EVENT_CRASH_DUMP = 0x8,
  EVENT_EMPTY_STATS = 0x9,
  EVENT_UPLOAD_FAILURE = 0xA,
  EVENT_DB_READ_ERROR = 0xB,
  EVENT_NO_HELI_MESH = 0x10,
  EVENT_INVALID_EMBLEM = 0x11,
  EVENT_UPLOAD_SP_SESSION = 0x12,
  EVENT_VALID_EMBLEM = 0x13,
  EVENT_POSTMATCH_EVENTS = 0x14,
  EVENT_MATCH_RECORD_FINISHED_AND_UPLOADED = 0x15,
  EVENT_LOG_MATCHMAKING_DATA = 0x1E,
  EVENT_LOG_XSESSION_NON_SERIALIZED_CALLS = 0x20,
  EVENT_SYS_ERROR = 0x29,
  EVENT_UI_ERROR = 0x2A,
  EVENT_SEASON_PASS_PURCHASED = 0x2B,
  EVENT_THEATER_CLIP_UPLOADED = 0x2C,
  UCD_ERROR = 0x2D,
  EREG_ENTRY_ERROR = 0x2E,
  EREG_YOUTUBE_ERROR = 0x2F,
  EVENT_UI_MP_ERROR = 0x30,
  EVENT_UI_ZM_ERROR = 0x31,
  EVENT_UI_CP_ERROR = 0x32,
  EVENT_DEDI_NOGEOINFO = 0x33,
  EVENT_CONNECTION_LOG = 0x34,
  EVENT_SESSION_MODIFY_FAILURE = 0x35,
  EVENT_DEDICATED_PING_LOG = 0x36,
  EVENT_STATS_ACCESS_ERROR = 0x37,
  EVENT_ZERO_FLAG_ERROR = 0x38,
  EVENT_SESSION_LOG = 0x39,
  EVENT_DOUBLE_XP_FLAG_CHANGED = 0x46, // Verified correct
  EVENT_DLC_FLAG_CHANGED = 0x47,
  EVENT_DEMO_UPLOAD_FAILURE = 0x48,
  EVENT_LEAGUE = 0x64,
  EVENT_REMOTE_CONSOLE = 0xC8,
  EVENT_WATCHDOG = 0xC9,
  EVENT_IGS = 0xFA,
  EVENT_DLC_MENUS_VIEWED = 0xFB,
  EVENT_CUSTOMCLASS_ERRORLOG = 0x12C,
  EVENT_IW6_PROMO_PURCHASED = 0x145,
  EVENT_IW6_PROMO_ACCESSED = 0x146,
  EVENT_RATE_MATCH = 0x190,
  EVENT_VOTE_MTX = 0x1A9,
  EVENT_SP_REMINDER_ACTION = 0x1AA,
  EVENT_DSP_PROMOTION_ACTION = 0x1AB,
  EVENT_CONTENT_AVAILABLE = 0x1AE,
  EVENT_REPORT_OFFENSIVE_EMBLEM = 0x1AF,
  EVENT_ANTICHEAT_DEBUG = 0x1B8,
  EVENT_MATCH_RECORD_MP_PUBLIC = 0x1F4,
  EVENT_MATCH_RECORD_CP_PUBLIC = 0x1F5,
  EVENT_MATCH_RECORD_ZM_PUBLIC = 0x1F6,
  EVENT_MATCH_RECORD_MP_PRIVATE = 0x1F7,
  EVENT_MATCH_RECORD_CP_PRIVATE = 0x1F8,
  EVENT_MATCH_RECORD_ZM_PRIVATE = 0x1F9,
  EVENT_BLACKBOX = 0x372,
  EVENT_STREAM_READ_LOG = 0x384,
  EVENT_KVS_FIRST = 0x1388,
  EVENT_KVS_LAST = 0x176F,
  EVENT_COMSCORE_FIRST = 0x2710,
  EVENT_COMSCORE_LAST = 0x274C,
  COUNT = 0x274D,
};

template <typename T> class bdReference {
public:
  T *m_ptr;
};

class bdReferencable;
class bdReferencableVTbl {
public:
  thiscall_t<void, bdReferencable *, int64_t> releaseFunc;
};

#pragma pack(push, 1)
template <typename VTbl = bdReferencableVTbl> class _bdReferencable {
public:
  VTbl *vtbl;
  volatile bdInt m_refCount;
  uint8_t _padding[4];
};
class bdReferencable : public _bdReferencable<bdReferencableVTbl> {};
ASSERT_SIZE(bdReferencable, 0x10);
#pragma pack(pop)

#pragma pack(push, 1)
class bdByteBuffer : public bdReferencable {
public:
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

class bdStopwatch {
public:
  bdUInt64 m_start;
};

#pragma pack(push, 1)
class bdTaskResult {
public:
  int32_t (**_vptr$bdTaskResult)(void);
  uint8_t _padding[16];
};
#pragma pack(pop)

#pragma pack(push, 1)
class bdTaskResultProcessor {
public:
  int32_t (**_vptr$bdTaskResultProcessor)(void);
};

#pragma pack(pop)

template <typename VTbl = bdReferencableVTbl>
class bdTask : public _bdReferencable<VTbl> {
public:
  enum class bdStatus : int32_t {
    EMPTY = 0x0,
    PENDING = 0x1,
    DONE = 0x2,
    FAILED = 0x3,
    TIMED_OUT = 0x4,
    CANCELLED = 0x5,
    MAX_STATUS = 0x6,
  };
};

enum class bdLobbyErrorCode : uint32_t {
  BD_NO_ERROR = 0x0,
  TOO_MANY_TASKS = 0x1,
  NOT_CONNECTED = 0x2,
  SEND_FAILED = 0x3,
  HANDLE_TASK_FAILED = 0x4,
  START_TASK_FAILED = 0x5,
  RESULT_EXCEEDS_BUFFER_SIZE = 0x64,
  ACCESS_DENIED = 0x65,
  EXCEPTION_IN_DB = 0x66,
  MALFORMED_TASK_HEADER = 0x67,
  INVALID_ROW = 0x68,
  EMPTY_ARG_LIST = 0x69,
  PARAM_PARSE_ERROR = 0x6A,
  PARAM_MISMATCHED_TYPE = 0x6B,
  SERVICE_NOT_AVAILABLE = 0x6C,
  CONNECTION_RESET = 0x6D,
  INVALID_USER_ID = 0x6E,
  LOBBY_PROTOCOL_VERSION_FAILURE = 0x6F,
  LOBBY_INTERNAL_FAILURE = 0x70,
  LOBBY_PROTOCOL_ERROR = 0x71,
  LOBBY_FAILED_TO_DECODE_UTF8 = 0x72,
  LOBBY_ASCII_EXPECTED = 0x73,
  ASYNCHRONOUS_ERROR = 0xC8,
  STREAMING_COMPLETE = 0xC9,
  MEMBER_NO_PROPOSAL = 0x12C,
  TEAMNAME_ALREADY_EXISTS = 0x12D,
  MAX_TEAM_MEMBERSHIPS_LIMITED = 0x12E,
  MAX_TEAM_OWNERSHIPS_LIMITED = 0x12F,
  NOT_A_TEAM_MEMBER = 0x130,
  INVALID_TEAM_ID = 0x131,
  INVALID_TEAM_NAME = 0x132,
  NOT_A_TEAM_OWNER = 0x133,
  NOT_AN_ADMIN_OR_OWNER = 0x134,
  MEMBER_PROPOSAL_EXISTS = 0x135,
  MEMBER_EXISTS = 0x136,
  TEAM_FULL = 0x137,
  VULGAR_TEAM_NAME = 0x138,
  TEAM_USERID_BANNED = 0x139,
  TEAM_EMPTY = 0x13A,
  INVALID_TEAM_PROFILE_QUERY_ID = 0x13B,
  TEAMNAME_TOO_SHORT = 0x13C,
  UNIQUE_PROFILE_DATA_EXISTS_ALREADY = 0x13D,
  INVALID_LEADERBOARD_ID = 0x190,
  INVALID_STATS_SET = 0x191,
  EMPTY_STATS_SET_IGNORED = 0x193,
  NO_DIRECT_ACCESS_TO_ARBITRATED_LBS = 0x194,
  STATS_WRITE_PERMISSION_DENIED = 0x195,
  STATS_WRITE_TYPE_DATA_TYPE_MISMATCH = 0x196,
  NO_STATS_FOR_USER = 0x197,
  INVALID_ACCESS_TO_UNRANKED_LB = 0x198,
  INVALID_EXTERNAL_TITLE_ID = 0x199,
  DIFFERENT_LEADERBOARD_SCHEMAS = 0x19A,
  TOO_MANY_LEADERBOARDS_REQUESTED = 0x19B,
  ENTITLEMENTS_ERROR = 0x19C,
  ENTITLEMENTS_INVALID_TITLEID = 0x19D,
  ENTITLEMENTS_INVALID_LEADERBOARDID = 0x19E,
  ENTITLEMENTS_INVALID_GET_MODE_FOR_TITLE = 0x19F,
  ENTITLEMENTS_URL_CONNECTION_ERROR = 0x1A0,
  ENTITLEMENTS_CONFIG_ERROR = 0x1A1,
  ENTITLEMENTS_NAMED_PARENT_ERROR = 0x1A2,
  ENTITLEMENTS_NAMED_KEY_ERROR = 0x1A3,
  TOO_MANY_ENTITY_IDS_REQUESTED = 0x1A4,
  STATS_READ_FAILED = 0x1A5,
  INVALID_TITLE_ID = 0x1F4,
  MESSAGING_INVALID_MAIL_ID = 0x258,
  SELF_BLOCK_NOT_ALLOWED = 0x259,
  GLOBAL_MESSAGE_ACCESS_DENIED = 0x25A,
  GLOBAL_MESSAGES_USER_LIMIT_EXCEEDED = 0x25B,
  MESSAGING_SENDER_DOES_NOT_EXIST = 0x25C,
  AUTH_NO_ERROR = 0x2BC,
  AUTH_BAD_REQUEST = 0x2BD,
  AUTH_SERVER_CONFIG_ERROR = 0x2BE,
  AUTH_BAD_TITLE_ID = 0x2BF,
  AUTH_BAD_ACCOUNT = 0x2C0,
  AUTH_ILLEGAL_OPERATION = 0x2C1,
  AUTH_INCORRECT_LICENSE_CODE = 0x2C2,
  AUTH_CREATE_USERNAME_EXISTS = 0x2C3,
  AUTH_CREATE_USERNAME_ILLEGAL = 0x2C4,
  AUTH_CREATE_USERNAME_VULGAR = 0x2C5,
  AUTH_CREATE_MAX_ACC_EXCEEDED = 0x2C6,
  AUTH_MIGRATE_NOT_SUPPORTED = 0x2C7,
  AUTH_TITLE_DISABLED = 0x2C8,
  AUTH_ACCOUNT_EXPIRED = 0x2C9,
  AUTH_ACCOUNT_LOCKED = 0x2CA,
  AUTH_UNKNOWN_ERROR = 0x2CB,
  AUTH_INCORRECT_PASSWORD = 0x2CC,
  AUTH_IP_NOT_IN_ALLOWED_RANGE = 0x2CD,
  AUTH_WII_TOKEN_VERIFICATION_FAILED = 0x2CE,
  AUTH_WII_AUTHENTICATION_FAILED = 0x2CF,
  AUTH_IP_KEY_LIMIT_REACHED = 0x2D0,
  AUTH_INVALID_GSPID = 0x2D1,
  AUTH_INVALID_IP_RANGE_ID = 0x2D2,
  AUTH_3DS_TOKEN_VERIFICATION_FAILED = 0x2D1,
  AUTH_3DS_AUTHENTICATION_FAILED = 0x2D2,
  AUTH_STEAM_APP_ID_MISMATCH = 0x2D3,
  AUTH_ABACCOUNTS_APP_ID_MISMATCH = 0x2D4,
  AUTH_CODO_USERNAME_NOT_SET = 0x2D5,
  AUTH_WIIU_TOKEN_VERIFICATION_FAILED = 0x2D6,
  AUTH_WIIU_AUTHENTICATION_FAILED = 0x2D7,
  AUTH_CODO_USERNAME_NOT_BASE64 = 0x2D8,
  AUTH_CODO_USERNAME_NOT_UTF8 = 0x2D9,
  AUTH_TENCENT_TICKET_EXPIRED = 0x2DA,
  AUTH_PS3_SERVICE_ID_MISMATCH = 0x2DB,
  AUTH_CODOID_NOT_WHITELISTED = 0x2DC,
  AUTH_PLATFORM_TOKEN_ERROR = 0x2DD,
  AUTH_JSON_FORMAT_ERROR = 0x2DE,
  AUTH_REPLY_CONTENT_ERROR = 0x2DF,
  AUTH_THIRD_PARTY_TOKEN_EXPIRED = 0x2E0,
  AUTH_CONTINUING = 0x2E1,
  AUTH_PLATFORM_DEVICE_ID_ERROR = 0x2E4,
  NO_PROFILE_INFO_EXISTS = 0x320,
  FRIENDSHIP_NOT_REQUSTED = 0x384,
  NOT_A_FRIEND = 0x385,
  SELF_FRIENDSHIP_NOT_ALLOWED = 0x387,
  FRIENDSHIP_EXISTS = 0x388,
  PENDING_FRIENDSHIP_EXISTS = 0x389,
  USERID_BANNED = 0x38A,
  FRIENDS_FULL = 0x38C,
  FRIENDS_NO_RICH_PRESENCE = 0x38D,
  RICH_PRESENCE_TOO_LARGE = 0x38E,
  NO_FILE = 0x3E8,
  PERMISSION_DENIED = 0x3E9,
  FILESIZE_LIMIT_EXCEEDED = 0x3EA,
  FILENAME_MAX_LENGTH_EXCEEDED = 0x3EB,
  EXTERNAL_STORAGE_SERVICE_ERROR = 0x3EC,
  CHANNEL_DOES_NOT_EXIST = 0x44D,
  CHANNEL_ALREADY_SUBSCRIBED = 0x44E,
  CHANNEL_NOT_SUBSCRIBED = 0x44F,
  CHANNEL_FULL = 0x450,
  CHANNEL_SUBSCRIPTIONS_FULL = 0x451,
  CHANNEL_NO_SELF_WHISPERING = 0x452,
  CHANNEL_ADMIN_REQUIRED = 0x453,
  CHANNEL_TARGET_NOT_SUBSCRIBED = 0x454,
  CHANNEL_REQUIRES_PASSWORD = 0x455,
  CHANNEL_TARGET_IS_SELF = 0x456,
  CHANNEL_PUBLIC_BAN_NOT_ALLOWED = 0x457,
  CHANNEL_USER_BANNED = 0x458,
  CHANNEL_PUBLIC_PASSWORD_NOT_ALLOWED = 0x459,
  CHANNEL_PUBLIC_KICK_NOT_ALLOWED = 0x45A,
  CHANNEL_MUTED = 0x45B,
  EVENT_DESC_TRUNCATED = 0x4B0,
  CONTENT_UNLOCK_UNKNOWN_ERROR = 0x514,
  UNLOCK_KEY_INVALID = 0x515,
  UNLOCK_KEY_ALREADY_USED_UP = 0x516,
  SHARED_UNLOCK_LIMIT_REACHED = 0x517,
  DIFFERENT_HARDWARE_ID = 0x518,
  INVALID_CONTENT_OWNER = 0x519,
  CONTENT_UNLOCK_INVALID_USER = 0x51A,
  CONTENT_UNLOCK_INVALID_CATEGORY = 0x51B,
  KEY_ARCHIVE_INVALID_WRITE_TYPE = 0x5DC,
  KEY_ARCHIVE_EXCEEDED_MAX_IDS_PER_REQUEST = 0x5DD,
  BANDWIDTH_TEST_TRY_AGAIN = 0x712,
  BANDWIDTH_TEST_STILL_IN_PROGRESS = 0x713,
  BANDWIDTH_TEST_NOT_PROGRESS = 0x714,
  BANDWIDTH_TEST_SOCKET_ERROR = 0x715,
  INVALID_SESSION_NONCE = 0x76D,
  ARBITRATION_FAILURE = 0x76F,
  ARBITRATION_USER_NOT_REGISTERED = 0x771,
  ARBITRATION_NOT_CONFIGURED = 0x772,
  CONTENTSTREAMING_FILE_NOT_AVAILABLE = 0x7D0,
  CONTENTSTREAMING_STORAGE_SPACE_EXCEEDED = 0x7D1,
  CONTENTSTREAMING_NUM_FILES_EXCEEDED = 0x7D2,
  CONTENTSTREAMING_UPLOAD_BANDWIDTH_EXCEEDED = 0x7D3,
  CONTENTSTREAMING_FILENAME_MAX_LENGTH_EXCEEDED = 0x7D4,
  CONTENTSTREAMING_MAX_THUMB_DATA_SIZE_EXCEEDED = 0x7D5,
  CONTENTSTREAMING_DOWNLOAD_BANDWIDTH_EXCEEDED = 0x7D6,
  CONTENTSTREAMING_NOT_ENOUGH_DOWNLOAD_BUFFER_SPACE = 0x7D7,
  CONTENTSTREAMING_SERVER_NOT_CONFIGURED = 0x7D8,
  CONTENTSTREAMING_INVALID_APPLE_RECEIPT = 0x7DA,
  CONTENTSTREAMING_APPLE_STORE_NOT_AVAILABLE = 0x7DB,
  CONTENTSTREAMING_APPLE_RECEIPT_FILENAME_MISMATCH = 0x7DC,
  CONTENTSTREAMING_HTTP_ERROR = 0x7E4,
  CONTENTSTREAMING_FAILED_TO_START_HTTP = 0x7E5,
  CONTENTSTREAMING_LOCALE_INVALID = 0x7E6,
  CONTENTSTREAMING_LOCALE_MISSING = 0x7E7,
  VOTERANK_ERROR_EMPTY_RATING_SUBMISSION = 0x7EE,
  VOTERANK_ERROR_MAX_VOTES_EXCEEDED = 0x7EF,
  VOTERANK_ERROR_INVALID_RATING = 0x7F0,
  MAX_NUM_TAGS_EXCEEDED = 0x82A,
  TAGGED_COLLECTION_DOES_NOT_EXIST = 0x82B,
  EMPTY_TAG_ARRAY = 0x82C,
  INVALID_QUERY_ID = 0x834,
  NO_ENTRY_TO_UPDATE = 0x835,
  SESSION_INVITE_EXISTS = 0x836,
  INVALID_SESSION_ID = 0x837,
  ATTACHMENT_TOO_LARGE = 0x838,
  INVALID_GROUP_ID = 0xAF0,
  MAIL_INVALID_MAIL_ID_ERROR = 0xB55,
  UCD_SERVICE_ERROR = 0xC80,
  UCD_SERVICE_DISABLED = 0xC81,
  UCD_UNINTIALIZED_ERROR = 0xC82,
  UCD_ACCOUNT_ALREADY_REGISTERED = 0xC83,
  UCD_ACCOUNT_NOT_REGISTERED = 0xC84,
  UCD_AUTH_ATTEMPT_FAILED = 0xC85,
  UCD_ACCOUNT_LINKING_ERROR = 0xC86,
  UCD_ENCRYPTION_ERROR = 0xC87,
  UCD_ACCOUNT_DATA_INVALID = 0xC88,
  UCD_ACCOUNT_DATA_INVALID_FIRSTNAME = 0xC89,
  UCD_ACCOUNT_DATA_INVALID_LASTNAME = 0xC8A,
  UCD_ACCOUNT_DATA_INVALID_DOB = 0xC8B,
  UCD_ACCOUNT_DATA_INVALID_EMAIL = 0xC8C,
  UCD_ACCOUNT_DATA_INVALID_COUNTRY = 0xC8D,
  UCD_ACCOUNT_DATA_INVALID_POSTCODE = 0xC8E,
  UCD_ACCOUNT_DATA_INVALID_PASSWORD = 0xC8F,
  UCD_ACCOUNT_NAME_ALREADY_RESISTERED = 0xC94,
  UCD_ACCOUNT_EMAIL_ALREADY_RESISTERED = 0xC95,
  UCD_GUEST_ACCOUNT_AUTH_CONFLICT = 0xC96,
  TWITCH_SERVICE_ERROR = 0xC1D,
  TWITCH_ACCOUNT_ALREADY_LINKED = 0xC1E,
  TWITCH_NO_LINKED_ACCOUNT = 0xC1F,
  YOUTUBE_SERVICE_ERROR = 0xCE5,
  YOUTUBE_SERVICE_COMMUNICATION_ERROR = 0xCE6,
  YOUTUBE_USER_DENIED_AUTHORIZATION = 0xCE7,
  YOUTUBE_AUTH_MAX_TIME_EXCEEDED = 0xCE8,
  YOUTUBE_USER_UNAUTHORIZED = 0xCE9,
  YOUTUBE_UPLOAD_MAX_TIME_EXCEEDED = 0xCEA,
  YOUTUBE_DUPLICATE_UPLOAD = 0xCEB,
  YOUTUBE_FAILED_UPLOAD = 0xCEC,
  YOUTUBE_ACCOUNT_ALREADY_REGISTERED = 0xCED,
  YOUTUBE_ACCOUNT_NOT_REGISTERED = 0xCEE,
  YOUTUBE_CONTENT_SERVER_ERROR = 0xCEF,
  YOUTUBE_UPLOAD_DOES_NOT_EXIST = 0xCF0,
  YOUTUBE_NO_LINKED_ACCOUNT = 0xCF1,
  YOUTUBE_DEVELOPER_TAGS_INVALID = 0xCF2,
  TWITTER_AUTH_ATTEMPT_FAILED = 0xDAD,
  TWITTER_AUTH_TOKEN_INVALID = 0xDAE,
  TWITTER_UPDATE_LIMIT_REACHED = 0xDAF,
  TWITTER_UNAVAILABLE = 0xDB0,
  TWITTER_ERROR = 0xDB1,
  TWITTER_TIMED_OUT = 0xDB2,
  TWITTER_DISABLED_FOR_USER = 0xDB3,
  TWITTER_ACCOUNT_AMBIGUOUS = 0xDB4,
  TWITTER_MAXIMUM_ACCOUNTS_REACHED = 0xDB5,
  TWITTER_ACCOUNT_NOT_REGISTERED = 0xDB6,
  TWITTER_DUPLICATE_STATUS = 0xDB7,
  TWITTER_ACCOUNT_ALREADY_REGISTERED = 0xE1C,
  FACEBOOK_AUTH_ATTEMPT_FAILED = 0xE11,
  FACEBOOK_AUTH_TOKEN_INVALID = 0xE12,
  FACEBOOK_PHOTO_DOES_NOT_EXIST = 0xE13,
  FACEBOOK_PHOTO_INVALID = 0xE14,
  FACEBOOK_PHOTO_ALBUM_FULL = 0xE15,
  FACEBOOK_UNAVAILABLE = 0xE16,
  FACEBOOK_ERROR = 0xE17,
  FACEBOOK_TIMED_OUT = 0xE18,
  FACEBOOK_DISABLED_FOR_USER = 0xE19,
  FACEBOOK_ACCOUNT_AMBIGUOUS = 0xE1A,
  FACEBOOK_MAXIMUM_ACCOUNTS_REACHED = 0xE1B,
  FACEBOOK_INVALID_NUM_PICTURES_REQUESTED = 0xE1C,
  FACEBOOK_VIDEO_DOES_NOT_EXIST = 0xE1D,
  FACEBOOK_ACCOUNT_ALREADY_REGISTERED = 0xE1E,
  APNS_INVALID_PAYLOAD = 0xE74,
  APNS_INVALID_TOKEN_LENGTH_ERROR = 0xE76,
  MAX_CONSOLEID_LENGTH_EXCEEDED = 0xEE1,
  MAX_WHITELIST_LENGTH_EXCEEDED = 0xEE2,
  USERGROUP_NAME_ALREADY_EXISTS = 0x1770,
  INVALID_USERGROUP_ID = 0x1771,
  USER_ALREADY_IN_USERGROUP = 0x1772,
  USER_NOT_IN_USERGROUP = 0x1773,
  INVALID_USERGROUP_MEMBER_TYPE = 0x1774,
  TOO_MANY_MEMBERS_REQUESTED = 0x1775,
  USERGROUP_NAME_TOO_SHORT = 0x1776,
  RICH_PRESENCE_DATA_TOO_LARGE = 0x1A90,
  RICH_PRESENCE_TOO_MANY_USERS = 0x1A91,
  PRESENCE_DATA_TOO_LARGE = 0x283C,
  PRESENCE_TOO_MANY_USERS = 0x283D,
  USER_LOGGED_IN_OTHER_TITLE = 0x283E,
  USER_NOT_LOGGED_IN = 0x283F,
  SUBSCRIPTION_TOO_MANY_USERS = 0x1B58,
  SUBSCRIPTION_TICKET_PARSE_ERROR = 0x1B59,
  CODO_ID_INVALID_DATA = 0x1BBC,
  INVALID_MESSAGE_FORMAT = 0x1BBD,
  TLOG_TOO_MANY_MESSAGES = 0x1BBE,
  CODO_ID_NOT_IN_WHITELIST = 0x1BBF,
  TLOG_MESSAGE_TRANSFORMATION_ERROR = 0x1BC0,
  REWARDS_NOT_ENABLED = 0x1BC1,
  MARKETPLACE_ERROR = 0x1F40,
  MARKETPLACE_RESOURCE_NOT_FOUND = 0x1F41,
  MARKETPLACE_INVALID_CURRENCY = 0x1F42,
  MARKETPLACE_INVALID_PARAMETER = 0x1F43,
  MARKETPLACE_RESOURCE_CONFLICT = 0x1F44,
  MARKETPLACE_STORAGE_ERROR = 0x1F45,
  MARKETPLACE_INTEGRITY_ERROR = 0x1F46,
  MARKETPLACE_INSUFFICIENT_FUNDS_ERROR = 0x1F47,
  MARKETPLACE_MMP_SERVICE_ERROR = 0x1F48,
  MARKETPLACE_PRECONDITION_REQUIRED = 0x1F49,
  MARKETPLACE_ITEM_MULTIPLE_PURCHASE_ERROR = 0x1F4A,
  MARKETPLACE_MISSING_REQUIRED_ENTITLEMENT = 0x1F4B,
  MARKETPLACE_VALIDATION_ERROR = 0x1F4C,
  MARKETPLACE_TENCENT_PAYMENT_ERROR = 0x1F4D,
  MARKETPLACE_SKU_NOT_COUPON_ENABLED_ERROR = 0x1F4E,
  LEAGUE_INVALID_TEAM_SIZE = 0x1FA4,
  LEAGUE_INVALID_TEAM = 0x1FA5,
  LEAGUE_INVALID_SUBDIVISION = 0x1FA6,
  LEAGUE_INVALID_LEAGUE = 0x1FA7,
  LEAGUE_TOO_MANY_RESULTS_REQUESTED = 0x1FA8,
  LEAGUE_METADATA_TOO_LARGE = 0x1FA9,
  LEAGUE_TEAM_ICON_TOO_LARGE = 0x1FAA,
  LEAGUE_TEAM_NAME_TOO_LONG = 0x1FAB,
  LEAGUE_ARRAY_SIZE_MISMATCH = 0x1FAC,
  LEAGUE_SUBDIVISION_MISMATCH = 0x2008,
  LEAGUE_INVALID_WRITE_TYPE = 0x2009,
  LEAGUE_INVALID_STATS_DATA = 0x200A,
  LEAGUE_SUBDIVISION_UNRANKED = 0x200B,
  LEAGUE_CROSS_TEAM_STATS_WRITE_PREVENTED = 0x200C,
  LEAGUE_INVALID_STATS_SEASON = 0x200D,
  COMMERCE_ERROR = 0x206C,
  COMMERCE_RESOURCE_NOT_FOUND = 0x206D,
  COMMERCE_STORAGE_INVALID_PARAMETER = 0x206E,
  COMMERCE_APPLICATION_INVALID_PARAMETER = 0x206F,
  COMMERCE_RESOURCE_CONFLICT = 0x2070,
  COMMERCE_STORAGE_ERROR = 0x2071,
  COMMERCE_INTEGRITY_ERROR = 0x2072,
  COMMERCE_MMP_SERVICE_ERROR = 0x2073,
  COMMERCE_PERMISSION_DENIED = 0x2074,
  COMMERCE_INSUFFICIENT_FUNDS_ERROR = 0x2075,
  COMMERCE_UNKNOWN_CURRENCY = 0x2076,
  COMMERCE_INVALID_RECEIPT = 0x2077,
  COMMERCE_RECEIPT_USED = 0x2078,
  COMMERCE_TRANSACTION_ALREADY_APPLIED = 0x2079,
  COMMERCE_INVALID_CURRENCY_TYPE = 0x207A,
  CONNECTION_COUNTER_ERROR = 0x20D0,
  LINKED_ACCOUNTS_INVALID_CONTEXT = 0x2198,
  LINKED_ACCOUNTS_INVALID_PLATFORM = 0x2199,
  LINKED_ACCOUNTS_LINKED_ACCOUNTS_FETCH_ERROR = 0x219A,
  LINKED_ACCOUNTS_INVALID_ACCOUNT = 0x219B,
  GMSG_INVALID_CATEGORY_ID = 0x27D8,
  GMSG_CATEGORY_MEMBERSHIPS_LIMIT = 0x27D9,
  GMSG_NONMEMBER_POST_DISALLOWED = 0x27DA,
  GMSG_CATEGORY_DISALLOWS_CLIENT_TYPE = 0x27DB,
  GMSG_PAYLOAD_TOO_BIG = 0x27DC,
  GMSG_MEMBER_POST_DISALLOWED = 0x27DD,
  GMSG_OVERLOADED = 0x27DE,
  GMSG_USER_PERCATEGORY_POST_RATE_EXCEEDED = 0x27DF,
  GMSG_USER_GLOBAL_POST_RATE_EXCEEDED = 0x27E0,
  GMSG_GROUP_POST_RATE_EXCEEDED = 0x27E1,
  MAX_ERROR_CODE = 0x27E2,
};

class bdRemoteTask;
class bdRemoteTaskVTbl : public bdReferencableVTbl {
public:
  thiscall_t<bdTask<bdRemoteTaskVTbl>::bdStatus, bdRemoteTask * /*this*/>
      checkTimeout;
  thiscall_t<void, bdRemoteTask * /*this*/, bdByteBufferRef * /*byteResults*/,
             const bdNChar8 * /*file*/, const bdNChar8 * /*function*/,
             const bdNChar8 * /*line*/
             >
      deserializeResult;
  thiscall_t<void, bdRemoteTask * /*this*/, bdFloat32 /*timeout*/> start;
  thiscall_t<bool, bdRemoteTask * /*this*/, bdByteBufferRef * /*byteResults*/,
             const bdNChar8 * /*file*/, const bdNChar8 * /*function*/,
             const bdNChar8 * /*line*/>
      deserialize;
};

#pragma pack(push, 1)
class bdRemoteTask : public bdTask<bdRemoteTaskVTbl> {
public:
  bdStopwatch m_timer;
  bdFloat32 m_timeout;
  bdTask::bdStatus m_status;
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

template <typename T> class bdArray {
public:
  T *m_data;
  bdUInt m_capacity;
  bdUInt m_size;
};

enum class TaskState : uint32_t {
  INVALID = 0x0,
  INIT = 0x1,
  INPROGRESS = 0x2,
  INCALLBACK = 0x3,
  CHILDCOMPLETE = 0x4,
  CHILDFAILED = 0x5,
  COMPLETED = 0x6,
  FAILED = 0x7,
  CANCELLED = 0x8,
};

enum class TaskGroup : uint32_t {
  TASK_GROUP_UNKNOWN = 0x0,
  TASK_GROUP_XSESSION = 0x1,
};

enum class UserEvent : int32_t {
  unknown = -1,
  enetDown = 0x0,
  enetUp = 0x1,
  loggedIn = 0x2,
  loggedOut = 0x3,
  netInfoGotBandwidth = 0x4,
  netInfoGotBasic = 0x5,
  netInfoGotDetailed = 0x6,
  netInfoError = 0x7,
  netInfoDialogComplete = 0x8,
  friendsGotFriendsList = 0x9,
  friendsNoFriends = 0xA,
  friendsListReady = 0xB,
  friendsListUpdated = 0xC,
  friendsPresenceUpdated = 0xD,
  friendsGuiExited = 0xE,
  blockListReady = 0xF,
  blockListUpdated = 0x10,
  blockListGotInformation = 0x11,
  blockListNoUser = 0x12,
  presenceSet = 0x13,
  presenceSetFailed = 0x14,
  presenceGotInformation = 0x15,
  presenceGetFailed = 0x16,
  presenceGameStatusUpdated = 0x17,
  presenceGameDataUpdated = 0x18,
  presenceGameTitleInfoUpdated = 0x19,
  rankingRangeRetrieved = 0x1A,
  rankingRangeRetrievedFail = 0x1B,
  rankingFriendsRetrieved = 0x1C,
  rankingFriendsRetrievedFail = 0x1D,
  rankingFriendsRetrievedFailNoFriends = 0x1E,
  rankingMatchingBoardFound = 0x1F,
  rankingHighScore = 0x20,
  rankingServerError = 0x21,
  rankingCommunityError = 0x22,
  rankingScoreRegistered = 0x23,
  rankingScoreRegisteredFail = 0x24,
  rankingScoreRegisteredFailNotBest = 0x25,
  rankingUserRankRetrieved = 0x26,
  rankingUserRankRetrievedFail = 0x27,
  rankingUsersRanksRetrieved = 0x28,
  rankingUsersRanksRetrievedFail = 0x29,
  rankingInitFail = 0x2A,
  rankingMemoryError = 0x2B,
  rankingGameDataAborted = 0x2C,
  rankingGameDataAbortedFail = 0x2D,
  rankingGameDataObtained = 0x2E,
  rankingGameDataObtainedFail = 0x2F,
  rankingGameDataRegistered = 0x30,
  rankingGameDataRegisteredFail = 0x31,
  matchingSessionCreated = 0x32,
  matchingSessionJoined = 0x33,
  matchingSessionSearchCompleted = 0x34,
  matchingSessionError = 0x35,
  matchingSessionLeft = 0x36,
  matchingSessionModified = 0x37,
  matchingSessionUpdate = 0x38,
  matchingSessionMessageSentError = 0x39,
  matchingSessionMemberKicked = 0x3A,
  matchingSessionInformation = 0x3B,
  sessionCreateResult = 0x3C,
  sessionJoinResult = 0x3D,
  sessionError = 0x3E,
  sessionLeaveResult = 0x3F,
  sessionModified = 0x40,
  sessionUpdateResult = 0x41,
  sessionGetInfoResult = 0x42,
  sessionGetInfoListResult = 0x43,
  sessionGetSessionDataResult = 0x44,
  sessionGetChangeableSessionDataResult = 0x45,
  sessionSearchResult = 0x46,
  sessionInviteNotification = 0x47,
  sessionInviteGetInfoResult = 0x48,
  sessionInviteGetInfoListResult = 0x49,
  sessionInviteGetDataResult = 0x4A,
  sessionInvitePostInvitationResult = 0x4B,
  sessionInviteSetDataUsedResult = 0x4C,
  messageSent = 0x4D,
  messageError = 0x4E,
  messageRetrieved = 0x4F,
  messageDialogTerminated = 0x50,
  messageInGameDataReceived = 0x51,
  messageInGameDataRetrievalDone = 0x52,
  messageAttachmentReceived = 0x53,
  messageAttachmentOpened = 0x54,
  messageInviteReceived = 0x55,
  messageInviteAccepted = 0x56,
  gameCustomDataItemListResult = 0x57,
  gameCustomDataGameDataResult = 0x58,
  gameCustomDataMessageResult = 0x59,
  gameCustomDataSetUseFlagResult = 0x5A,
  gameCustomDataGameThumbnailResult = 0x5B,
  gameCustomDataNotification = 0x5C,
  activityFeedPostPlayedWithResult = 0x5D,
  activityFeedGetPlayedWithResult = 0x5E,
  activityFeedGetTitleNewsResult = 0x5F,
  activityFeedGetTitleFeedResult = 0x60,
  activityFeedGetUserNewsResult = 0x61,
  activityFeedGetUserFeedResult = 0x62,
  activityFeedGetSharedVideosResult = 0x63,
  activityFeedGetSharedScreenshotsResult = 0x64,
  activityFeedPostInGameResult = 0x65,
  activityFeedPostLikeResult = 0x66,
  activityFeedPostDislikeResult = 0x67,
  activityFeedGetWhoLikedResult = 0x68,
  commerceSessionCreated = 0x69,
  commerceSessionAborted = 0x6A,
  commerceGotCategoryInfo = 0x6B,
  commerceGotProductList = 0x6C,
  commerceGotDetailedProductInfo = 0x6D,
  commerceGotDetailedProductInfoList = 0x6E,
  commerceCategoryBrowseStarted = 0x6F,
  commerceCategoryBrowseFinished = 0x70,
  commerceProductBrowseStarted = 0x71,
  commerceProductBrowseSuccess = 0x72,
  commerceProductBrowseAborted = 0x73,
  commerceProductBrowseFinished = 0x74,
  commerceNoEntitlements = 0x75,
  commerceGotEntitlementList = 0x76,
  commerceConsumedEntitlement = 0x77,
  commerceCheckoutStarted = 0x78,
  commerceCheckoutFinished = 0x79,
  commerceJoinPlusStarted = 0x7A,
  commerceJoinPlusFinished = 0x7B,
  commerceDownloadListStarted = 0x7C,
  commerceDownloadListFinished = 0x7D,
  commerceVoucherInputStarted = 0x7E,
  commerceVoucherInputFinished = 0x7F,
  commerceInstallStarted = 0x80,
  commerceInstallFinished = 0x81,
  commerceError = 0x82,
  authGotTicket = 0x83,
  authGotCachedTicket = 0x84,
  authGotAccessToken = 0x85,
  authNewTicket = 0x86,
  authError = 0x87,
  tssGotData = 0x88,
  tssGotDataFromSlot = 0x89,
  tssGotDataStatus = 0x8A,
  tssNoData = 0x8B,
  tssError = 0x8C,
  tusDataSet = 0x8D,
  tusDataReceived = 0x8E,
  tusVariablesSet = 0x8F,
  tusVariablesReceived = 0x90,
  tusAddedAndGotVariable = 0x91,
  tusDataDeleted = 0x92,
  tusError = 0x93,
  snsMessagePosted = 0x94,
  snsDialogStarted = 0x95,
  snsDialogFinished = 0x96,
  snsUserPermissionNotGiven = 0x97,
  snsError = 0x98,
  profileGotOnlineId = 0x99,
  profileGotNpId = 0x9A,
  profileGotOnlineName = 0x9B,
  profileGotAvatarUrl = 0x9C,
  profileGotMyLanguages = 0x9D,
  profileGotCachedUserInfo = 0x9E,
  profileGotCountryInfo = 0x9F,
  profileGotParentalInfo = 0xA0,
  profileGotPlatform = 0xA1,
  profileError = 0xA2,
  profileGotNpUserInformation = 0xA3,
  profileGotNpUsersInformation = 0xA4,
  profileGotPersonalDetailsAvailable = 0xA5,
  trophyListRetrievalSuccess = 0xA6,
  trophyListRetrievalFail = 0xA7,
  trophyPlatinumUnlocked = 0xA8,
  trophyUnlockSuccess = 0xA9,
  trophyUnlockFail = 0xAA,
  trophySetSetupSuccess = 0xAB,
  trophySetSetupFail = 0xAC,
  trophyGroupInfoRetrievalSuccess = 0xAD,
  trophyGroupInfoRetrievalFail = 0xAE,
  trophyGameInfoRetrievalSuccess = 0xAF,
  trophyGameInfoRetrievalFail = 0xB0,
  trophyProgressSuccess = 0xB1,
  trophyProgressFail = 0xB2,
  wordFilterSuccess = 0xB3,
  wordFilterFail = 0xB4,
  challengesRetrieveListResult = 0xB5,
  challengesRetrieveResponsesResult = 0xB6,
  challengesRetrieveChallengesResult = 0xB7,
  challengesSendChallengeResult = 0xB8,
  challengesNotifyChallengeResult = 0xB9,
  challengesConsumeResult = 0xBA,
  challengesInspectItemResult = 0xBB,
  challengesDataRetrieveResult = 0xBC,
  serviceTerminate = 0xBD,
  serviceError = 0xBE,
  libraryTerminated = 0xBF,
};

enum class ServiceType : uint32_t {
  core = 0x0,
  netInfo = 0x1,
  matching = 0x2,
  ranking = 0x3,
  tus = 0x4,
  tss = 0x5,
  auth = 0x6,
  friends = 0x7,
  webApi = 0x8,
  commerce = 0x9,
  profile = 0xA,
  presence = 0xB,
  trophy = 0xC,
  messaging = 0xD,
  inGameMessage = 0xE,
  sessions = 0xF,
  wordFilter = 0x10,
  gameCustomData = 0x11,
  activityFeed = 0x12,
  sns = 0x13,
  challenges = 0x14,
  size = 0x15,
};

enum class LocalTaskState : uint32_t {
  LOCAL_TASK_STATE_INPROGRESS = 0x0,
  LOCAL_TASK_STATE_COMPLETED = 0x1,
  LOCAL_TASK_STATE_FAILED = 0x2,
};

struct TaskRecord;
typedef fastcall_t<LocalTaskState, TaskRecord *> localTaskFunc;

typedef fastcall_t<bool, TaskRecord *> task_callback;

typedef uint64_t TaskCategory;

#pragma pack(push, 1)
struct TaskDefinition {
  TaskCategory category;
  const char *name;
  int32_t payloadSize;
  uint8_t _padding14[4];
  task_callback completed_callback;
  task_callback failure_callback;
  localTaskFunc localTask;
  ServiceType serviceType;
  int32_t numUserEvents;
  UserEvent userEvents[2];
};
#pragma pack(pop)

struct Event {
  ServiceType service;
  UserEvent event;
  int32_t returnCode;
};

#pragma pack(push, 1)
template <typename T> struct _TaskRecord {
  TaskRecord *next;
  const TaskDefinition *definition;
  TaskState state;
  ControllerIndex_t controllerIndex;
  uint32_t lastPoll;
  uint32_t lastPollMS;
  uint32_t id;
  TaskGroup group;
  int32_t startMS;
  int32_t timeOut;
  int32_t timeIn;
  uint8_t _padding34[4];
  bdRemoteTaskRef remoteTask;

  /*
    `eventInfo` might be PS4-specific - unsure currently.
    SCE and NP toolkit namespaces were removed from the type name
    to create a best-effort PC-equivalent definition, but it also may not be
    used at all.
  */
  Event eventInfo;
  uint8_t _padding4C[4];
  TaskRecord *nestedTask;
  T *payload;
  bool isChildTask;
  bool cancelImmediately;
  bool skipAllCallbacksAfterComplete;
  uint8_t _padding63[5];
};
struct TaskRecord : public _TaskRecord<void *> {};
ASSERT_SIZE(TaskRecord, 0x68);
#pragma pack(pop)

} // namespace dw
} // namespace game