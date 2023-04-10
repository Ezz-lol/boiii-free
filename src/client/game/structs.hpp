#pragma once

#define PROTOCOL 2

#ifdef __cplusplus
namespace game
{
#endif

	enum ControllerIndex_t
	{
		INVALID_CONTROLLER_PORT = 0xFFFFFFFF,
		CONTROLLER_INDEX_FIRST = 0x0,
		CONTROLLER_INDEX_0 = 0x0,
		CONTROLLER_INDEX_1 = 0x1,
		CONTROLLER_INDEX_2 = 0x2,
		CONTROLLER_INDEX_3 = 0x3,
		CONTROLLER_INDEX_COUNT = 0x4,
	};

	enum eGameModes
	{
		MODE_GAME_MATCHMAKING_PLAYLIST = 0x0,
		MODE_GAME_MATCHMAKING_MANUAL = 0x1,
		MODE_GAME_DEFAULT = 0x2,
		MODE_GAME_LEAGUE = 0x3,
		MODE_GAME_THEATER = 0x4,
		MODE_GAME_COUNT = 0x5,
		MODE_GAME_INVALID = 0x5,
	};

	enum eModes
	{
		MODE_ZOMBIES = 0x0,
		MODE_MULTIPLAYER = 0x1,
		MODE_CAMPAIGN = 0x2,
		MODE_COUNT = 0x3,
		MODE_INVALID = 0x3,
		MODE_FIRST = 0x0,
	};

	enum eNetworkModes
	{
		MODE_NETWORK_OFFLINE = 0x0,
		MODE_NETWORK_SYSTEMLINK = 0x1,
		MODE_NETWORK_ONLINE = 0x2,
		MODE_NETWORK_COUNT = 0x3,
		MODE_NETWORK_INVALID = 0x3,
	};

	enum MapPreload
	{
		MAP_PRELOAD_NONE = 0x0,
		MAP_PRELOAD_FRONTEND = 0x1,
		MAP_PRELOAD_IN_GAME = 0x2,
	};

	enum bdLobbyErrorCode
	{
		BD_NO_ERROR = 0x0,
		BD_TOO_MANY_TASKS = 0x1,
		BD_NOT_CONNECTED = 0x2,
		BD_SEND_FAILED = 0x3,
		BD_HANDLE_TASK_FAILED = 0x4,
		BD_START_TASK_FAILED = 0x5,
		BD_RESULT_EXCEEDS_BUFFER_SIZE = 0x64,
		BD_ACCESS_DENIED = 0x65,
		BD_EXCEPTION_IN_DB = 0x66,
		BD_MALFORMED_TASK_HEADER = 0x67,
		BD_INVALID_ROW = 0x68,
		BD_EMPTY_ARG_LIST = 0x69,
		BD_PARAM_PARSE_ERROR = 0x6A,
		BD_PARAM_MISMATCHED_TYPE = 0x6B,
		BD_SERVICE_NOT_AVAILABLE = 0x6C,
		BD_CONNECTION_RESET = 0x6D,
		BD_INVALID_USER_ID = 0x6E,
		BD_LOBBY_PROTOCOL_VERSION_FAILURE = 0x6F,
		BD_LOBBY_INTERNAL_FAILURE = 0x70,
		BD_LOBBY_PROTOCOL_ERROR = 0x71,
		BD_LOBBY_FAILED_TO_DECODE_UTF8 = 0x72,
		BD_LOBBY_ASCII_EXPECTED = 0x73,
		BD_ASYNCHRONOUS_ERROR = 0xC8,
		BD_STREAMING_COMPLETE = 0xC9,
		BD_MEMBER_NO_PROPOSAL = 0x12C,
		BD_TEAMNAME_ALREADY_EXISTS = 0x12D,
		BD_MAX_TEAM_MEMBERSHIPS_LIMITED = 0x12E,
		BD_MAX_TEAM_OWNERSHIPS_LIMITED = 0x12F,
		BD_NOT_A_TEAM_MEMBER = 0x130,
		BD_INVALID_TEAM_ID = 0x131,
		BD_INVALID_TEAM_NAME = 0x132,
		BD_NOT_A_TEAM_OWNER = 0x133,
		BD_NOT_AN_ADMIN_OR_OWNER = 0x134,
		BD_MEMBER_PROPOSAL_EXISTS = 0x135,
		BD_MEMBER_EXISTS = 0x136,
		BD_TEAM_FULL = 0x137,
		BD_VULGAR_TEAM_NAME = 0x138,
		BD_TEAM_USERID_BANNED = 0x139,
		BD_TEAM_EMPTY = 0x13A,
		BD_INVALID_TEAM_PROFILE_QUERY_ID = 0x13B,
		BD_TEAMNAME_TOO_SHORT = 0x13C,
		BD_UNIQUE_PROFILE_DATA_EXISTS_ALREADY = 0x13D,
		BD_INVALID_LEADERBOARD_ID = 0x190,
		BD_INVALID_STATS_SET = 0x191,
		BD_EMPTY_STATS_SET_IGNORED = 0x193,
		BD_NO_DIRECT_ACCESS_TO_ARBITRATED_LBS = 0x194,
		BD_STATS_WRITE_PERMISSION_DENIED = 0x195,
		BD_STATS_WRITE_TYPE_DATA_TYPE_MISMATCH = 0x196,
		BD_NO_STATS_FOR_USER = 0x197,
		BD_INVALID_ACCESS_TO_UNRANKED_LB = 0x198,
		BD_INVALID_EXTERNAL_TITLE_ID = 0x199,
		BD_DIFFERENT_LEADERBOARD_SCHEMAS = 0x19A,
		BD_TOO_MANY_LEADERBOARDS_REQUESTED = 0x19B,
		BD_ENTITLEMENTS_ERROR = 0x19C,
		BD_ENTITLEMENTS_INVALID_TITLEID = 0x19D,
		BD_ENTITLEMENTS_INVALID_LEADERBOARDID = 0x19E,
		BD_ENTITLEMENTS_INVALID_GET_MODE_FOR_TITLE = 0x19F,
		BD_ENTITLEMENTS_URL_CONNECTION_ERROR = 0x1A0,
		BD_ENTITLEMENTS_CONFIG_ERROR = 0x1A1,
		BD_ENTITLEMENTS_NAMED_PARENT_ERROR = 0x1A2,
		BD_ENTITLEMENTS_NAMED_KEY_ERROR = 0x1A3,
		BD_TOO_MANY_ENTITY_IDS_REQUESTED = 0x1A4,
		BD_STATS_READ_FAILED = 0x1A5,
		BD_INVALID_TITLE_ID = 0x1F4,
		BD_MESSAGING_INVALID_MAIL_ID = 0x258,
		BD_SELF_BLOCK_NOT_ALLOWED = 0x259,
		BD_GLOBAL_MESSAGE_ACCESS_DENIED = 0x25A,
		BD_GLOBAL_MESSAGES_USER_LIMIT_EXCEEDED = 0x25B,
		BD_MESSAGING_SENDER_DOES_NOT_EXIST = 0x25C,
		BD_AUTH_NO_ERROR = 0x2BC,
		BD_AUTH_BAD_REQUEST = 0x2BD,
		BD_AUTH_SERVER_CONFIG_ERROR = 0x2BE,
		BD_AUTH_BAD_TITLE_ID = 0x2BF,
		BD_AUTH_BAD_ACCOUNT = 0x2C0,
		BD_AUTH_ILLEGAL_OPERATION = 0x2C1,
		BD_AUTH_INCORRECT_LICENSE_CODE = 0x2C2,
		BD_AUTH_CREATE_USERNAME_EXISTS = 0x2C3,
		BD_AUTH_CREATE_USERNAME_ILLEGAL = 0x2C4,
		BD_AUTH_CREATE_USERNAME_VULGAR = 0x2C5,
		BD_AUTH_CREATE_MAX_ACC_EXCEEDED = 0x2C6,
		BD_AUTH_MIGRATE_NOT_SUPPORTED = 0x2C7,
		BD_AUTH_TITLE_DISABLED = 0x2C8,
		BD_AUTH_ACCOUNT_EXPIRED = 0x2C9,
		BD_AUTH_ACCOUNT_LOCKED = 0x2CA,
		BD_AUTH_UNKNOWN_ERROR = 0x2CB,
		BD_AUTH_INCORRECT_PASSWORD = 0x2CC,
		BD_AUTH_IP_NOT_IN_ALLOWED_RANGE = 0x2CD,
		BD_AUTH_WII_TOKEN_VERIFICATION_FAILED = 0x2CE,
		BD_AUTH_WII_AUTHENTICATION_FAILED = 0x2CF,
		BD_AUTH_IP_KEY_LIMIT_REACHED = 0x2D0,
		BD_AUTH_INVALID_GSPID = 0x2D1,
		BD_AUTH_INVALID_IP_RANGE_ID = 0x2D2,
		BD_AUTH_3DS_TOKEN_VERIFICATION_FAILED = 0x2D1,
		BD_AUTH_3DS_AUTHENTICATION_FAILED = 0x2D2,
		BD_AUTH_STEAM_APP_ID_MISMATCH = 0x2D3,
		BD_AUTH_ABACCOUNTS_APP_ID_MISMATCH = 0x2D4,
		BD_AUTH_CODO_USERNAME_NOT_SET = 0x2D5,
		BD_AUTH_WIIU_TOKEN_VERIFICATION_FAILED = 0x2D6,
		BD_AUTH_WIIU_AUTHENTICATION_FAILED = 0x2D7,
		BD_AUTH_CODO_USERNAME_NOT_BASE64 = 0x2D8,
		BD_AUTH_CODO_USERNAME_NOT_UTF8 = 0x2D9,
		BD_AUTH_TENCENT_TICKET_EXPIRED = 0x2DA,
		BD_AUTH_PS3_SERVICE_ID_MISMATCH = 0x2DB,
		BD_AUTH_CODOID_NOT_WHITELISTED = 0x2DC,
		BD_AUTH_PLATFORM_TOKEN_ERROR = 0x2DD,
		BD_AUTH_JSON_FORMAT_ERROR = 0x2DE,
		BD_AUTH_REPLY_CONTENT_ERROR = 0x2DF,
		BD_AUTH_THIRD_PARTY_TOKEN_EXPIRED = 0x2E0,
		BD_AUTH_CONTINUING = 0x2E1,
		BD_AUTH_PLATFORM_DEVICE_ID_ERROR = 0x2E4,
		BD_NO_PROFILE_INFO_EXISTS = 0x320,
		BD_FRIENDSHIP_NOT_REQUSTED = 0x384,
		BD_NOT_A_FRIEND = 0x385,
		BD_SELF_FRIENDSHIP_NOT_ALLOWED = 0x387,
		BD_FRIENDSHIP_EXISTS = 0x388,
		BD_PENDING_FRIENDSHIP_EXISTS = 0x389,
		BD_USERID_BANNED = 0x38A,
		BD_FRIENDS_FULL = 0x38C,
		BD_FRIENDS_NO_RICH_PRESENCE = 0x38D,
		BD_RICH_PRESENCE_TOO_LARGE = 0x38E,
		BD_NO_FILE = 0x3E8,
		BD_PERMISSION_DENIED = 0x3E9,
		BD_FILESIZE_LIMIT_EXCEEDED = 0x3EA,
		BD_FILENAME_MAX_LENGTH_EXCEEDED = 0x3EB,
		BD_EXTERNAL_STORAGE_SERVICE_ERROR = 0x3EC,
		BD_CHANNEL_DOES_NOT_EXIST = 0x44D,
		BD_CHANNEL_ALREADY_SUBSCRIBED = 0x44E,
		BD_CHANNEL_NOT_SUBSCRIBED = 0x44F,
		BD_CHANNEL_FULL = 0x450,
		BD_CHANNEL_SUBSCRIPTIONS_FULL = 0x451,
		BD_CHANNEL_NO_SELF_WHISPERING = 0x452,
		BD_CHANNEL_ADMIN_REQUIRED = 0x453,
		BD_CHANNEL_TARGET_NOT_SUBSCRIBED = 0x454,
		BD_CHANNEL_REQUIRES_PASSWORD = 0x455,
		BD_CHANNEL_TARGET_IS_SELF = 0x456,
		BD_CHANNEL_PUBLIC_BAN_NOT_ALLOWED = 0x457,
		BD_CHANNEL_USER_BANNED = 0x458,
		BD_CHANNEL_PUBLIC_PASSWORD_NOT_ALLOWED = 0x459,
		BD_CHANNEL_PUBLIC_KICK_NOT_ALLOWED = 0x45A,
		BD_CHANNEL_MUTED = 0x45B,
		BD_EVENT_DESC_TRUNCATED = 0x4B0,
		BD_CONTENT_UNLOCK_UNKNOWN_ERROR = 0x514,
		BD_UNLOCK_KEY_INVALID = 0x515,
		BD_UNLOCK_KEY_ALREADY_USED_UP = 0x516,
		BD_SHARED_UNLOCK_LIMIT_REACHED = 0x517,
		BD_DIFFERENT_HARDWARE_ID = 0x518,
		BD_INVALID_CONTENT_OWNER = 0x519,
		BD_CONTENT_UNLOCK_INVALID_USER = 0x51A,
		BD_CONTENT_UNLOCK_INVALID_CATEGORY = 0x51B,
		BD_KEY_ARCHIVE_INVALID_WRITE_TYPE = 0x5DC,
		BD_KEY_ARCHIVE_EXCEEDED_MAX_IDS_PER_REQUEST = 0x5DD,
		BD_BANDWIDTH_TEST_TRY_AGAIN = 0x712,
		BD_BANDWIDTH_TEST_STILL_IN_PROGRESS = 0x713,
		BD_BANDWIDTH_TEST_NOT_PROGRESS = 0x714,
		BD_BANDWIDTH_TEST_SOCKET_ERROR = 0x715,
		BD_INVALID_SESSION_NONCE = 0x76D,
		BD_ARBITRATION_FAILURE = 0x76F,
		BD_ARBITRATION_USER_NOT_REGISTERED = 0x771,
		BD_ARBITRATION_NOT_CONFIGURED = 0x772,
		BD_CONTENTSTREAMING_FILE_NOT_AVAILABLE = 0x7D0,
		BD_CONTENTSTREAMING_STORAGE_SPACE_EXCEEDED = 0x7D1,
		BD_CONTENTSTREAMING_NUM_FILES_EXCEEDED = 0x7D2,
		BD_CONTENTSTREAMING_UPLOAD_BANDWIDTH_EXCEEDED = 0x7D3,
		BD_CONTENTSTREAMING_FILENAME_MAX_LENGTH_EXCEEDED = 0x7D4,
		BD_CONTENTSTREAMING_MAX_THUMB_DATA_SIZE_EXCEEDED = 0x7D5,
		BD_CONTENTSTREAMING_DOWNLOAD_BANDWIDTH_EXCEEDED = 0x7D6,
		BD_CONTENTSTREAMING_NOT_ENOUGH_DOWNLOAD_BUFFER_SPACE = 0x7D7,
		BD_CONTENTSTREAMING_SERVER_NOT_CONFIGURED = 0x7D8,
		BD_CONTENTSTREAMING_INVALID_APPLE_RECEIPT = 0x7DA,
		BD_CONTENTSTREAMING_APPLE_STORE_NOT_AVAILABLE = 0x7DB,
		BD_CONTENTSTREAMING_APPLE_RECEIPT_FILENAME_MISMATCH = 0x7DC,
		BD_CONTENTSTREAMING_HTTP_ERROR = 0x7E4,
		BD_CONTENTSTREAMING_FAILED_TO_START_HTTP = 0x7E5,
		BD_CONTENTSTREAMING_LOCALE_INVALID = 0x7E6,
		BD_CONTENTSTREAMING_LOCALE_MISSING = 0x7E7,
		BD_VOTERANK_ERROR_EMPTY_RATING_SUBMISSION = 0x7EE,
		BD_VOTERANK_ERROR_MAX_VOTES_EXCEEDED = 0x7EF,
		BD_VOTERANK_ERROR_INVALID_RATING = 0x7F0,
		BD_MAX_NUM_TAGS_EXCEEDED = 0x82A,
		BD_TAGGED_COLLECTION_DOES_NOT_EXIST = 0x82B,
		BD_EMPTY_TAG_ARRAY = 0x82C,
		BD_INVALID_QUERY_ID = 0x834,
		BD_NO_ENTRY_TO_UPDATE = 0x835,
		BD_SESSION_INVITE_EXISTS = 0x836,
		BD_INVALID_SESSION_ID = 0x837,
		BD_ATTACHMENT_TOO_LARGE = 0x838,
		BD_INVALID_GROUP_ID = 0xAF0,
		BD_MAIL_INVALID_MAIL_ID_ERROR = 0xB55,
		BD_UCD_SERVICE_ERROR = 0xC80,
		BD_UCD_SERVICE_DISABLED = 0xC81,
		BD_UCD_UNINTIALIZED_ERROR = 0xC82,
		BD_UCD_ACCOUNT_ALREADY_REGISTERED = 0xC83,
		BD_UCD_ACCOUNT_NOT_REGISTERED = 0xC84,
		BD_UCD_AUTH_ATTEMPT_FAILED = 0xC85,
		BD_UCD_ACCOUNT_LINKING_ERROR = 0xC86,
		BD_UCD_ENCRYPTION_ERROR = 0xC87,
		BD_UCD_ACCOUNT_DATA_INVALID = 0xC88,
		BD_UCD_ACCOUNT_DATA_INVALID_FIRSTNAME = 0xC89,
		BD_UCD_ACCOUNT_DATA_INVALID_LASTNAME = 0xC8A,
		BD_UCD_ACCOUNT_DATA_INVALID_DOB = 0xC8B,
		BD_UCD_ACCOUNT_DATA_INVALID_EMAIL = 0xC8C,
		BD_UCD_ACCOUNT_DATA_INVALID_COUNTRY = 0xC8D,
		BD_UCD_ACCOUNT_DATA_INVALID_POSTCODE = 0xC8E,
		BD_UCD_ACCOUNT_DATA_INVALID_PASSWORD = 0xC8F,
		BD_UCD_ACCOUNT_NAME_ALREADY_RESISTERED = 0xC94,
		BD_UCD_ACCOUNT_EMAIL_ALREADY_RESISTERED = 0xC95,
		BD_UCD_GUEST_ACCOUNT_AUTH_CONFLICT = 0xC96,
		BD_TWITCH_SERVICE_ERROR = 0xC1D,
		BD_TWITCH_ACCOUNT_ALREADY_LINKED = 0xC1E,
		BD_TWITCH_NO_LINKED_ACCOUNT = 0xC1F,
		BD_YOUTUBE_SERVICE_ERROR = 0xCE5,
		BD_YOUTUBE_SERVICE_COMMUNICATION_ERROR = 0xCE6,
		BD_YOUTUBE_USER_DENIED_AUTHORIZATION = 0xCE7,
		BD_YOUTUBE_AUTH_MAX_TIME_EXCEEDED = 0xCE8,
		BD_YOUTUBE_USER_UNAUTHORIZED = 0xCE9,
		BD_YOUTUBE_UPLOAD_MAX_TIME_EXCEEDED = 0xCEA,
		BD_YOUTUBE_DUPLICATE_UPLOAD = 0xCEB,
		BD_YOUTUBE_FAILED_UPLOAD = 0xCEC,
		BD_YOUTUBE_ACCOUNT_ALREADY_REGISTERED = 0xCED,
		BD_YOUTUBE_ACCOUNT_NOT_REGISTERED = 0xCEE,
		BD_YOUTUBE_CONTENT_SERVER_ERROR = 0xCEF,
		BD_YOUTUBE_UPLOAD_DOES_NOT_EXIST = 0xCF0,
		BD_YOUTUBE_NO_LINKED_ACCOUNT = 0xCF1,
		BD_YOUTUBE_DEVELOPER_TAGS_INVALID = 0xCF2,
		BD_TWITTER_AUTH_ATTEMPT_FAILED = 0xDAD,
		BD_TWITTER_AUTH_TOKEN_INVALID = 0xDAE,
		BD_TWITTER_UPDATE_LIMIT_REACHED = 0xDAF,
		BD_TWITTER_UNAVAILABLE = 0xDB0,
		BD_TWITTER_ERROR = 0xDB1,
		BD_TWITTER_TIMED_OUT = 0xDB2,
		BD_TWITTER_DISABLED_FOR_USER = 0xDB3,
		BD_TWITTER_ACCOUNT_AMBIGUOUS = 0xDB4,
		BD_TWITTER_MAXIMUM_ACCOUNTS_REACHED = 0xDB5,
		BD_TWITTER_ACCOUNT_NOT_REGISTERED = 0xDB6,
		BD_TWITTER_DUPLICATE_STATUS = 0xDB7,
		BD_TWITTER_ACCOUNT_ALREADY_REGISTERED = 0xE1C,
		BD_FACEBOOK_AUTH_ATTEMPT_FAILED = 0xE11,
		BD_FACEBOOK_AUTH_TOKEN_INVALID = 0xE12,
		BD_FACEBOOK_PHOTO_DOES_NOT_EXIST = 0xE13,
		BD_FACEBOOK_PHOTO_INVALID = 0xE14,
		BD_FACEBOOK_PHOTO_ALBUM_FULL = 0xE15,
		BD_FACEBOOK_UNAVAILABLE = 0xE16,
		BD_FACEBOOK_ERROR = 0xE17,
		BD_FACEBOOK_TIMED_OUT = 0xE18,
		BD_FACEBOOK_DISABLED_FOR_USER = 0xE19,
		BD_FACEBOOK_ACCOUNT_AMBIGUOUS = 0xE1A,
		BD_FACEBOOK_MAXIMUM_ACCOUNTS_REACHED = 0xE1B,
		BD_FACEBOOK_INVALID_NUM_PICTURES_REQUESTED = 0xE1C,
		BD_FACEBOOK_VIDEO_DOES_NOT_EXIST = 0xE1D,
		BD_FACEBOOK_ACCOUNT_ALREADY_REGISTERED = 0xE1E,
		BD_APNS_INVALID_PAYLOAD = 0xE74,
		BD_APNS_INVALID_TOKEN_LENGTH_ERROR = 0xE76,
		BD_MAX_CONSOLEID_LENGTH_EXCEEDED = 0xEE1,
		BD_MAX_WHITELIST_LENGTH_EXCEEDED = 0xEE2,
		BD_USERGROUP_NAME_ALREADY_EXISTS = 0x1770,
		BD_INVALID_USERGROUP_ID = 0x1771,
		BD_USER_ALREADY_IN_USERGROUP = 0x1772,
		BD_USER_NOT_IN_USERGROUP = 0x1773,
		BD_INVALID_USERGROUP_MEMBER_TYPE = 0x1774,
		BD_TOO_MANY_MEMBERS_REQUESTED = 0x1775,
		BD_USERGROUP_NAME_TOO_SHORT = 0x1776,
		BD_RICH_PRESENCE_DATA_TOO_LARGE = 0x1A90,
		BD_RICH_PRESENCE_TOO_MANY_USERS = 0x1A91,
		BD_PRESENCE_DATA_TOO_LARGE = 0x283C,
		BD_PRESENCE_TOO_MANY_USERS = 0x283D,
		BD_USER_LOGGED_IN_OTHER_TITLE = 0x283E,
		BD_USER_NOT_LOGGED_IN = 0x283F,
		BD_SUBSCRIPTION_TOO_MANY_USERS = 0x1B58,
		BD_SUBSCRIPTION_TICKET_PARSE_ERROR = 0x1B59,
		BD_CODO_ID_INVALID_DATA = 0x1BBC,
		BD_INVALID_MESSAGE_FORMAT = 0x1BBD,
		BD_TLOG_TOO_MANY_MESSAGES = 0x1BBE,
		BD_CODO_ID_NOT_IN_WHITELIST = 0x1BBF,
		BD_TLOG_MESSAGE_TRANSFORMATION_ERROR = 0x1BC0,
		BD_REWARDS_NOT_ENABLED = 0x1BC1,
		BD_MARKETPLACE_ERROR = 0x1F40,
		BD_MARKETPLACE_RESOURCE_NOT_FOUND = 0x1F41,
		BD_MARKETPLACE_INVALID_CURRENCY = 0x1F42,
		BD_MARKETPLACE_INVALID_PARAMETER = 0x1F43,
		BD_MARKETPLACE_RESOURCE_CONFLICT = 0x1F44,
		BD_MARKETPLACE_STORAGE_ERROR = 0x1F45,
		BD_MARKETPLACE_INTEGRITY_ERROR = 0x1F46,
		BD_MARKETPLACE_INSUFFICIENT_FUNDS_ERROR = 0x1F47,
		BD_MARKETPLACE_MMP_SERVICE_ERROR = 0x1F48,
		BD_MARKETPLACE_PRECONDITION_REQUIRED = 0x1F49,
		BD_MARKETPLACE_ITEM_MULTIPLE_PURCHASE_ERROR = 0x1F4A,
		BD_MARKETPLACE_MISSING_REQUIRED_ENTITLEMENT = 0x1F4B,
		BD_MARKETPLACE_VALIDATION_ERROR = 0x1F4C,
		BD_MARKETPLACE_TENCENT_PAYMENT_ERROR = 0x1F4D,
		BD_MARKETPLACE_SKU_NOT_COUPON_ENABLED_ERROR = 0x1F4E,
		BD_LEAGUE_INVALID_TEAM_SIZE = 0x1FA4,
		BD_LEAGUE_INVALID_TEAM = 0x1FA5,
		BD_LEAGUE_INVALID_SUBDIVISION = 0x1FA6,
		BD_LEAGUE_INVALID_LEAGUE = 0x1FA7,
		BD_LEAGUE_TOO_MANY_RESULTS_REQUESTED = 0x1FA8,
		BD_LEAGUE_METADATA_TOO_LARGE = 0x1FA9,
		BD_LEAGUE_TEAM_ICON_TOO_LARGE = 0x1FAA,
		BD_LEAGUE_TEAM_NAME_TOO_LONG = 0x1FAB,
		BD_LEAGUE_ARRAY_SIZE_MISMATCH = 0x1FAC,
		BD_LEAGUE_SUBDIVISION_MISMATCH = 0x2008,
		BD_LEAGUE_INVALID_WRITE_TYPE = 0x2009,
		BD_LEAGUE_INVALID_STATS_DATA = 0x200A,
		BD_LEAGUE_SUBDIVISION_UNRANKED = 0x200B,
		BD_LEAGUE_CROSS_TEAM_STATS_WRITE_PREVENTED = 0x200C,
		BD_LEAGUE_INVALID_STATS_SEASON = 0x200D,
		BD_COMMERCE_ERROR = 0x206C,
		BD_COMMERCE_RESOURCE_NOT_FOUND = 0x206D,
		BD_COMMERCE_STORAGE_INVALID_PARAMETER = 0x206E,
		BD_COMMERCE_APPLICATION_INVALID_PARAMETER = 0x206F,
		BD_COMMERCE_RESOURCE_CONFLICT = 0x2070,
		BD_COMMERCE_STORAGE_ERROR = 0x2071,
		BD_COMMERCE_INTEGRITY_ERROR = 0x2072,
		BD_COMMERCE_MMP_SERVICE_ERROR = 0x2073,
		BD_COMMERCE_PERMISSION_DENIED = 0x2074,
		BD_COMMERCE_INSUFFICIENT_FUNDS_ERROR = 0x2075,
		BD_COMMERCE_UNKNOWN_CURRENCY = 0x2076,
		BD_COMMERCE_INVALID_RECEIPT = 0x2077,
		BD_COMMERCE_RECEIPT_USED = 0x2078,
		BD_COMMERCE_TRANSACTION_ALREADY_APPLIED = 0x2079,
		BD_COMMERCE_INVALID_CURRENCY_TYPE = 0x207A,
		BD_CONNECTION_COUNTER_ERROR = 0x20D0,
		BD_LINKED_ACCOUNTS_INVALID_CONTEXT = 0x2198,
		BD_LINKED_ACCOUNTS_INVALID_PLATFORM = 0x2199,
		BD_LINKED_ACCOUNTS_LINKED_ACCOUNTS_FETCH_ERROR = 0x219A,
		BD_LINKED_ACCOUNTS_INVALID_ACCOUNT = 0x219B,
		BD_GMSG_INVALID_CATEGORY_ID = 0x27D8,
		BD_GMSG_CATEGORY_MEMBERSHIPS_LIMIT = 0x27D9,
		BD_GMSG_NONMEMBER_POST_DISALLOWED = 0x27DA,
		BD_GMSG_CATEGORY_DISALLOWS_CLIENT_TYPE = 0x27DB,
		BD_GMSG_PAYLOAD_TOO_BIG = 0x27DC,
		BD_GMSG_MEMBER_POST_DISALLOWED = 0x27DD,
		BD_GMSG_OVERLOADED = 0x27DE,
		BD_GMSG_USER_PERCATEGORY_POST_RATE_EXCEEDED = 0x27DF,
		BD_GMSG_USER_GLOBAL_POST_RATE_EXCEEDED = 0x27E0,
		BD_GMSG_GROUP_POST_RATE_EXCEEDED = 0x27E1,
		BD_MAX_ERROR_CODE = 0x27E2,
	};

	enum bdNATType : uint8_t
	{
		BD_NAT_UNKNOWN = 0x0,
		BD_NAT_OPEN = 0x1,
		BD_NAT_MODERATE = 0x2,
		BD_NAT_STRICT = 0x3,
	};

	enum itemTextStyle
	{
		ITEM_TEXTSTYLE_NORMAL = 0,
		ITEM_TEXTSTYLE_SHADOWED = 3,
		ITEM_TEXTSTYLE_SHADOWEDMORE = 6,
		ITEM_TEXTSTYLE_BORDERED = 7,
		ITEM_TEXTSTYLE_BORDEREDMORE = 8,
		ITEM_TEXTSTYLE_MONOSPACE = 128,
		ITEM_TEXTSTYLE_MONOSPACESHADOWED = 132,
	};

	enum errorParm
	{
		ERR_FATAL = 0,
		ERR_DROP = 1,
		ERR_SERVERDISCONNECT = 2,
		ERR_DISCONNECT = 3,
		ERR_SCRIPT = 4,
		ERR_SCRIPT_DROP = 5,
		ERR_LOCALIZATION = 6,
		ERR_MAPLOADERRORSUMMARY = 7,
	};

	enum errorCode
	{
		ERROR_NONE = 0x0,
		ERROR_FATAL = 0x1,
		ERROR_DROP = 0x2,
		ERROR_FROM_STARTUP = 0x4,
		ERROR_SERVERDISCONNECT = 0x8,
		ERROR_DISCONNECT = 0x10,
		ERROR_SCRIPT = 0x20,
		ERROR_SCRIPT_DROP = 0x40,
		ERROR_LOCALIZATION = 0x80,
		ERROR_UI = 0x100,
		ERROR_LUA = 0x200,
		ERROR_SOFTRESTART = 0x400,
		ERROR_SOFTRESTART_KEEPDW = 0x800,
	};

	enum XAssetType
	{
		ASSET_TYPE_PHYSPRESET = 0x0,
		ASSET_TYPE_PHYSCONSTRAINTS = 0x1,
		ASSET_TYPE_DESTRUCTIBLEDEF = 0x2,
		ASSET_TYPE_XANIMPARTS = 0x3,
		ASSET_TYPE_XMODEL = 0x4,
		ASSET_TYPE_XMODELMESH = 0x5,
		ASSET_TYPE_MATERIAL = 0x6,
		ASSET_TYPE_COMPUTE_SHADER_SET = 0x7,
		ASSET_TYPE_TECHNIQUE_SET = 0x8,
		ASSET_TYPE_IMAGE = 0x9,
		ASSET_TYPE_SOUND = 0xA,
		ASSET_TYPE_SOUND_PATCH = 0xB,
		ASSET_TYPE_CLIPMAP = 0xC,
		ASSET_TYPE_COMWORLD = 0xD,
		ASSET_TYPE_GAMEWORLD = 0xE,
		ASSET_TYPE_MAP_ENTS = 0xF,
		ASSET_TYPE_GFXWORLD = 0x10,
		ASSET_TYPE_LIGHT_DEF = 0x11,
		ASSET_TYPE_LENSFLARE_DEF = 0x12,
		ASSET_TYPE_UI_MAP = 0x13,
		ASSET_TYPE_FONT = 0x14,
		ASSET_TYPE_FONTICON = 0x15,
		ASSET_TYPE_LOCALIZE_ENTRY = 0x16,
		ASSET_TYPE_WEAPON = 0x17,
		ASSET_TYPE_WEAPONDEF = 0x18,
		ASSET_TYPE_WEAPON_VARIANT = 0x19,
		ASSET_TYPE_WEAPON_FULL = 0x1A,
		ASSET_TYPE_CGMEDIA = 0x1B,
		ASSET_TYPE_PLAYERSOUNDS = 0x1C,
		ASSET_TYPE_PLAYERFX = 0x1D,
		ASSET_TYPE_SHAREDWEAPONSOUNDS = 0x1E,
		ASSET_TYPE_ATTACHMENT = 0x1F,
		ASSET_TYPE_ATTACHMENT_UNIQUE = 0x20,
		ASSET_TYPE_WEAPON_CAMO = 0x21,
		ASSET_TYPE_CUSTOMIZATION_TABLE = 0x22,
		ASSET_TYPE_CUSTOMIZATION_TABLE_FE_IMAGES = 0x23,
		ASSET_TYPE_CUSTOMIZATION_TABLE_COLOR = 0x24,
		ASSET_TYPE_SNDDRIVER_GLOBALS = 0x25,
		ASSET_TYPE_FX = 0x26,
		ASSET_TYPE_TAGFX = 0x27,
		ASSET_TYPE_NEW_LENSFLARE_DEF = 0x28,
		ASSET_TYPE_IMPACT_FX = 0x29,
		ASSET_TYPE_IMPACT_SOUND = 0x2A,
		ASSET_TYPE_PLAYER_CHARACTER = 0x2B,
		ASSET_TYPE_AITYPE = 0x2C,
		ASSET_TYPE_CHARACTER = 0x2D,
		ASSET_TYPE_XMODELALIAS = 0x2E,
		ASSET_TYPE_RAWFILE = 0x2F,
		ASSET_TYPE_STRINGTABLE = 0x30,
		ASSET_TYPE_STRUCTURED_TABLE = 0x31,
		ASSET_TYPE_LEADERBOARD = 0x32,
		ASSET_TYPE_DDL = 0x33,
		ASSET_TYPE_GLASSES = 0x34,
		ASSET_TYPE_TEXTURELIST = 0x35,
		ASSET_TYPE_SCRIPTPARSETREE = 0x36,
		ASSET_TYPE_KEYVALUEPAIRS = 0x37,
		ASSET_TYPE_VEHICLEDEF = 0x38,
		ASSET_TYPE_ADDON_MAP_ENTS = 0x39,
		ASSET_TYPE_TRACER = 0x3A,
		ASSET_TYPE_SLUG = 0x3B,
		ASSET_TYPE_SURFACEFX_TABLE = 0x3C,
		ASSET_TYPE_SURFACESOUNDDEF = 0x3D,
		ASSET_TYPE_FOOTSTEP_TABLE = 0x3E,
		ASSET_TYPE_ENTITYFXIMPACTS = 0x3F,
		ASSET_TYPE_ENTITYSOUNDIMPACTS = 0x40,
		ASSET_TYPE_ZBARRIER = 0x41,
		ASSET_TYPE_VEHICLEFXDEF = 0x42,
		ASSET_TYPE_VEHICLESOUNDDEF = 0x43,
		ASSET_TYPE_TYPEINFO = 0x44,
		ASSET_TYPE_SCRIPTBUNDLE = 0x45,
		ASSET_TYPE_SCRIPTBUNDLELIST = 0x46,
		ASSET_TYPE_RUMBLE = 0x47,
		ASSET_TYPE_BULLETPENETRATION = 0x48,
		ASSET_TYPE_LOCDMGTABLE = 0x49,
		ASSET_TYPE_AIMTABLE = 0x4A,
		ASSET_TYPE_ANIMSELECTORTABLESET = 0x4B,
		ASSET_TYPE_ANIMMAPPINGTABLE = 0x4C,
		ASSET_TYPE_ANIMSTATEMACHINE = 0x4D,
		ASSET_TYPE_BEHAVIORTREE = 0x4E,
		ASSET_TYPE_BEHAVIORSTATEMACHINE = 0x4F,
		ASSET_TYPE_TTF = 0x50,
		ASSET_TYPE_SANIM = 0x51,
		ASSET_TYPE_LIGHT_DESCRIPTION = 0x52,
		ASSET_TYPE_SHELLSHOCK = 0x53,
		ASSET_TYPE_XCAM = 0x54,
		ASSET_TYPE_BG_CACHE = 0x55,
		ASSET_TYPE_TEXTURE_COMBO = 0x56,
		ASSET_TYPE_FLAMETABLE = 0x57,
		ASSET_TYPE_BITFIELD = 0x58,
		ASSET_TYPE_ATTACHMENT_COSMETIC_VARIANT = 0x59,
		ASSET_TYPE_MAPTABLE = 0x5A,
		ASSET_TYPE_MAPTABLE_LOADING_IMAGES = 0x5B,
		ASSET_TYPE_MEDAL = 0x5C,
		ASSET_TYPE_MEDALTABLE = 0x5D,
		ASSET_TYPE_OBJECTIVE = 0x5E,
		ASSET_TYPE_OBJECTIVE_LIST = 0x5F,
		ASSET_TYPE_UMBRA_TOME = 0x60,
		ASSET_TYPE_NAVMESH = 0x61,
		ASSET_TYPE_NAVVOLUME = 0x62,
		ASSET_TYPE_BINARYHTML = 0x63,
		ASSET_TYPE_LASER = 0x64,
		ASSET_TYPE_BEAM = 0x65,
		ASSET_TYPE_STREAMER_HINT = 0x66,
		ASSET_TYPE_COUNT = 0x67,
		ASSET_TYPE_STRING = 0x68,
		ASSET_TYPE_ASSETLIST = 0x69,
		ASSET_TYPE_REPORT = 0x6A,
		ASSET_TYPE_DEPEND = 0x68,
		ASSET_TYPE_FULL_COUNT = 0x6C,
	};

	struct RawFile
	{
		const char* name;
		int len;
		const char* buffer;
	};

	struct XZoneBuffer
	{
		const void* data;
		size_t dataSize;
	};

	struct XZoneInfo
	{
		const char* name;
		int allocFlags;
		int freeFlags;
		int allocSlot;
		int freeSlot;
		XZoneBuffer fileBuffer;
	};

	typedef void (*xcommand_t)();

	struct cmd_function_s
	{
		cmd_function_s* next;
		const char* name;
		const char* autoCompleteDir;
		const char* autoCompleteExt;
		xcommand_t function;
		int autoComplete;
	};

	struct CmdArgs
	{
		int nesting;
		int localClientNum[8];
		int controllerIndex[8];
		int argshift[8];
		int argc[8];
		const char** argv[8];
		char textPool[8192];
		const char* argvPool[512];
		int usedTextPool[8];
		int totalUsedArgvPool;
		int totalUsedTextPool;
	};

	struct va_info_t
	{
		char va_string[4][1024];
		int index;
	};

	struct TLSData
	{
		va_info_t* vaInfo;
		jmp_buf* errorJmpBuf;
		void* traceInfo;
		CmdArgs* cmdArgs;
		void* errorData;
	};

	typedef uint32_t dvarStrHash_t;

	enum dvarType_t
	{
		DVAR_TYPE_INVALID = 0x0,
		DVAR_TYPE_BOOL = 0x1,
		DVAR_TYPE_FLOAT = 0x2,
		DVAR_TYPE_FLOAT_2 = 0x3,
		DVAR_TYPE_FLOAT_3 = 0x4,
		DVAR_TYPE_FLOAT_4 = 0x5,
		DVAR_TYPE_INT = 0x6,
		DVAR_TYPE_ENUM = 0x7,
		DVAR_TYPE_STRING = 0x8,
		DVAR_TYPE_COLOR = 0x9,
		DVAR_TYPE_INT64 = 0xA,
		DVAR_TYPE_UINT64 = 0xB,
		DVAR_TYPE_LINEAR_COLOR_RGB = 0xC,
		DVAR_TYPE_COLOR_XYZ = 0xD,
		DVAR_TYPE_COLOR_LAB = 0xE,
		DVAR_TYPE_SESSIONMODE_BASE_DVAR = 0xF,
		DVAR_TYPE_COUNT = 0x10,
	};

	enum dvarFlags_e
	{
		DVAR_NONE = 0,
		DVAR_ARCHIVE = 1 << 0,
		DVAR_USERINFO = 1 << 1,
		DVAR_SERVERINFO = 1 << 2,
		DVAR_SYSTEMINFO = 1 << 3,
		DVAR_LATCH = 1 << 4,
		DVAR_ROM = 1 << 5,
		DVAR_SAVED = 1 << 6,
		DVAR_INIT = 1 << 7,
		DVAR_CHEAT = 1 << 8,
		//DVAR_UNKNOWN = 1 << 9,
		DVAR_EXTERNAL = 1 << 10,
		//DVAR_UNKNOWN3x = 1 << 11-13,
		DVAR_SESSIONMODE = 1 << 15
	};

	typedef float vec_t;

	union vec4_t
	{
		vec_t v[4];
		//vec4_t::$E8049E02A67BEF20C2B48C1E90A72E45 _anon_0;
		//vec4_t::$65A5F67E76558B5B186374890F5F7384 _anon_1;
		//vec3_t xyz;
	};

	struct dvar_t;

	struct DvarValue
	{
		union
		{
			bool enabled;
			int integer;
			uint32_t unsignedInt;
			int64_t integer64;
			uint64_t unsignedInt64;
			float value;
			vec4_t vector;
			const char* string;
			byte color[4];
			const dvar_t* indirect[3];
		} value;

		uint64_t encryptedValue;
	};

	union DvarLimits
	{
		struct
		{
			int stringCount;
			const char** strings;
		} enumeration;

		struct
		{
			int min;
			int max;
		} integer;

		struct
		{
			int64_t min;
			int64_t max;
		} integer64;

		struct
		{
			uint64_t min;
			uint64_t max;
		} unsignedInt64;

		struct
		{
			float min;
			float max;
		} value;

		struct
		{
			vec_t min;
			vec_t max;
		} vector;
	};

	struct dvar_t
	{
		dvarStrHash_t name;
		const char* debugName;
		const char* description;
		unsigned int flags;
		dvarType_t type;
		bool modified;
		DvarValue current;
		DvarValue latched;
		DvarValue reset;
		DvarLimits domain;
		dvar_t* hashNext;
	};

	struct netipv4_t
	{
		byte a;
		byte b;
		byte c;
		byte d;
	};

	enum netadrtype_t
	{
		NA_BOT = 0x0,
		NA_BAD = 0x1,
		NA_LOOPBACK = 0x2,
		NA_RAWIP = 0x3,
		NA_IP = 0x4,
	};

	enum netsrc_t
	{
		NS_NULL = -1,
		NS_CLIENT1 = 0x0,
		NS_CLIENT2 = 0x1,
		NS_CLIENT3 = 0x2,
		NS_CLIENT4 = 0x3,
		NS_SERVER = 0x4,
		NS_MAXCLIENTS = 0x4,
		NS_PACKET = 0x5,
	};

	struct netadr_t
	{
		union
		{
			netipv4_t ipv4;
			uint32_t addr;
		};

		uint16_t port;
		netadrtype_t type;
		netsrc_t localNetID;
	};

	struct XNADDR
	{
		byte addrBuff[37];
	};

	struct bdSecurityID
	{
		byte ab[8];
	};

	typedef bdSecurityID XNKID;

	struct bdSecurityKey
	{
		byte ab[16];
	};

	typedef bdSecurityKey XNKEY;

	typedef uint64_t XUID;

	struct SerializedAdr
	{
		byte valid;
		byte addrBuff[37];
	};

	struct XSESSION_INFO
	{
		XNKID sessionID;
		XNADDR hostAddress;
		XNKEY keyExchangeKey;
	};

	typedef int qboolean;

	enum PacketModeList
	{
		PACKETDATA_FIRST = 0x0,
		PACKETDATA_UNDEFINED = 0x0,
		PACKETDATA_HEADER = 0x1,
		PACKETDATA_OVERHEAD = 0x2,
		PACKETDATA_DATA = 0x3,
		PACKETDATA_RELIABLEDATA = 0x4,
		PACKETDATA_ZEROFLOAT = 0x5,
		PACKETDATA_SMALLFLOAT = 0x6,
		PACKETDATA_LARGEFLOAT = 0x7,
		PACKETDATA_ZEROINT = 0x8,
		PACKETDATA_SMALLANGLE = 0x9,
		PACKETDATA_ZEROANGLE = 0xA,
		PACKETDATA_TIMEDELTA = 0xB,
		PACKETDATA_TIME = 0xC,
		PACKETDATA_24BITFLAGINDEX = 0xD,
		PACKETDATA_GROUNDENTITY = 0xE,
		PACKETDATA_ENTITYNUM = 0xF,
		PACKETDATA_LASTFIELDCHANGED = 0x10,
		PACKETDATA_NOTNETWORKDATA = 0x11,
		PACKETDATA_ORIGINDELTA = 0x12,
		PACKETDATA_ORIGIN = 0x13,
		NUM_PACKETDATA_MODES = 0x14,
	};

	struct PacketMode
	{
		unsigned int start;
		PacketModeList mode;
	};

	struct msg_t
	{
		qboolean overflowed;
		qboolean readOnly;
		byte* data;
		byte* splitData;
		int maxsize;
		int cursize;
		int splitSize;
		int readcount;
		int bit;
		int lastEntityRef;
		qboolean flush;
		netsrc_t targetLocalNetID;
		//PacketMode analysis;
	};

	typedef void* bdCommonAddrRef;

	struct HostInfo
	{
		uint64_t xuid;
		char name[32];
		netadr_t netAdr;
		SerializedAdr serializedAdr;
		bdSecurityID secId;
		bdSecurityKey secKey;
		uint32_t serverLocation;
	};

	enum LobbyType
	{
		LOBBY_TYPE_INVALID = 0xFFFFFFFF,
		LOBBY_TYPE_PRIVATE = 0x0,
		LOBBY_TYPE_GAME = 0x1,
		LOBBY_TYPE_TRANSITION = 0x2,
		LOBBY_TYPE_COUNT = 0x3,
		LOBBY_TYPE_FIRST = 0x0,
		LOBBY_TYPE_LAST = 0x2,
		LOBBY_TYPE_AUTO = 0x3,
	};

	enum LobbyNetworkMode
	{
		LOBBY_NETWORKMODE_INVALID = 0xFFFFFFFF,
		LOBBY_NETWORKMODE_LOCAL = 0x0,
		LOBBY_NETWORKMODE_LAN = 0x1,
		LOBBY_NETWORKMODE_LIVE = 0x2,
		LOBBY_NETWORKMODE_COUNT = 0x3,
	};

	enum LobbyMainMode
	{
		LOBBY_MAINMODE_INVALID = 0xFFFFFFFF,
		LOBBY_MAINMODE_CP = 0x0,
		LOBBY_MAINMODE_MP = 0x1,
		LOBBY_MAINMODE_ZM = 0x2,
		LOBBY_MAINMODE_COUNT = 0x3,
	};

	struct LobbyParams
	{
		LobbyNetworkMode networkMode;
		LobbyMainMode mainMode;
	};

	enum JoinType
	{
		JOIN_TYPE_NORMAL = 0x0,
		JOIN_TYPE_PLAYLIST = 0x1,
		JOIN_TYPE_FRIEND = 0x2,
		JOIN_TYPE_INVITE = 0x3,
		JOIN_TYPE_PARTY = 0x4,
		JOIN_TYPE_COUNT = 0x5,
	};

	struct JoinHost
	{
		HostInfo info;
		LobbyType lobbyType;
		LobbyParams lobbyParams;
		uint64_t reservationKey;
		int retryTime;
		int retryCount;
	};

	enum JoinSourceState
	{
		JOIN_SOURCE_STATE_IDLE = 0x0,
		JOIN_SOURCE_STATE_CONNECT_TO_NEXT_HOST = 0x1,
		JOIN_SOURCE_STATE_ASSOCIATING = 0x2,
		JOIN_SOURCE_STATE_HANDSHAKING = 0x3,
		JOIN_SOURCE_STATE_WAITING_FOR_AGREEMENT = 0x4,
		JOIN_SOURCE_STATE_CONNECTION_FAILED = 0x5,
		JOIN_SOURCE_STATE_CONNECTION_SUCCESS = 0x6,
		JOIN_SOURCE_STATE_ENDING_HOST = 0x7,
		JOIN_SOURCE_STATE_CLEANUP = 0x8,
		JOIN_SOURCE_STATE_COUNT = 0x9,
	};

	enum JoinResult
	{
		JOIN_RESULT_INVALID = 0x0,
		JOIN_RESULT_SUCCESS = 0x1,
		JOIN_RESULT_CONNECT_TO_HOST_FAILURE = 0x2,
		JOIN_RESULT_PROBE_SEND_FAILURE = 0x3,
		JOIN_RESULT_PROBE_TIMEOUT = 0x4,
		JOIN_RESULT_PROBE_INVALID_LOBBY = 0x5,
		JOIN_RESULT_PROBE_INVALID_INFO = 0x6,
		JOIN_RESULT_PROBE_RESULT_INVALID = 0x7,
		JOIN_RESULT_INVALID_LOBBY = 0x8,
		JOIN_RESULT_SEND_AGREEMENT_REQUEST_FAILED = 0x9,
		JOIN_RESULT_HANDSHAKE_WINDOW_EXPIRED = 0xA,
		JOIN_RESULT_AGREEMENT_WINDOW_EXPIRED = 0xB,
		JOIN_RESULT_JOIN_DISABLED = 0xC,
		JOIN_RESULT_JOIN_ALREADY_IN_PROGRESS = 0xD,
		JOIN_RESULT_NOT_JOINABLE_NOT_HOSTING = 0xE,
		JOIN_RESULT_NOT_JOINABLE_NOT_IDLE = 0xF,
		JOIN_RESULT_NOT_JOINABLE_CLOSED = 0x10,
		JOIN_RESULT_NOT_JOINABLE_INVITE_ONLY = 0x11,
		JOIN_RESULT_NOT_JOINABLE_FRIENDS_ONLY = 0x12,
		JOIN_RESULT_LOBBY_FULL = 0x13,
		JOIN_RESULT_NETWORK_MODE_MISMATCH = 0x14,
		JOIN_RESULT_MISMATCH_PLAYLISTID = 0x15,
		JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_NEW = 0x16,
		JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_OLD = 0x17,
		JOIN_RESULT_MISMATCH_PROTOCOL_VERSION = 0x18,
		JOIN_RESULT_MISMATCH_NETFIELD_CHECKSUM = 0x19,
		JOIN_RESULT_MISMATCH_FFOTD_VERSION_TO_NEW = 0x1A,
		JOIN_RESULT_MISMATCH_FFOTD_VERSION_TO_OLD = 0x1B,
		JOIN_RESULT_MIGRATE_IN_PROGRESS = 0x1C,
		JOIN_RESULT_COULD_NOT_RESERVE = 0x1D,
		JOIN_RESPONSE_COUNT = 0x1E,
	};

	typedef void (*joinCompleteCallback)(int, JoinResult);

	struct AgreementStatus
	{
		XUID xuid;
		char name[32];
		bool responded;
		bool agrees;
		int startTime;
		int responseTime;
	};

	struct Agreement
	{
		int nonce;
		AgreementStatus status[18];
		int requestCount;
		int responseCount;
		int agreeCount;
	};

	struct Join
	{
		JoinSourceState state;
		int actionId;
		int startTime;
		int duration;
		ControllerIndex_t controllerIndex;
		LobbyType sourceLobbyType;
		LobbyType targetLobbyType;
		joinCompleteCallback joinComplete;
		JoinHost hostList[50];
		int hostCount;
		int processedCount;
		bool isFinalized;
		JoinHost potentialHost;
		Agreement agreement;
		Agreement debugAgreement;
		JoinType joinType;
		JoinResult joinResult;
	};

	struct ServerInfo
	{
		uint16_t m_usConnectionPort;
		uint16_t m_usQueryPort;
		uint32_t m_unIP;
		int m_nPing;
		byte unk[0x22];
		char mapname[32];
		char description[64];
		char gamemode[16];
		char modname[32];
		int playerCount;
		int maxPlayers;
		int unk2;
		int unk3;
		int unk4;
		bool dedicated;
		bool ranked;
		bool hardcore;
		bool zombies;
		char servername[64];
		char tags[128];
		int unk5;
		int unk6;
	};

#ifdef __cplusplus
	namespace hks
	{
		struct lua_State;
		struct HashTable;
		struct StringTable;
		struct cclosure;
		typedef int hksBool;
		typedef char hksChar;
		typedef unsigned __int8 hksByte;
		typedef __int16 hksShort16;
		typedef unsigned __int16 hksUshort16;
		typedef float HksNumber;
		typedef int hksInt32;
		typedef unsigned int hksUint32;
		typedef __int64 hksInt64;
		typedef unsigned __int64 hksUint64;

		typedef int HksGcCost;


		typedef size_t hksSize;
		typedef void* (*lua_Alloc)(void*, void*, size_t, size_t);
		typedef hksInt32 (*lua_CFunction)(lua_State*);

		struct GenericChunkHeader
		{
			hksSize m_flags;
		};

		struct ChunkHeader : GenericChunkHeader
		{
			ChunkHeader* m_next;
		};

		struct ChunkList
		{
			ChunkHeader m_head;
		};

		struct UserData : ChunkHeader
		{
			unsigned __int64 m_envAndSizeOffsetHighBits;
			unsigned __int64 m_metaAndSizeOffsetLowBits;
			char m_data[8];
		};

		struct InternString
		{
			unsigned __int64 m_flags;
			unsigned __int64 m_lengthbits;
			unsigned int m_hash;
			char m_data[30];
		};

		union HksValue
		{
			cclosure* cClosure;
			void* closure;
			UserData* userData;
			HashTable* table;
			void* tstruct;
			InternString* str;
			void* thread;
			void* ptr;
			float number;
			unsigned int native;
			bool boolean;
		};

		enum HksObjectType
		{
			TANY = 0xFFFFFFFE,
			TNONE = 0xFFFFFFFF,
			TNIL = 0x0,
			TBOOLEAN = 0x1,
			TLIGHTUSERDATA = 0x2,
			TNUMBER = 0x3,
			TSTRING = 0x4,
			TTABLE = 0x5,
			TFUNCTION = 0x6,
			// idk
			TUSERDATA = 0x7,
			TTHREAD = 0x8,
			TIFUNCTION = 0x9,
			// Lua function
			TCFUNCTION = 0xA,
			// C function
			TUI64 = 0xB,
			TSTRUCT = 0xC,
			NUM_TYPE_OBJECTS = 0xE,
		};

		struct HksObject
		{
			HksObjectType t;
			HksValue v;
		};

		const struct hksInstruction
		{
			unsigned int code;
		};

		struct ActivationRecord
		{
			HksObject* m_base;
			const hksInstruction* m_returnAddress;
			__int16 m_tailCallDepth;
			__int16 m_numVarargs;
			int m_numExpectedReturns;
		};

		struct CallStack
		{
			ActivationRecord* m_records;
			ActivationRecord* m_lastrecord;
			ActivationRecord* m_current;
			const hksInstruction* m_current_lua_pc;
			const hksInstruction* m_hook_return_addr;
			int m_hook_level;
		};

		struct ApiStack
		{
			HksObject* top;
			HksObject* base;
			HksObject* alloc_top;
			HksObject* bottom;
		};

		struct UpValue : ChunkHeader
		{
			HksObject m_storage;
			HksObject* loc;
			UpValue* m_next;
		};

		struct CallSite
		{
			_SETJMP_FLOAT128 m_jumpBuffer[16];
			CallSite* m_prev;
		};

		enum Status
		{
			NEW = 0x1,
			RUNNING = 0x2,
			YIELDED = 0x3,
			DEAD_ERROR = 0x4,
		};

		enum HksError
		{
			HKS_NO_ERROR = 0,
			HKS_ERRSYNTAX = -4,
			HKS_ERRFILE = -5,
			HKS_ERRRUN = -100,
			HKS_ERRMEM = -200,
			HKS_ERRERR = -300,
			HKS_THROWING_ERROR = -500,
			HKS_GC_YIELD = 1,
		};

		struct lua_Debug
		{
			int event;
			const char* name;
			const char* namewhat;
			const char* what;
			const char* source;
			int currentline;
			int nups;
			int nparams;
			int ishksfunc;
			int linedefined;
			int lastlinedefined;
			char short_src[512];
			int callstack_level;
			int is_tail_call;
		};

		using lua_function = int(__fastcall*)(lua_State*);

		struct luaL_Reg
		{
			const char* name;
			lua_function function;
		};

		struct Node
		{
			HksObject m_key;
			HksObject m_value;
		};

		struct StringPinner
		{
			struct Node
			{
				InternString* m_strings[32];
				Node* m_prev;
			};

			lua_State* const m_state;
			StringPinner* const m_prev;
			InternString** m_nextStringsPlace;
			Node m_firstNode;
			Node* m_currentNode;
		};

		struct StringTable
		{
			InternString** m_data;
			unsigned int m_count;
			unsigned int m_mask;
			StringPinner* m_pinnedStrings;
		};

		struct Metatable
		{
		};

		struct HashTable : ChunkHeader
		{
			Metatable* m_meta;
			unsigned int m_version;
			unsigned int m_mask;
			Node* m_hashPart;
			HksObject* m_arrayPart;
			unsigned int m_arraySize;
			Node* m_freeNode;
		};

		struct cclosure : ChunkHeader
		{
			lua_function m_function;
			HashTable* m_env;
			__int16 m_numUpvalues;
			__int16 m_flags;
			InternString* m_name;
			HksObject m_upvalues[1];
		};

		enum HksCompilerSettings_BytecodeSharingFormat
		{
			BYTECODE_DEFAULT = 0x0,
			BYTECODE_INPLACE = 0x1,
			BYTECODE_REFERENCED = 0x2,
		};

		enum HksCompilerSettings_IntLiteralOptions
		{
			INT_LITERALS_NONE = 0x0,
			INT_LITERALS_LUD = 0x1,
			INT_LITERALS_32BIT = 0x1,
			INT_LITERALS_UI64 = 0x2,
			INT_LITERALS_64BIT = 0x2,
			INT_LITERALS_ALL = 0x3,
		};

		struct HksCompilerSettings
		{
			int m_emitStructCode;
			const char** m_stripNames;
			int m_emitGlobalMemoization;
			int _m_isHksGlobalMemoTestingMode;
			HksCompilerSettings_BytecodeSharingFormat m_bytecodeSharingFormat;
			HksCompilerSettings_IntLiteralOptions m_enableIntLiterals;
			int (*m_debugMap)(const char*, int);
		};

		enum HksBytecodeSharingMode : __int64
		{
			HKS_BYTECODE_SHARING_OFF = 0,
			HKS_BYTECODE_SHARING_ON = 1,
			HKS_BYTECODE_SHARING_SECURE = 2
		};

		struct HksGcWeights
		{
			int m_removeString;
			int m_finalizeUserdataNoMM;
			int m_finalizeUserdataGcMM;
			int m_cleanCoroutine;
			int m_removeWeak;
			int m_markObject;
			int m_traverseString;
			int m_traverseUserdata;
			int m_traverseCoroutine;
			int m_traverseWeakTable;
			int m_freeChunk;
			int m_sweepTraverse;
		};

		struct GarbageCollector_Stack
		{
			void* m_storage;
			unsigned int m_numEntries;
			unsigned int m_numAllocated;
		};

		struct ProtoList
		{
			void** m_protoList;
			unsigned __int16 m_protoSize;
			unsigned __int16 m_protoAllocSize;
		};

		struct MemoryManager;

		struct GarbageCollector
		{
			struct ResumeStack
			{
				void* m_storage;
				hksInt32 m_numEntries;
				hksUint32 m_numAllocated;
			};

			struct GreyStack
			{
				HksObject* m_storage;
				hksSize m_numEntries;
				hksSize m_numAllocated;
			};

			struct RemarkStack
			{
				HashTable** m_storage;
				hksSize m_numAllocated;
				hksSize m_numEntries;
			};

			struct WeakStack_Entry
			{
				hksInt32 m_weakness;
				HashTable* m_table;
			};

			struct WeakStack
			{
				WeakStack_Entry* m_storage;
				hksInt32 m_numEntries;
				hksUint32 m_numAllocated;
			};

			HksGcCost m_target;
			HksGcCost m_stepsLeft;
			HksGcCost m_stepLimit;
			HksGcWeights m_costs;
			HksGcCost m_unit;
			void* m_jumpPoint;
			lua_State* m_mainState;
			lua_State* m_finalizerState;
			MemoryManager* m_memory;
			void* m_emergencyGCMemory;
			hksInt32 m_phase;
			ResumeStack m_resumeStack;
			GreyStack m_greyStack;
			RemarkStack m_remarkStack;
			WeakStack m_weakStack;
			hksBool m_finalizing;
			HksObject m_safeTableValue;
			lua_State* m_startOfStateStackList;
			lua_State* m_endOfStateStackList;
			lua_State* m_currentState;
			HksObject m_safeValue;
			void* m_compiler;
			void* m_bytecodeReader;
			void* m_bytecodeWriter;
			hksInt32 m_pauseMultiplier;
			HksGcCost m_stepMultiplier;
			hksSize m_emergencyMemorySize;
			bool m_stopped;
			lua_CFunction m_gcPolicy;
			hksSize m_pauseTriggerMemoryUsage;
			hksInt32 m_stepTriggerCountdown;
			hksUint32 m_stringTableIndex;
			hksUint32 m_stringTableSize;
			UserData* m_lastBlackUD;
			UserData* m_activeUD;
		};

		enum MemoryManager_ChunkColor
		{
			RED = 0x0,
			BLACK = 0x1,
		};

		enum Hks_DeleteCheckingMode
		{
			HKS_DELETE_CHECKING_OFF = 0x0,
			HKS_DELETE_CHECKING_ACCURATE = 0x1,
			HKS_DELETE_CHECKING_SAFE = 0x2,
		};

		struct MemoryManager
		{
			enum ChunkColor : __int32
			{
				WHITE = 0x0,
				BLACK = 0x1,
			};

			lua_Alloc m_allocator;
			void* m_allocatorUd;
			ChunkColor m_chunkColor;
			hksSize m_used;
			hksSize m_highwatermark;
			ChunkList m_allocationList;
			ChunkList m_sweepList;
			ChunkHeader* m_lastKeptChunk;
			lua_State* m_state;
		};

		struct StaticStringCache
		{
			HksObject m_objects[41];
		};

		enum HksBytecodeEndianness
		{
			HKS_BYTECODE_DEFAULT_ENDIAN = 0x0,
			HKS_BYTECODE_BIG_ENDIAN = 0x1,
			HKS_BYTECODE_LITTLE_ENDIAN = 0x2,
		};

		struct RuntimeProfileData_Stats
		{
			unsigned __int64 hksTime;
			unsigned __int64 callbackTime;
			unsigned __int64 gcTime;
			unsigned __int64 cFinalizerTime;
			unsigned __int64 compilerTime;
			unsigned int hkssTimeSamples;
			unsigned int callbackTimeSamples;
			unsigned int gcTimeSamples;
			unsigned int compilerTimeSamples;
			unsigned int num_newuserdata;
			unsigned int num_tablerehash;
			unsigned int num_pushstring;
			unsigned int num_pushcfunction;
			unsigned int num_newtables;
		};

		struct RuntimeProfileData
		{
			__int64 stackDepth;
			__int64 callbackDepth;
			unsigned __int64 lastTimer;
			RuntimeProfileData_Stats frameStats;
			unsigned __int64 gcStartTime;
			unsigned __int64 finalizerStartTime;
			unsigned __int64 compilerStartTime;
			unsigned __int64 compilerStartGCTime;
			unsigned __int64 compilerStartGCFinalizerTime;
			unsigned __int64 compilerCallbackStartTime;
			__int64 compilerDepth;
			void* outFile;
			lua_State* rootState;
		};

		struct HksGlobal
		{
			MemoryManager m_memory;
			GarbageCollector m_collector;
			StringTable m_stringTable;
			__int64 padding3;
			HksBytecodeSharingMode m_bytecodeSharingMode;
			int padding;
			HksObject m_registry;
			ChunkList m_userDataList;
			lua_State* m_root;
			StaticStringCache m_staticStringCache;
			void* m_debugger;
			void* m_profiler;
			RuntimeProfileData m_runProfilerData;
			HksCompilerSettings m_compilerSettings;
			int (*m_panicFunction)(lua_State*);
			void* m_luaplusObjectList;
			int m_heapAssertionFrequency;
			int m_heapAssertionCount;
			void (*m_logFunction)(lua_State*, const char*, ...);
			void (*m_emergencyGCFailFunction)(lua_State*, size_t);
			HksBytecodeEndianness m_bytecodeDumpEndianness;
			int padding2;
		};

		struct lua_State
		{
			ChunkHeader baseclass;
			HksGlobal* m_global;
			CallStack m_callStack;
			ApiStack m_apistack;
			UpValue* pending;
			HksObject globals;
			HksObject m_cEnv;
			CallSite* m_callsites;
			int m_numberOfCCalls;
			void* m_context;
			InternString* m_name;
			lua_State* m_nextState;
			lua_State* m_nextStateStack;
			Status m_status;
			HksError m_error;
		};
	}
#endif

	typedef uint32_t ScrVarCanonicalName_t;

	enum svscmd_type
	{
		SV_CMD_CAN_IGNORE_0 = 0x0,
		SV_CMD_RELIABLE_0 = 0x1,
	};

	struct client_s
	{
		int client_state;
		char __pad0[0x28];
		netadr_t address;
		char __pad1[0x5588];
		uint64_t xuid;
		char __pad2[0xB5D84];
		int guid;
		char __pad3[0x8];
		bool bIsTestClient;
		char __pad4[0x29DAC];
	};

#ifdef __cplusplus
	static_assert(sizeof(client_s) == 0xE5110);

	static_assert(offsetof(game::client_s, address) == 0x2C);
	static_assert(offsetof(game::client_s, xuid) == 0x55C8);
	static_assert(offsetof(game::client_s, guid) == 0xBB354);
	static_assert(offsetof(game::client_s, bIsTestClient) == 0xBB360);
#endif

	struct client_s_cl : client_s
	{
		char __pad1_0[0x60];
	};

#ifdef __cplusplus
	static_assert(sizeof(client_s_cl) == 0xE5170);
#endif

	enum scriptInstance_t
	{
		SCRIPTINSTANCE_SERVER = 0x0,
		SCRIPTINSTANCE_CLIENT = 0x1,
		SCRIPT_INSTANCE_MAX = 0x2,
	};

	struct gclient_s
	{
		char __pad0[0x8C];
		float velocity[3];
		char __pad1[59504];
		char flags;
	};

	struct EntityState
	{
		int number;
	};

	struct gentity_s
	{
		EntityState s;
		unsigned char __pad0[0x24C];
		gclient_s* client;
		unsigned char __pad1[0x2A0];
	};

#ifdef __cplusplus
	static_assert(sizeof(gentity_s) == 0x4F8);
#endif

	enum workshop_type
	{
		WORKSHOP_MOD = 0x1,
		WORKSHOP_USERMAP = 0x2
	};

	struct workshop_data
	{
		char title[100];
		char folderName[32];
		char publisherId[32];
		char description[256];
		char contentPathToZoneFiles[260];
		char absolutePathContentFolder[260];
		char absolutePathZoneFiles[260];
		int unk; // 1
		int unk2; // 0
		unsigned int publisherIdInteger;
		int unk3;
		unsigned int unk4;
		workshop_type type;
	};

#ifdef __cplusplus
	static_assert(sizeof(workshop_data) == 0x4C8);
#endif

	struct DDLMember
	{
		const char* name;
		int index;
		void* parent;
		int bitSize;
		int limitSize;
		int offset;
		int type;
		int externalIndex;
		unsigned int rangeLimit;
		unsigned int serverDelta;
		unsigned int clientDelta;
		int arraySize;
		int enumIndex;
		int permission;
	};

	struct DDLHash
	{
		int hash;
		int index;
	};

	struct DDLHashTable
	{
		DDLHash* list;
		int count;
		int max;
	};

	struct DDLStruct
	{
		const char* name;
		int bitSize;
		int memberCount;
		DDLMember* members;
		DDLHashTable hashTableUpper;
		DDLHashTable hashTableLower;
	};

	struct DDLEnum
	{
		const char* name;
		int memberCount;
		const char** members;
		DDLHashTable hashTable;
	};

	struct DDLDef
	{
		char* name;
		uint16_t version;
		unsigned int checksum;
		byte flags;
		int bitSize;
		int byteSize;
		DDLStruct* structList;
		int structCount;
		DDLEnum* enumList;
		int enumCount;
		DDLDef* next;
		int headerBitSize;
		int headerByteSize;
		int reserveSize;
		int userFlagsSize;
		bool paddingUsed;
	};

	struct DDLContext;
	typedef void (* DDLWriteCB)(DDLContext*, void*);

	struct DDLContext
	{
		void* buff;
		int len;
		const DDLDef* def;
		DDLWriteCB writeCB;
		void* userData;
	};

	struct emblemChallengeLookup_t
	{
		__int16 challengeIndex;
		unsigned char itemIndex;
	};

	enum CharacterItemType
	{
		CHARACTER_ITEM_TYPE_BODY = 0x0,
		CHARACTER_ITEM_TYPE_HELMET = 0x1,
		CHARACTER_ITEM_TYPE_COUNT = 0x2,
	};

	typedef __int16 BGEmblemBackgroundID;

	union XAssetHeader
	{
		/*PhysPreset* physPreset;
		PhysConstraints* physConstraints;
		DestructibleDef* destructibleDef;
		XAnimParts* parts;
		XModel* model;
		XModelMesh* modelMesh;
		Material* material;
		MaterialComputeShaderSet* computeShaderSet;
		MaterialTechniqueSet* techniqueSet;
		GfxImage* image;
		SndBank* sound;
		SndPatch* soundPatch;
		clipMap_t* clipMap;
		ComWorld* comWorld;
		GameWorld* gameWorld;
		MapEnts* mapEnts;
		GfxWorld* gfxWorld;
		GfxLightDef* lightDef;
		GfxLensFlareDef* lensFlareDef;
		Font* font;
		FontIcon* fontIcon;
		LocalizeEntry* localize;
		WeaponVariantDef* weapon;
		WeaponAttachment* attachment;
		WeaponAttachmentUnique* attachmentUnique;
		WeaponCamo* weaponCamo;
		CustomizationTable* customizationTable;
		CustomizationColorInfo* customizationColorInfo;
		SndDriverGlobals* sndDriverGlobals;
		FxEffectDefHandleRaw fx;
		TagFxSet* tagFX;
		FxLensFlareDefPtr newLensFlareDef;
		FxImpactTable* impactFx;
		SoundsImpactTable* impactSounds;
		CgMediaTable* cgMedia;
		PlayerSoundsTable* playerSounds;
		PlayerFXTable* playerFX;
		SharedWeaponSounds* sharedWeaponSounds;
		RawFile* rawfile;
		StringTable* stringTable;
		StructuredTable* structuredTable;
		LeaderboardDef* leaderboardDef;
		DDLRoot* ddlRoot;
		Glasses* glasses;
		TextureList* textureList;
		ScriptParseTree* scriptParseTree;
		KeyValuePairs* keyValuePairs;
		VehicleDef* vehicleDef;
		AddonMapEnts* addonMapEnts;
		TracerDef* tracerDef;
		Qdb* qdb;
		Slug* slug;
		SurfaceFXTableDef* surfaceFXTableDef;
		SurfaceSoundDef* surfaceSoundDef;
		FootstepTableDef* footstepTableDef;
		EntitySoundImpacts* entitySoundImpacts;
		EntityFxImpacts* entityFxImpacts;
		ZBarrierDef* zbarrierDef;
		VehicleFxDef* vehicleFxDef;
		VehicleSoundDef* vehicleSoundDef;
		ArchiveTypeInfoArray* typeInfo;
		ScriptBundle* scriptBundle;
		ScriptBundleList* scriptBundleList;
		RumbleInfo* rumble;
		BulletPenetrationTable* bulletPenetration;
		LocDmgTable* locDmgTable;
		AimTable* aimTable;
		XModelAlias* xModelAlias;
		Character* character;
		AIType* aiType;
		PlayerCharacter* player_character;
		AnimSelectorTableSet* animSelectorTableSet;
		AnimMappingTable* animMappingTable;
		AnimStateMachine* animStateMachine;
		BehaviorTree* behaviorTree;
		BehaviorStateMachine* behaviorStateMachine;
		TTFDef* ttfDef;
		GfxSiegeAnim* sanim;
		GfxLightDescription* lightDescription;
		ShellshockParams* shellshock;
		XCam* xcam;
		BGCacheInfo* bgCache;
		TextureCombo* textureCombo;
		FlameTable* flameTable;
		Bitfield* bitfield;
		AttachmentCosmeticVariant* attachmentCosmeticVariant;
		MapTable* mapTable;
		Medal* medal;
		MedalTable* medalTable;
		Objective* objective;
		ObjectiveList* objectiveList;
		NavMeshData* navMesh;
		NavVolumeData* navVolume;
		BinaryHTML* binaryHTML;
		LaserDef* laserDef;
		BeamDef* beamDef;
		StreamerHint* streamerHint;*/
		void* data;
		RawFile* luaFile;
	};

	struct XAsset
	{
		XAssetType type;
		XAssetHeader header;
	};

	typedef void XAssetEnum(XAssetHeader, void*);

#ifdef __cplusplus
}
#endif
