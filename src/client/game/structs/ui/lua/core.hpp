#pragma once

#include <cstdint>
namespace game {
namespace ui {
namespace lua {
enum class PresencePrimary : int32_t {
  UNKNOWN = -1,
  OFFLINE = 0,
  ONLINE = 1,
  LOCAL = 2,
  TITLE = 3,
  AWAY = 4,
  BLOCKED = 5,
};
enum class PresenceActivity : int32_t {
  MENU_FIRST = 0,
  UNKNOWN = 0,
  OFFLINE = 0,
  ONLINE_NOT_IN_TITLE = 1,
  MENU_INLOBBY = 2,
  MENU_LOOKING_FOR_PARTY = 3,
  MENU_LOOKING_FOR_PLAYERS = 4,
  MENU_LOCAL = 5,
  MENU_START = 6,
  IN_TITLE = 7,
  NOT_IN_TITLE = 8,
  MENU_LAST = 15,
  CP_FIRST = 16,
  CP_PLAYING_COOP_ON_MAP = 17,
  CP_PLAYING_SOLO_ON_MAP = 18,
  CP_PLAYING_COOP_IN_SAFEHOUSE = 19,
  CP_PLAYING_SOLO_IN_SAFEHOUSE = 20,
  CP_IN_LOBBY = 21,
  CP_WATCHING_FILM = 22,
  CP_LOOKING_FOR_PARTY = 23,
  CP_LOOKING_FOR_PLAYERS = 24,
  CP_PLAYING_LOCAL = 25,
  CP_LAST = 31,
  MP_FIRST = 32,
  MP_PLAYING_GMODE_ON_MAP = 33,
  MP_IN_LOBBY = 34,
  MP_WATCHING_FILM = 35,
  MP_LOOKING_FOR_PARTY = 36,
  MP_LOOKING_FOR_PLAYERS = 37,
  MP_PLAYING_LOCAL = 38,
  MP_LAST = 47,
  ZM_FIRST = 48,
  ZM_PLAYING_MAP_ON_ROUND = 49,
  ZM_IN_LOBBY = 50,
  ZM_WATCHING_FILM = 51,
  ZM_LOOKING_FOR_PARTY = 52,
  ZM_LOOKING_FOR_PLAYERS = 53,
  ZM_PLAYING_LOCAL = 54,
  ZM_LAST = 63,
  LAST = 64,
};
enum class PresenceCtx : int32_t {
  MENU_FIRST = 0,
  UNKNOWN = 0,
  MENU_MODE_SELECT = 1,
  MENU_START = 2,
  MENU_NOT_ACTIVE = 3,
  MENU_LAST = 15,
  CP_FIRST = 16,
  CP_MAIN_MENU = 17,
  CP_PRIVATE_GAME = 18,
  CP_PUBLIC_MATCH = 19,
  CP_THEATER = 20,
  CP_LAST = 31,
  MP_FIRST = 32,
  MP_MAIN_MENU = 33,
  MP_CUSTOM_GAMES = 34,
  MP_PUBLIC_MATCH = 35,
  MP_THEATER = 36,
  MP_LAST = 47,
  ZM_FIRST = 48,
  ZM_MAIN_MENU = 49,
  ZM_PRIVATE_GAME = 50,
  ZM_PUBLIC_MATCH = 51,
  ZM_THEATER = 52,
  ZM_LAST = 63,
  LAST = 64,
};
enum class PresenceFilter : int32_t {
  ALL = 0,
  ONLINE_AND_NOT_IN_TITLE = 1,
  ONLINE_AND_IN_TITLE = 2,
  ONLINE = 3,
  CACHED = 255,
};
enum class PresenceSorting : int32_t {
  UNSORTED = 0,
  ONLINE_MOST_RECENT = 1,
  ALPHABETICAL = 2,
};
enum class GroupMenu : int32_t {
  DEFAULT = 0,
  HOME = 0,
  ROSTER = 1,
  BADGES = 2,
  LEADERBOARD = 3,
  SHARE = 4,
  ADMIN = 5,
  COUNT = 6,
};
enum class GroupMemberStatus : int32_t {
  MEMBER = 0,
  ADMIN = 1,
  OWNER = 2,
  COUNT = 3,
  INVALID = 3,
};
enum class GroupPrivacy : int32_t {
  PUBLIC = 0,
  PRIVATE = 1,
  COUNT = 2,
};
enum class GroupApplicationState : int32_t {
  NONE = 0,
  PENDING = 1,
  REJECTED = 2,
  WITHDRAWN = 3,
};
enum class GroupBufferType : int32_t {
  SELF = 0,
  SEARCH = 1,
  COUNT = 2,
  FIRST = 0,
  INVALID = 2,
};
enum class GroupSearchType : int32_t {
  INVALID = 0,
  FRIENDS = 1,
  SELECTED_FRIEND = 2,
  POPULAR = 3,
  SEARCH_BY_PARAMS = 4,
  COUNT = 5,
};
enum class GroupJoinApprovalType : int32_t {
  OFF = 0,
  ON = 1,
  INVITE_ONLY = 2,
  TYPE_INVALID = 3,
};
enum class GroupAdminPrivilege : int32_t {
  INVITE_PLAYERS = 0,
  APPROVE_JOIN_REQUESTS = 1,
  REMOVE_MEMBERS = 2,
  BAN_MEMBERS = 3,
  FAVORITE_SHOWCASE_CONTENT = 4,
  REMOVE_SHOWCASE_CONTENT = 5,
  BLOCK_MEMBER_CONTENT = 6,
  EDIT_FEATURED_CONTENT = 7,
  EDIT_GROUP_MESSAGE = 8,
  EDIT_GROUP_LEADERBOARD = 9,
  EDIT_GROUP_EMBLEM = 10,
  EDIT_GROUP_DESCRIPTION = 11,
  EDIT_GROUP_TYPE = 12,
  EDIT_GROUP_NAME = 13,
  MANAGE_ADMINS = 14,
  COUNT = 15,
  TYPE_INVALID = 15,
};
enum class GroupSearchSizeRangeIndex : int32_t {
  INDEX_1 = 0,
  INDEX_FIRST = 0,
  INDEX_2 = 1,
  INDEX_3 = 2,
  INDEX_4 = 3,
  INDEX_LAST = 3,
  COUNT = 4,
  INDEX_ANY = 4,
};
enum class GroupUpdateGamertagsType : int32_t {
  INVITES = 0,
  GROUP_MEMBERS = 1,
  COUNT = 2,
};
enum class GroupUpdatePrivacyType : int32_t {
  QUICKJOIN = 0,
  GROUP_MEMBERS = 1,
  COUNT = 2,
};
enum class connectionType_e : int32_t {
  UNKNOWN = 0,
  WIRED = 1,
  WIRELESS = 2,
};
enum class XONLINE_NAT_TYPE : int32_t {
  UNKNOWN = 0,
  OPEN = 1,
  MODERATE = 2,
  XONLINE_NAT_STRICT = 3,
  TYPE_COUNT = 4,
};
enum class LiveInternalEvent : int32_t {
  FIRST = 0,
  RESERVED = 0,
  FRIENDS_LIST_UPDATED = 1,
  FRIENDS_PRESENCE_UPDATED = 2,
  QUICKJOIN_PRESENCE_UPDATED = 3,
  COUNT = 4,
};
enum class SYSINFO : int32_t {
  VERSION_NUMBER = 0,
  CONNECTIVITY_INFO = 1,
  NAT_TYPE = 2,
  CUSTOMER_SUPPORT_LINK = 3,
  BANDWIDTH = 4,
  IP_ADDRESS = 5,
  EXTERNAL_IP_ADDRESS = 6,
  INTERNAL_IP_ADDRESS = 7,
  GEOGRAPHICAL_REGION = 8,
  Q = 9,
  CONSOLE_ID = 10,
  MAC_ADDRESS = 11,
  NAT_TYPE_LOBBY = 12,
  CONNECTION_TYPE = 13,
};
enum class demoCameraMode : int32_t {
  NONE = 0,
  THIRDPERSON = 1,
  FREECAM = 2,
  COUNT = 3,
};
enum class demoFreeCameraMode : int32_t {
  FREEROAM = 0,
  EDIT = 1,
  DOLLY = 2,
  LOCK_ON = 3,
  LIGHTMAN = 4,
  COUNT = 5,
  FIRST = 0,
  LAST = 2,
};
enum class demoLightmanLightMode : int32_t {
  OMNI = 0,
  SPOT = 1,
  COUNT = 2,
};
enum class demoClipState : int32_t {
  IDLE = 0,
  RECORD_UNCOMPRESSED_SNAPHOT = 1,
  RECORD_CONTINUOUS = 2,
  PAUSED = 3,
  PREVIEW = 4,
  STREAMING = 5,
};
enum class demoClipTransition : int32_t {
  NONE = 0,
  FADE = 1,
  COUNT = 2,
  FIRST = 0,
  LAST = 1,
};
enum class demoContextMode : int32_t {
  PLAYBACK = 0,
  BASIC = 1,
  DIRECTOR = 2,
  OBJECT_LINK = 3,
  LIGHTER = 4,
  HIGHLIGHT_REEL = 5,
};
enum class demoDollyCameraTimeScaleMode : int32_t {
  MANUAL = 0,
  LINEAR = 1,
  FIXED = 2,
  COUNT = 3,
};
enum class demoInformationScreenTypes : int32_t {
  NONE = 0,
  FILM_START_SCREEN_FADE_IN = 1,
  FILM_START_SCREEN_FADE_OUT = 2,
  TRANSITION = 3,
  WAITING_FOR_DOWNLOAD = 4,
  COUNT = 5,
};
enum class demoHighlightReelPlayerFilter : int32_t {
  ONLY_ME = 0,
  FRIENDS = 1,
  ME_AND_FRIENDS = 2,
  ONLY_FRIENDLY_TEAM = 3,
  ONLY_ENEMY_TEAM = 4,
  EVERYONE = 5,
  FIRST = 0,
  LAST = 5,
};
enum class demoHighlightReelTransitionFilter : int32_t {
  NONE = 0,
  FADE = 1,
  RANDOM = 2,
  FIRST = 0,
  LAST = 2,
};
enum class demoHighlightReelStylesFilter : int32_t {
  BEST_MOMENTS = 0,
  EXPLOSIVE_AND_PROJECTILE_KILLS = 1,
  VEHICLE_KILLS = 2,
  GUN_KILLS_ONLY = 3,
  GUN_KILLS_AND_EVENTS = 4,
  DOG_KILLS = 5,
  ZM_RAMPAGER = 6,
  ZM_SAVIOR_AND_SINNER = 7,
  ZM_TEAM_PLAYER = 8,
  ZM_BIG_SPENDER = 9,
  ZM_HANDYMAN = 10,
  FIRST = 0,
  LAST = 10,
};
enum class demoHighlightReelGameResultFilter : int32_t {
  OFF = 0,
  FINAL_RESULT = 1,
  WITH_EVENTS = 2,
  ALL_RESULTS = 3,
  FIRST = 0,
  LAST = 3,
};
enum class LobbyClientType : int32_t {
  INVALID = -1,
  ALL = 0,
  LOCAL = 1,
  REMOTE = 2,
  SPLITSCREEN_ALL = 3,
  SPLITSCREEN_REMOTE = 4,
  GUEST = 5,
};
enum class LobbyType : int32_t {
  INVALID = -1,
  PRIVATE = 0,
  GAME = 1,
  COUNT = 2,
  FIRST = 0,
  LAST = 1,
};
enum class LobbyModule : int32_t {
  INVALID = -1,
  HOST = 0,
  CLIENT = 1,
  COUNT = 2,
  FIRST = 0,
  LAST = 1,
  PEER_TO_PEER = 3,
};
enum class LobbyMode : int32_t {
  INVALID = -1,
  PUBLIC = 0,
  CUSTOM = 1,
  THEATER = 2,
  ARENA = 3,
  FREERUN = 4,
  COUNT = 5,
};
enum class LobbyNetworkMode : int32_t {
  INVALID = -1,
  UNKNOWN = 0,
  LAN = 1,
  LIVE = 2,
  COUNT = 3,
  LOCAL = 1,
};
enum class PartyPrivacy : int32_t {
  OPEN = 0,
  FRIENDS_ONLY = 1,
  INVITE_ONLY = 2,
  CLOSED = 3,
  COUNT = 4,
};
enum class LobbyJoinable : int32_t {
  NO_NOT_IN_LOBBY = 0,
  YES = 1,
  YES_FRIENDS_ONLY = 2,
  NO_PARTY_PRIVACY_INVITE_ONLY = 3,
  NO_PARTY_PRIVACY_CLOSED = 4,
  NO_PARTY_FULL = 5,
  NO_GAME_FULL = 6,
  NO_PLAYING_LOCAL = 7,
  NO_GAME_PAUSED = 8,
  NO = 9,
  NO_YOU_NEED_DLC = 10,
  COUNT = 11,
};
enum class LobbyMainMode : int32_t {
  INVALID = -1,
  CP = 0,
  MP = 1,
  ZM = 2,
  COUNT = 3,
};
enum class LobbyHostType : int32_t {
  INVALID = -1,
  P2P = 0,
  DEDICATED = 1,
  COUNT = 2,
};
enum class LobbyMapVote : int32_t {
  INVALID = -1,
  NONE = 0,
  NEXT = 1,
  PREVIOUS = 2,
  RANDOM = 3,
};
enum class LobbyProcessComplete : int32_t {
  INVALID = -1,
  SUCCESS = 0,
  FAILURE = 1,
  LOBBY_PROCESS_COMPLETE_ERROR = 2,
};
enum class LobbyDisconnectClient : int32_t {
  INVALID = -1,
  DROP = 0,
  KICK = 1,
  BADDLC = 2,
  KICK_PARTY = 3,
  HOSTRELOAD = 4,
  NOPARTYCHAT = 5,
  LOBBY_DISCONNECT_CLIENT_ERROR = 6,
  MISSING_CONTENT = 7,
};
enum class LiveUserState : int32_t {
  INVALID = -1,
  SIGNED_IN = 0,
  SIGNED_OUT = 1,
  COUNT = 2,
};
enum class SessionStatus : int32_t {
  INVALID = -1,
  IDLE = 0,
  JOIN = 1,
  MERGE = 2,
  MIGRATE = 3,
  PAUSED = 4,
  SESSION_STATUS_ERROR = 5,
  COUNT = 6,
};
enum class QueryID : int32_t {
  SESSIONS_INVALID = 0,
  SESSIONS_BY_PLAYLIST_VERSION = 1,
  SESSIONS_ALL = 2,
  SESSION_BY_GAME_SECURITY_ID = 3,
  SESSION_DEDICATED = 4,
  SESSION_DEDICATED_PARKED = 5,
  SESSIONS_BY_SERVER_TYPE = 6,
};
enum ShowInMatchmaking : int32_t {
  SHOW_IN_MATCHMAKING_FALSE = 0,
  SHOW_IN_MATCHMAKING_TRUE = 1,
};
enum class LaunchGameState : int32_t {
  START = 0,
  PUMP = 1,
  CLEAR = 2,
  COUNT = 3,
};
enum class LobbyOnlineState : int32_t {
  IDLE = 0,
  ACTIVE = 1,
  CREATE = 2,
  UPDATE = 3,
  LOBBY_ONLINE_STATE_DELETE = 4,
  LOBBY_ONLINE_STATE_ERROR = 5,
};
enum class LobbyOnlineUpdateEventType : int32_t {
  PUMP = 0,
  SUCCESS = 1,
  LOBBY_ONLINE_UPDATE_ERROR = 2,
};
enum class SessionActive : int32_t {
  INACTIVE = 0,
  KEEP_ALIVE = 1,
  ACTIVE = 2,
};
enum class SessionDirty : int32_t {
  STATE = 1,
  UI = 2,
  CLIENT_LIST = 4,
};
enum class PlatformSessions : int32_t {
  OFF = 0,
  ON_NONFATAL = 1,
  ON_NONFATAL_NO_MORE_SESSIONS = 2,
  ON_FATAL = 3,
  ON_FATAL_NO_MORE_SESSIONS = 4,
  COUNT = 4,
};
enum class SteamServerRequestType : int32_t {
  INTERNET = 0,
  LAN = 1,
  FRIENDS = 2,
  FAVORITES = 3,
  HISTORY = 4,
  SPECTATOR = 5,
  ZOMBIES = 6,
  COUNT = 7,
};
enum class SteamServerSortType : int32_t {
  PROTECTED_ASCENDING = 0,
  PROTECTED_DESCENDING = 1,
  DEDICATED_ASCENDING = 2,
  DEDICATED_DESCENDING = 3,
  RANKED_ASCENDING = 4,
  RANKED_DESCENDING = 5,
  NAME_ASCENDING = 6,
  NAME_DESCENDING = 7,
  MAP_ASCENDING = 8,
  MAP_DESCENDING = 9,
  HARDCORE_ASCENDING = 10,
  HARDCORE_DESCENDING = 11,
  GAMETYPE_ASCENDING = 12,
  GAMETYPE_DESCENDING = 13,
  PLAYERS_ASCENDING = 14,
  PLAYERS_DESCENDING = 15,
  PING_ASCENDING = 16,
  PING_DESCENDING = 17,
  COUNT = 18,
};
enum class SteamServerFilterType : int32_t {
  MAP = 0,
  GAMETYPE = 1,
  MOD = 2,
  KEYWORDS = 3,
  COUNT = 4,
};
enum class SteamServerAttributeFilter : int32_t {
  DEDICATED = 0,
  NOTDEDICATED = 1,
  RANKED = 2,
  UNRANKED = 3,
  HARDCORE = 4,
  CORE = 5,
  COUNT = 6,
};
enum class DurangoSessionJoinRestriction : int32_t {
  UNKNOWN = -1,
  NONE = -1,
  LOCAL = -1,
  FOLLOWED = -1,
};
enum class DurangoSessionVisibility : int32_t {
  UNKNOWN = -1,
  ANY = -1,
  PRIVATE = -1,
  VISIBLE = -1,
  FULL = -1,
  OPEN = -1,
};
enum class DurangoWriteSessionStatus : int32_t {
  UNKNOWN = -1,
  ACCESSDENIED = -1,
  CREATED = -1,
  CONFLICT = -1,
  HANDLENOTFOUND = -1,
  OUTOFSYNC = -1,
  SESSIONDELETED = -1,
  UPDATED = -1,
};
enum class LobbyListComparisonType : int32_t {
  EQUAL_TO_OR_LESS_THAN = -2,
  LESS_THAN = -1,
  EQUAL = 0,
  GREATER_THAN = 1,
  EQUAL_TO_OR_GREATER_THAN = 2,
  NOT_EQUAL = 3,
};
enum class GameServerStatus : int32_t {
  IDLE = 0,
  PRELOADING = 1,
  STARTING_SERVER = 2,
  MIGRATING = 3,
  RUNNING = 4,
  RUNNING_DEMO = 5,
};
enum class JoinType : int32_t {
  NORMAL = 0,
  PLAYLIST = 1,
  FRIEND = 2,
  INVITE = 3,
  PARTY = 4,
  GROUPS = 5,
  COUNT = 6,
};
enum class JoinResult : int32_t {
  INVALID = 0,
  SUCCESS = 1,
  CONNECT_TO_HOST_FAILURE = 2,
  PROBE_SEND_FAILURE = 3,
  PROBE_TIMEOUT = 4,
  PROBE_INVALID_LOBBY = 5,
  PROBE_INVALID_INFO = 6,
  PROBE_RESULT_INVALID = 7,
  INVALID_LOBBY = 8,
  SEND_AGREEMENT_REQUEST_FAILED = 9,
  HANDSHAKE_WINDOW_EXPIRED = 10,
  AGREEMENT_WINDOW_EXPIRED = 11,
  JOIN_DISABLED = 12,
  JOIN_ALREADY_IN_PROGRESS = 13,
  NOT_JOINABLE_OFFLINE_MODE = 14,
  NOT_JOINABLE_NOT_HOSTING = 15,
  NOT_JOINABLE_NOT_IDLE = 16,
  NOT_JOINABLE_CLOSED = 17,
  NOT_JOINABLE_INVITE_ONLY = 18,
  NOT_JOINABLE_FRIENDS_ONLY = 19,
  NOT_JOINABLE_SOLO_MODE = 20,
  NO_JOIN_IN_PROGRESS = 21,
  GAME_PAUSED = 22,
  CHUNK_MP_REQUIRED = 23,
  CHUNK_ZM_REQUIRED = 24,
  CHUNK_CP_REQUIRED = 25,
  CHUNK_MP_REQUIRED_HOST = 26,
  CHUNK_ZM_REQUIRED_HOST = 27,
  CHUNK_CP_REQUIRED_HOST = 28,
  SPLITSCREEN_NOT_ALLOWED = 29,
  IN_PUBLIC_PROBATION = 30,
  IN_ARENA_PROBATION = 31,
  OVER_MAX_PARTY_LIMIT = 32,
  NO_PARTIES = 33,
  LOBBY_FULL = 34,
  NETWORK_MODE_MISMATCH = 35,
  MISMATCH_PLAYLISTID = 36,
  MISMATCH_PLAYLIST_VERSION_TO_NEW = 37,
  MISMATCH_PLAYLIST_VERSION_TO_OLD = 38,
  MISMATCH_PROTOCOL_VERSION = 39,
  MISMATCH_NETFIELD_CHECKSUM = 40,
  MISMATCH_FFOTD_VERSION_TO_NEW = 41,
  MISMATCH_FFOTD_VERSION_TO_OLD = 42,
  MIGRATE_IN_PROGRESS = 43,
  COULD_NOT_RESERVE = 44,
  BAD_DLC_BITS = 45,
  BAD_MPHD_BITS = 46,
  VM_FAILURE_1 = 47,
  VM_FAILURE_2 = 48,
  VM_FAILURE_3 = 49,
  VM_FAILURE_4 = 50,
  STARTER_PACK_RESTRICT = 51,
  INVALID_PASSWORD = 52,
  BANNED = 53,
  BANNED_MEMBER = 54,
  UNDER_MIN_PARTY_LIMIT = 55,
  COUNT = 56,
};
enum class LobbyTaskType : int32_t {
  HOST_CREATE = 0,
  HOST_ADDLOCAL = 1,
  HOST_END = 2,
  HOST_SEARCH = 3,
  HOST_ADVERTISE = 4,
  HOST_GAMERTAGSTOXUIDS = 5,
  CLIENT_END = 6,
  CLIENT_LAUNCH_GAME = 7,
  COUNT = 8,
};
enum class LobbyTaskState : int32_t {
  IDLE = 0,
  HOSTCREATE_PENDING = 1,
  HOSTCREATE_DWNET = 2,
  HOSTCREATE_SESSION = 3,
  HOSTEND_PENDING = 4,
  HOSTEND_ONLINE = 5,
  HOSTEND_DISCONNECT = 6,
  HOSTEND_COOLOFF = 7,
  HOSTEND_SHUTDOWN = 8,
  HOSTSEARCH_PENDING = 9,
  HOSTSEARCH_INPROGRESS = 10,
  HOSTSEARCH_BEGINJOIN = 11,
  HOSTSEARCH_JOINING = 12,
  HOSTADVERTISE_PENDING = 13,
  HOSTADVERTISE_INPROGRESS = 14,
  HOSTPLATFORMSESSIONCREATE = 15,
  HOSTPLATFORMSESSIONUPDATE = 16,
  HOSTPLATFORMSESSIONINVITE = 17,
  HOSTPLATFORMSESSIONGETSESSIONINFO = 18,
  HOSTPLATFORMSESSIONGETINVITEINFO = 19,
  HOSTPLATFORMSESSIONLEAVE = 20,
  HOSTPLATFORMSESSIONSEARCH = 21,
  HOSTPLATFORMSESSIONJOIN = 22,
  HOSTPLATFORMSESSIONSETDURANGOJOINRESTRICTION = 23,
  HOSTGAMERTAGSTOXUIDS = 24,
  CLIENTEND_PENDING = 25,
  CLIENTEND_DISCONNECT = 26,
  CLIENTEND_COOLOFF = 27,
  CLIENTEND_SHUTDOWN = 28,
  HOSTLAUNCH_INIT = 29,
  HOSTLAUNCH_PUMP = 30,
  HOSTLAUNCH_PRELOAD = 31,
  HOSTLAUNCH_GO = 32,
  CLIENTLAUNCH_INIT = 33,
  CLIENTLAUNCH_PUMP = 34,
  CLIENTLAUNCH_GO = 35,
  SUCCESS = 36,
  FAILURE = 37,
};

enum class UIVisibilityBit : int32_t {
  BIT_HUD_VISIBLE = 0,
  BIT_WEAPON_HUD_VISIBLE = 1,
  BIT_G_COMPASS_SHOW_ENEMIES = 2,
  BIT_RADAR_CLIENT = 3,
  BIT_NEMESIS_KILLCAM = 4,
  UI_CLIENT_HIGHEST_VISIBILITY_BIT = 4,
  BIT_FINAL_KILLCAM = 5,
  BIT_ROUND_END_KILLCAM = 6,
  BIT_RADAR_ALLIES = 7,
  BIT_RADAR_AXIS = 8,
  BIT_ENABLE_POPUPS = 9,
  BIT_BOMB_TIMER = 10,
  BIT_BOMB_TIMER_A = 11,
  BIT_BOMB_TIMER_B = 12,
  BIT_AMMO_COUNTER_HIDE = 13,
  BIT_HUD_HARDCORE = 14,
  BIT_DRAW_SPECTATOR_MESSAGES = 15,
  BIT_DISABLE_INGAME_MENU = 16,
  BIT_GAME_ENDED = 17,
  BIT_OVERTIME = 18,
  UI_CLIENT_AND_MATCH_HIGHEST_VISIBILITY_BIT = 18,
  BIT_DEMO_CAMERA_MODE_THIRDPERSON = 19,
  BIT_DEMO_CAMERA_MODE_MOVIECAM = 20,
  BIT_DEMO_ALL_GAME_HUD_HIDDEN = 21,
  BIT_DEMO_HUD_HIDDEN = 22,
  BIT_IN_KILLCAM = 23,
  BIT_SELECTING_LOCATION = 24,
  BIT_IS_FLASH_BANGED = 25,
  BIT_UI_ACTIVE = 26,
  BIT_SPECTATING_CLIENT = 27,
  BIT_IS_SCOPED = 28,
  BIT_IN_VEHICLE = 29,
  BIT_IN_GUIDED_MISSILE = 30,
  BIT_IS_FUEL_WEAPON = 31,
  BIT_SELECTING_LOCATIONAL_KILLSTREAK = 32,
  BIT_IS_DEMO_PLAYING = 33,
  BIT_IS_DEMO_MOVIE_RENDERING = 34,
  BIT_ADS_JAVELIN = 35,
  BIT_EXTRACAM_ON = 36,
  BIT_EXTRACAM_ACTIVE = 37,
  BIT_EXTRACAM_STATIC = 38,
  BIT_TEAM_FREE = 39,
  BIT_TEAM_ALLIES = 40,
  BIT_TEAM_AXIS = 41,
  BIT_TEAM_SPECTATOR = 42,
  BIT_COMPASS_VISIBLE = 43,
  BIT_HUD_SHOWOBJICONS = 44,
  BIT_SCOREBOARD_OPEN = 45,
  BIT_POPUPS_VISIBLE = 46,
  BIT_HUD_OBITUARIES = 47,
  BIT_POF_SPEC_ALLOW_FREELOOK = 48,
  BIT_POF_FOLLOW = 49,
  BIT_IN_REMOTE_KILLSTREAK_STATIC = 50,
  BIT_EMP_ACTIVE = 51,
  BIT_TOTAL_COVERAGE_ACTIVE = 52,
  BIT_COUNTER_UAV_ACTIVE = 53,
  BIT_IS_THIRD_PERSON = 54,
  BIT_PLAYER_DEAD = 55,
  BIT_IN_REMOTE_MISSILE = 56,
  BIT_IS_PLAYER_ZOMBIE = 57,
  BIT_IS_PLAYER_IN_AFTERLIFE = 58,
  BIT_CAMERA_ANIM_HIDING_HUD = 59,
  BIT_MIGRATING_HOST = 60,
  BIT_HIDE_FOR_FULLSCREEN_MENU = 61,
  BIT_DRAW_RETICLE = 62,
  UI_VISIBILITY_BIT_COUNT = 63,
};
enum class ObjectiveTypes : int32_t {
  INVALID = -1,
  WAYPOINT = 0,
  _3DPROMPT = 1,
  NOINDICATOR = 2,
  WOUNDED_SOLDIER = 3,
  COUNT = 4,
};
enum class WeaponFireEvent : int32_t {
  CHANGE = 0,
  FIRING = 1,
  FIRED = 2,
  CANCELLED = 3,
};
enum class WoundedSoldierType : int32_t {
  MINOR = 0,
  MEDIUM = 1,
  SEVERE = 2,
  CRITICAL = 3,
  COUNT = 4,
};
enum class RocketLauncherTargetStatesFlags : int32_t {
  FOUND = 0,
  LOCKING = 1,
  LOCKED = 2,
  OUT_OF_RANGE = 3,
};
enum class BleedOutStateFlags : int32_t {
  BLEEDING_OUT = 0,
  BEING_REVIVED = 1,
  CLAMPED = 2,
};
enum class VehiclePartDestroyed : int32_t {
  ONE = 0,
  TWO = 1,
  COUNT = 2,
};
enum class MissileLockedOnDirection : int32_t {
  CENTER = 0,
  UP = 1,
  DOWN = 2,
  LEFT = 3,
  RIGHT = 4,
  COUNT = 5,
};
enum class VehicleFullscreenFilter : int32_t {
  NONE = 0,
  INFRARED = 1,
  FLIR = 2,
};
enum class WeakpointWidgetStates : int32_t {
  DEFAULT = 0,
  CLOSE = 1,
  MEDIUM = 2,
  WWS_FAR = 3,
  OBSCURED = 4,
  HIDDEN = 5,
  CLOSING = 6,
};
enum class EnemyTargetStates : int32_t {
  NONE = 0,
  LOCKED = 1,
  OUT_OF_RANGE = 2,
};
enum class LUIAlignment : int32_t {
  NONE = 0,
  LEFT = 1,
  CENTER = 2,
  RIGHT = 4,
  TOP = 8,
  MIDDLE = 16,
  BOTTOM = 32,
  SCREEN_SPACE = 64,
  HORIZONTAL_MASK = 7,
  VERTICAL_MASK = 56,
};
enum class UIModelClientFieldType : int32_t {
  INT = 0,
  FLOAT = 1,
  COUNTER = 2,
};
enum class ArmbladeLockState : int32_t {
  STATE_NONE = 0,
  TARGETTED = 1,
  ATTACKING = 2,
};
enum class SettingTeamIndicator : int32_t {
  FULL = 0,
  ABBREVIATED = 1,
  ICON = 2,
  COUNT = 3,
};
enum class DamageFeedbackState : int32_t {
  NONE = 0,
  ONE = 1,
  TWO = 2,
  THREE = 3,
  FOUR = 4,
  FIVE = 5,
  COUNT = 6,
  DFS_IGNORE = 6,
  NEXT_UNUSED_BIT = 4,
  MASK = 7,
};
enum class DamageFeedbackFlags : int32_t {
  FLAK = 0,
  TAC_MASK = 1,
  ARMOR = 2,
  DEAD = 3,
};
enum class ColorVisionDeficiencies : int32_t {
  OFF = 0,
  DEUTERANOMALY = 1,
  PROTANOMALY = 2,
  TRITANOMALY = 3,
  COUNT = 4,
};
enum class HlsChannelState_e : int32_t {
  LIVE = 0,
  REPLAY = 1,
  OFFLINE = 2,
  NOT_FOUND = 3,
};
enum class LMGInfiniteState : int32_t {
  PRINTING = 0,
  FIRING = 1,
  FULL = 2,
  EMPTY = 3,
};
enum class CompassType : int32_t {
  PARTIAL = 0,
  FULL = 1,
  PARTIAL_WORLDALIGNED = 2,
};
enum class bunkCollectibleSizes_e : int32_t {
  SMALL = 0,
  MEDIUM = 1,
  LARGE = 2,
  COUNT = 3,
};
enum class KeyboardType : int32_t {
  NONE = 0,
  DEMO = 1,
  EMAIL = 2,
  CUSTOM_CLASS = 3,
  CLASS_SET = 4,
  BUBBLEGUM_PACK = 5,
  LEAGUES = 6,
  TWITCH_USER = 7,
  TWITCH_PASS = 8,
  TEXT_MESSAGE = 9,
  ADD_FRIEND = 10,
  REGISTRATION_INPUT_PASSWORD = 11,
  REGISTRATION_INPUT_ACCENTS = 12,
  REGISTRATION_INPUT_NUMERIC = 13,
  REGISTRATION_INPUT_ACCOUNTNAME = 14,
  NUMERIC_FIELD = 15,
  CLAN_TAG = 16,
  TWITTER_USER = 17,
  TWITTER_PASS = 18,
  PASSWORD = 19,
  REGISTER_FIRST_NAME = 20,
  REGISTER_LAST_NAME = 21,
  REGISTER_ZIP_CODE = 22,
  REGISTER_USERNAME = 23,
  REGISTER_EMAIL = 24,
  REGISTER_PASSWORD = 25,
  CONFIRM_PASSWORD = 26,
  FILESHARE_DOWNLOAD_NAME = 27,
  FILESHARE_PUBLISH_NAME = 28,
  FILESHARE_PUBLISH_DESCRIPTION = 29,
  FILESHARE_PUBLISH_TAGS = 30,
  GUNSMITH = 31,
  EMBLEMS = 32,
  PAINTJOB = 33,
  TOTP = 34,
  TEAM_IDENTITY_TEAMNAME = 35,
  SERVER_NAME = 36,
  SERVER_DESCRIPTION = 37,
  SERVER_PASSWORD = 38,
  SERVER_CLIENT_PASSWORD = 39,
  SERVER_FILTER_KEYWORDS = 40,
  COUNT = 41,
};
enum class LbFilter : int32_t {
  NONE = 0,
  FRIENDS = 1,
  LOBBY_MEMBERS = 2,
  CUSTOM = 3,
  COUNT = 4,
};
enum class Limit : int32_t {
  LIMIT_INT_MAX = 0,
  LIMIT_UINT_MAX = 1,
  LIMIT_INT64_MAX = 2,
  LIMIT_UINT64_MAX = 3,
};
enum class LbTrackType : int32_t {
  ALLTIME = 0,
  WEEKLY = 1,
  MONTHLY = 2,
  PRESTIGE_ALLTIME = 3,
  PRESTIGE_WEEKLY = 4,
  PRESTIGE_MONTHLY = 5,
  DAILY = 6,
  PRESTIGE_DAILY = 7,
  COUNT = 8,
};
enum class StoreProductPurchaseStatus : int32_t {
  NOT_PURCHASED = 0,
  PURCHASED = 1,
};
enum class CustomizationType : int32_t {
  PAINTSHOP_VIEW_LEFT = 0,
  PAINTSHOP_VIEW_TOP = 1,
  PAINTSHOP_VIEW_RIGHT = 2,
  EMBLEM = 3,
  COUNT = 4,
};
enum class CustomizationScaleType : int32_t {
  FIXED = 0,
  FREE = 1,
};
enum CustomizationTypeLayerCount : int32_t {
  CUSTOMIZATION_TYPE_EMBLEM_MAX_LAYERS = 64,
  CUSTOMIZATION_TYPE_PAINTSHOP_MAX_LAYERS = 64,
};
enum CustomizationPaintjobInvalidID : int32_t {
  CUSTOMIZATION_INVALID_PAINTJOB_INDEX = 15,
  CUSTOMIZATION_INVALID_PAINTJOB_SLOT = 15,
};
enum CustomizationTypeGroupCount : int32_t {
  CUSTOMIZATION_TYPE_MAX_GROUPS = 11,
};
enum class CustomizationColorSwatch : int32_t {
  ROWS = 9,
  COLUMNS = 13,
};
enum class CustomizationEditorMode : int32_t {
  BROWSE = 0,
  EDIT = 1,
  NONE = 2,
  COUNT = 3,
};
enum class CustomizationColorMode : int32_t {
  MIXER = 0,
  SOLID = 1,
  NONE = 2,
  COUNT = 3,
};
enum class CustomizationColorNum : int32_t {
  ZERO = 0,
  ONE = 1,
  GRADIENT = 2,
  NONE = 3,
  COUNT = 4,
};
enum class CustomizationGradientType : int32_t {
  LINEAR = 0,
  RADIAL = 1,
  COUNT = 2,
};
enum class GunsmithMode : int32_t {
  WEAPONLIST = 0,
  VARIANTS = 1,
};
enum class PaintshopMode : int32_t {
  WEAPONLIST = 0,
  PAINTJOBS = 1,
};
enum class ShowcaseWeaponMode : int32_t {
  WEAPONLIST = 0,
  VARIANTS = 1,
};
enum class MessageDialogType : int32_t {
  INFO = 0,
  WARNING = 1,
  MESSAGE_DIALOG_TYPE_ERROR = 2,
  NOTICE = 3,
};

enum class CACType : int32_t {
  INVALID = -1,
  MP_PUBLIC = 0,
  MP_CUSTOM = 1,
  MP_ARENA = 2,
  MP_OFFLINE = 3,
  FR = 4,
  FR_OFFLINE = 5,
  CP_ONLINE = 6,
  CP_OFFLINE = 7,
  ZM_ONLINE = 8,
  ZM_OFFLINE = 9,
  COUNT = 10,
};
enum class ClassSetType : int32_t {
  INVALID = -1,
  MP_PUBLIC = 0,
  MP_CUSTOM = 1,
  MP_ARENA = 2,
  COUNT = 3,
};
enum class eGameType : int32_t {
  COOP = 0,
  DOA = 1,
  CPZM = 2,
  COUNT = 3,
  INVALID = 3,
};
enum class LobbyPregameState : int32_t {
  IDLE = 0,
  ITEM_VOTE = 1,
  CHARACTER_DRAFT = 2,
  CAC_EDIT = 3,
  SCORESTREAK_EDIT = 4,
  LAUNCH_GAME = 5,
  COUNT = 6,
};
enum class LobbyClientPregameState : int32_t {
  IDLE = 0,
  SELECTING = 1,
  SELECTED = 2,
  SELECTION_FORCED = 3,
  SELECTION_COMPLETE = 4,
  RESET = 5,
  END = 6,
  COUNT = 7,
};
enum class VoteType : int32_t {
  INVALID = 0,
  BAN = 1,
  PROTECT = 2,
  COUNT = 3,
};
enum class VoteItemType : int32_t {
  INVALID = 0,
  ITEM = 1,
  ATTACHMENT = 2,
  ITEM_ATTACHMENT = 3,
  ATTACHMENT_GROUP = 4,
  COUNT = 5,
};
enum class PregameSortType : int32_t {
  RANDOM = 0,
  CLIENTNUM = 1,
  CODCASTER = 2,
  COUNT = 3,
};
enum class PregameShuffleMethod : int32_t {
  NONE = 0,
  REVERSE = 1,
  RANDOM = 2,
  COUNT = 3,
};
enum class ItemRestrictionState : int32_t {
  NONE = 0,
  RESTRICTED = 1,
  BANONLY = 2,
  PROTECTED = 3,
};
enum class ProbationMode : int32_t {
  PUBLIC = 0,
  FIRST = 0,
  ARENA = 1,
  COUNT = 2,
};
enum class heroLoadoutTypes_e : int32_t {
  INVALID = -1,
  FIRST = 0,
  WEAPON = 0,
  GADGET = 1,
  COUNT = 2,
};
enum class HeroState : int32_t {
  CHARGING = 0,
  READY = 1,
  PRIMED = 2,
  ACTIVE = 3,
  FLICKERING = 4,
};
enum class eWeaponOptionGroup : int32_t {
  FIRST = 0,
  CAMO = 0,
  TAG = 1,
  EMBLEM = 2,
  PAINTJOB = 3,
  FIRST_ATTACHMENT_OPTION = 4,
  RETICLE = 4,
  LENS = 5,
  RETICLE_COLOR = 6,
  LAST_ATTACHMENT_OPTION = 6,
  COUNT = 7,
  INVALID = 7,
};
enum class itemGroup_t : int32_t {
  SMG = 0,
  FIRST = 0,
  ASSAULT = 1,
  CQB = 2,
  LMG = 3,
  SNIPER = 4,
  PISTOL = 5,
  LAUNCHER = 6,
  KNIFE = 7,
  SPECIAL = 8,
  SHOTGUN = 9,
  EXPLOSIVE = 10,
  GRENADE = 11,
  MASTERKEY = 12,
  GRENADELAUNCHER = 13,
  FLAMETHROWER = 14,
  SPECIALTY = 15,
  SPECIALGRENADE = 16,
  MISCWEAPON = 17,
  FEATURE = 18,
  BONUSCARD = 19,
  CYBERCOM_TACRIG = 20,
  GLOBAL_ITEMS_START = 21,
  KILLSTREAK = 21,
  HERO = 22,
  BUBBLEGUM = 23,
  BUBBLEGUM_CONSUMABLE = 24,
  CYBERCOM_CORE = 25,
  CYBERCOM_ABILITIES = 26,
  COUNT = 27,
  INVALID = -1,
};
enum class prestigeTokenType_e : int32_t {
  RESPEC = 0,
  RESET_STATS = 1,
  EXTRA_CAC = 2,
  COUNT = 3,
};
enum class consoleLabel : int32_t {
  DEFAULT = 0,
  LIVE = 4,
  LIVE_XBOX = 5,
  LIVE_PS4 = 6,
  LIVE_MATCHMAKING = 7,
  DEMONWARE = 8,
  LEADERBOARDS = 9,
  LOBBY = 10,
  LOBBYHOST = 11,
  LOBBYCLIENT = 12,
  LOBBYVM = 13,
  MIGRATION = 14,
};
enum class errorCode : int32_t {
  NONE = 0,
  FATAL = 1,
  DROP = 2,
  FROM_STARTUP = 4,
  SERVERDISCONNECT = 8,
  DISCONNECT = 16,
  SCRIPT = 32,
  SCRIPT_DROP = 64,
  LOCALIZATION = 128,
  UI = 256,
  LUA = 512,
  SOFTRESTART = 1024,
  SOFTRESTART_KEEPDW = 2048,
  SOFTRESTART_SILENT = 4096,
};
enum class eVoipStatus : int32_t {
  HIDDEN = 0,
  TALKING = 1,
  NOTTALKING = 2,
  MUTED = 3,
  ENEMY_TALKING = 4,
};
enum class objectiveState_t : int32_t {
  EMPTY = 0,
  ACTIVE = 1,
  INVISIBLE = 2,
  DONE = 3,
  CURRENT = 4,
  FAILED = 5,
  NUMSTATES = 6,
};

enum class scoreBoardColumns_e : int32_t {
  XUID = 0,
  RANK_BG = 1,
  RANK = 2,
  RANK_ICON = 3,
  NAME = 4,
  ONE = 5,
  TWO = 6,
  THREE = 7,
  FOUR = 8,
  FIVE = 9,
  PING_BAR = 10,
  CLIENTNUM = 11,
  PING = 12,
};
enum class WeaponAttributesColumn : int32_t {
  GROUP = 0,
  REFERENCE = 1,
  ACCURACY = 2,
  DAMAGE = 3,
  RANGE = 4,
  FIRE_RATE = 5,
  MOBILITY = 6,
  MAG_SIZE = 7,
  FIRE_TYPE = 8,
  WEAPON_NAME = 9,
  COUNT = 10,
};
enum class attachmentTableColumn_e : int32_t {
  INDEX = 0,
  ATTACHPOINT = 1,
  GROUP = 2,
  NAME = 3,
  REFERENCE = 4,
  ITEM_COUNT = 5,
  IMAGE = 6,
  DESCRIPTION = 7,
  CP_DESCRIPTION = 8,
  DATA_STRING = 9,
  DATA_INT = 10,
  DATA_INT2 = 11,
  COMPATIBLE_ATTACHMENTS = 12,
  ALLOCATION = 13,
  UNLOCK_LEVEL = 14,
  UNLOCK_PLEVEL = 15,
  BOT_PROBABILITY = 16,
  DEFAULT_PRIMARY_CLASS = 17,
  DEFAULT_SECONDARY_CLASS = 18,
  ELITE = 19,
  WEAPONOPTION_FILTER = 20,
  ENTITLEMENT_NAME = 21,
  SPECIAL_CONTRACT_INDEX = 22,
  LIMITED_ITEM = 23,
  COUNT = 24,
};
enum class KVSChannel : int32_t {
  MATCHMAKING_QOS = -1,
  CLIENT_STATS = -1,
};
enum class CustomGamesTaskUIState : int32_t {
  IDLE = 0,
  WORKING = 1,
  DONE = 2,
  CUSTOMGAMES_TASK_UI_ERROR = 3,
};
enum class fileShareLocation : int32_t {
  INVALID = 0,
  PUBLISHERSTORAGE = 1,
  USERSTORAGE = 2,
  POOLEDSTORAGE = 3,
};
enum class FileshareTaskUIState : int32_t {
  IDLE = 0,
  WORKING = 1,
  DONE = 2,
  FILESHARE_TASK_UI_ERROR = 3,
};
enum class FileshareFileCategory : int32_t {
  INVALID = -1,
  ALL = 0,
  EMBLEM = 1,
  PAINTJOB = 2,
  VARIANT = 3,
};
enum class MediaType : int32_t {
  FILESHARE = 0,
  LOCAL = 1,
};
enum class FileshareCommunityDataType : int32_t {
  INVALID = 0,
  POPULAR = 1,
  RECENT = 2,
  TRENDING = 3,
  RECENT_GAMES = 4,
};
enum class FileshareGroupsDataType : int32_t {
  INVALID = 0,
  RECENT = 1,
  FAVORITE = 2,
  FEATURED = 3,
};
enum class FileshareMode : int32_t {
  USER = 0,
  COMMUNITY = 1,
  GROUPS = 2,
};
enum class ScreenshotViewType : int32_t {
  SCREENSHOT = 0,
  THUMBNAIL = 1,
  MOTD = 2,
  EMBLEM = 3,
  COUNT = 4,
};
enum class ShoutcasterSettingsXray : int32_t {
  OFF = 0,
  ENEMY = 1,
  ALL = 2,
  COUNT = 3,
};
enum class ShoutcasterSettingShowPerks : int32_t {
  OFF = 0,
  ON = 1,
  ALWAYS = 2,
  STANDARD = 3,
  COUNT = 4,
};
enum class ShoutcasterSettingMinimapMode : int32_t {
  OFF = 0,
  STANDARD = 1,
  FULL = 2,
  COUNT = 3,
};
enum class ShoutcasterSettingTeamIdentityMode : int32_t {
  FACTION = 0,
  CUSTOM = 1,
  COUNT = 2,
};
enum class dlcIndex_t : int32_t {
  DEV_MAP_INDEX = -1,
  ORIGINAL_MAP_INDEX = 0,
  DLC0ZM_INDEX = 1,
  DLC0MP_INDEX = 2,
  DLC1_INDEX = 3,
  DLC2_INDEX = 4,
  DLC3_INDEX = 5,
  DLC4_INDEX = 6,
  DLC5_INDEX = 7,
  DLC1ZM_INDEX = 8,
  DLC2ZM_INDEX = 9,
  DLC3ZM_INDEX = 10,
  DLC4ZM_INDEX = 11,
  DLC6_INDEX = 12,
  DLCPC_INDEX = 13,
  DLC_INDEX_COUNT = 14,
  USERMAP_INDEX = 15,
};
enum class LUIButton : int32_t {
  NONE = 0,
  LEFT = 1,
  UP = 2,
  RIGHT = 3,
  DOWN = 4,
  XBA_PSCROSS = 5,
  XBB_PSCIRCLE = 6,
  XBX_PSSQUARE = 7,
  XBY_PSTRIANGLE = 8,
  BACK = 9,
  START = 10,
  LB = 11,
  RB = 12,
  LTRIG = 13,
  RTRIG = 14,
  LSTICK_PRESSED = 15,
  RSTICK_PRESSED = 16,
  PCKEY_MWHEELUP = 17,
  PCKEY_MWHEELDOWN = 18,
  PCKEY_0 = 19,
  PCKEY_1 = 20,
  PCKEY_2 = 21,
  COUNT = 22,
};
enum class LUIButtonFlags : int32_t {
  DOWN = 1,
  DPAD = 2,
  ISREPEAT = 4,
};
enum class LUIButtonPromptStates : int32_t {
  HIDE_PROMPTS = 0,
  ENABLE_PROMPTS = 1,
  DISABLE_PROMPTS = 2,
};
enum class gamepadType_e : int32_t {
  ORBIS = 0,
  DURANGO = 1,
  STEAM = 2,
  COUNT = 3,
};
enum class chatChannel_e : int32_t {
  ALL = 0,
  TEAM = 1,
  PARTY = 2,
  COUNT = 3,
};
enum class chatInGameState_e : int32_t {
  PLAYING = 0,
  SPECTATOR = 1,
  DEAD = 2,
  LOBBY = 3,
  COUNT = 4,
};
enum class HostServer : int32_t {
  SERVER_OK = 0,
  SERVER_BANDWIDTH_TEST_IN_PROGRESS = 1,
  SERVER_NOT_HOST_OF_PARTY = 2,
  SERVER_DEDICATED_ONLY = 3,
  SERVER_NAT_TYPE_NOT_ALLOWED = 4,
  SERVER_WE_SHOULD_NOT_HOST = 5,
  SERVER_IS_NOT_GOOD_CITIZEN = 6,
  SERVER_HAS_NOT_NECESSARY_BANDWIDTH = 7,
  SERVER_NET_ENGINE_NOT_STARTED = 8,
  SERVER_LIVE_STREAMING = 9,
  SERVER_BANNED_FROM_HOSTING = 10,
  NEVER_HOST_SERVER_DVAR = 11,
  SERVER_IS_IN_SHARE_PLAY = 12,
};
enum class StorageResult : int32_t {
  SUCCESS = 0,
  FAILURE_ERROR = 1,
  FAILURE_DDL = 2,
  FAILURE_NOTFOUND = 3,
};
enum class BuildIntField : int32_t {
  BUILD_NUMBER = 0,
  CHANGELIST = 1,
  INFO_VERSION = 2,
};
enum class BuildStringField : int32_t {
  MAJOR_VERSION = 0,
  MINOR_VERSION = 1,
  BUILD_ID = 2,
  BUILD_VERSION = 3,
  BUILD_MACHINE = 4,
  BUILD_TYPE = 5,
  BUILD_TIME = 6,
  BUILD_DISPLAY_NAME = 7,
  BUILD_NAME = 8,
  BUILD_BASE_NAME = 9,
  BUILD_CONFIG = 10,
  BUILD_MODE = 11,
  BUILD_PROJECT_NAME = 12,
  BUILD_LOCAL_CHANGES = 13,
  BUILD_INFO_NAME = 14,
  BUILD_INFO_SOURCE = 15,
  BUILD_INFO_STRING = 16,
};
enum class LiveMetPlayerUpdate : int32_t {
  INTERNAL = 0,
  RESET = 1,
  DOWNLOAD = 2,
  COUNT = 3,
};
enum class statsMilestoneTypes_t : int32_t {
  GLOBAL = 0,
  FIRST = 0,
  WEAPON = 1,
  GAMEMODE = 2,
  GROUP = 3,
  ATTACHMENTS = 4,
  SPECIALIST = 5,
  DAILY = 6,
  COUNT = 7,
  INVALID = 7,
};
enum class milestoneTableColumns_t : int32_t {
  INDEX = 0,
  TIERID = 1,
  TARGETVALUE = 2,
  STATTYPE = 3,
  STATNAME = 4,
  STRING = 5,
  XPEARNED = 6,
  PREREQUISITE = 7,
  EXCLUDE = 8,
  UNLOCKITEM = 9,
  CHALLENGESTATNAME = 10,
  UNLOCKLVL = 11,
  UNLOCKIMAGE = 12,
  INCLUDE = 13,
  UNLOCKPLVL = 14,
  RESETONPRESTIGE = 15,
  CATEGORY = 16,
  ISMAXLEVEL = 17,
  ISMASTERY = 18,
  ISITEMUNLOCKED = 19,
  ISEXPERT = 20,
  SETGLOBALSTAT = 21,
  XPGROUPMASK = 22,
  UNLOCKHEROITEM = 23,
  VIALSEARNED = 24,
};
enum class PlatformPresenceFeatures : int32_t {
  NONE = 0,
  BASIC = 1,
  INTITLE = 2,
  EXTENDED_DW = 3,
  EXTENDED_PLATFORM = 4,
  COUNT = 5,
};
enum class InventoryCurrency : int32_t {
  INVALID = -1,
  COD_POINTS = 0,
  MP_LOOT_XP = 1,
  MP_CRYPTO_KEYS = 2,
  ZM_VIALS = 3,
  MP_BUNDLE_ITEM = 4,
  _200CP_COMPENSATION = 5,
  _1100CP_COMPENSATION = 6,
  _2400CP_COMPENSATION = 7,
  _5000CP_COMPENSATION = 8,
  _13000CP_COMPENSATION = 9,
  CP_COMPENSATION = 10,
  RESERVE_2 = 11,
  RESERVE_3 = 12,
  RESERVE_4 = 13,
  RESERVE_5 = 14,
  COUNT = 15,
};
enum class LootCrateType : int32_t {
  COMMON = 0,
  RARE = 1,
  BRIBE = 2,
  COUNT = 3,
};
enum class WeaponOptionFilter : int32_t {
  ZM = 0,
  MP = 1,
  CP = 2,
  MODE_COUNT = 3,
  BM = 3,
  EXTRAS = 4,
  COUNT = 5,
  FIRST = 0,
};
enum class MultiplayerMusicTracks : int32_t {
  NONE = 0,
  DEFAULT = 1,
  RANDOM = 2,
  FIRST_TRACK = 3,
  MAX = 64,
};
enum class MusicPlayerTableColumn : int32_t {
  INDEX = 0,
  ALIAS = 1,
  TITLE = 2,
  ARTIST = 3,
  ARTIST_2 = 4,
  FILTERS = 5,
};
enum class FrontendMusicTrackStates : int32_t {
  DEFAULT = 0,
  ON = 1,
  OFF = 2,
  COUNT = 3,
};
enum class InventoryPurchaseResult : int32_t {
  INVALID = 0,
  INPROGRESS = 1,
  SUCCESS = 2,
  FAILURE = 3,
};
enum class LootBurnDupeState : int32_t {
  IDLE = 0,
  REQUESTED = 1,
  BUSY = 2,
  DONE = 3,
  FAILED = 4,
};
enum class ContentFlagBits : int32_t {
  UNKNOWN = 1,
  ORIGINALMAPS = 2,
  DLC0ZM = 4,
  DLC0MP = 8,
  DLC1 = 16,
  DLC2 = 32,
  DLC3 = 64,
  DLC4 = 128,
  DLC5 = 256,
  DLC1ZM = 512,
  DLC2ZM = 1024,
  DLC3ZM = 2048,
  DLC4ZM = 4096,
  DLC6 = 0x2000,
  DLCPC = 0x4000,
};
enum class GPadButtonConfigs : int32_t {
  DEFAULT = 0,
  EXPERIMENTAL = 1,
  LEFTY = 2,
  NOMAD = 3,
  CHARLIE = 4,
  GUNSLINGER = 5,
  BUMPERJUMPER = 6,
  BUMPERTACTICAL = 7,
  STICKMOVE = 8,
  COUNT = 9,
};
enum class LootRarityType : int32_t {
  COMMON = 0,
  RARE = 1,
  LEGENDARY = 2,
  EPIC = 3,
  ZMCOMMON = 4,
  ZMRARE = 5,
  ZMULTRARARE = 6,
  COUNT = 7,
};
enum class GPadStickConfigs : int32_t {
  DEFAULT = 0,
  SOUTHPAW = 1,
  LEGACY = 2,
  LEGACYSOUTHPAW = 3,
  COUNT = 4,
};
enum class LootResultType : int32_t {
  INVALID = 0,
  SUCCESS = 1,
  FAILURE_INSUFFICIENT_FUNDS = 2,
  FAILURE_BAD_RESPONE = 3,
  FAILURE = 4,
  COUNT = 5,
};
enum class bunkCollectibleSlots_e : int32_t {
  INVALID_SLOT = -1,
  COLLECTIBLE_SMALL_1 = 0,
  FIRST_SMALL_COLLECTIBLE = 0,
  FIRST_COLLECTIBLE = 0,
  COLLECTIBLE_SMALL_2 = 1,
  COLLECTIBLE_SMALL_3 = 2,
  COLLECTIBLE_SMALL_4 = 3,
  COLLECTIBLE_SMALL_5 = 4,
  COLLECTIBLE_SMALL_6 = 5,
  LAST_SMALL_COLLECTIBLE = 5,
  COLLECTIBLE_MEDIUM_1 = 6,
  FIRST_MEDIUM_COLLECTIBLE = 6,
  COLLECTIBLE_MEDIUM_2 = 7,
  LAST_MEDIUM_COLLECTIBLE = 7,
  COLLECTIBLE_LARGE_1 = 8,
  FIRST_LARGE_COLLECTIBLE = 8,
  LAST_LARGE_COLLECTIBLE = 8,
  LAST_COLLECTIBLE = 8,
  COLLECTIBLE_COUNT = 9,
};
enum class loadoutGadgetType_e : int32_t {
  OFFHAND_PRIMARY = 0,
  OFFHAND_SECONDARY = 1,
  OFFHAND_SPECIAL = 2,
  HERO_WEAPON = 3,
  COUNT = 4,
};
enum class CharacterGenderTypes : int32_t {
  MALE = 0,
  MP = 0,
  FIRST = 0,
  FEMALE = 1,
  COUNT = 2,
  INVALID = 2,
  ANY = 3,
};
enum class CharacterItemType : int32_t {
  BODY = 0,
  HELMET = 1,
  COUNT = 2,
};
enum class team_t : int32_t {
  FREE = 0,
  BAD = 0,
  DEAD = 0,
  ALLIES = 1,
  AXIS = 2,
  THREE = 3,
  FOUR = 4,
  FIVE = 5,
  SIX = 6,
  SEVEN = 7,
  EIGHT = 8,
  NINE = 9,
  TEN = 10,
  ELEVEN = 11,
  TWELVE = 12,
  THIRTEEN = 13,
  FOURTEEN = 14,
  FIFTEEN = 15,
  SIXTEEN = 16,
  SEVENTEEN = 17,
  EIGHTEEN = 18,
  NEUTRAL = 19,
  SPECTATOR = 20,
  NUM_PLAYING_TEAMS = 20,
  NUM_TEAMS = 21,
  LOCALPLAYERS = 22,
  FIRST_PLAYING_TEAM = 1,
  LAST_PLAYING_TEAM = 19,
};
enum class CampaignMode : int32_t {
  DEFAULT = 0,
  ZOMBIES = 1,
  COUNT = 2,
  INVALID = 3,
};
enum class FeaturedCardType : int32_t {
  NONE = 0,
  REGULAR = 1,
  GROUPS = 2,
  CRM = 3,
  MOTD = 4,
  STORE = 5,
  COUNT = 6,
};
enum class WaypointState : int32_t {
  DEFAULT = 0,
  OUT_OF_VIEWPORT = 1,
  LINE_OF_SIGHT = 2,
  OUT_OF_LINE_OF_SIGHT = 3,
  DISTANCE_CULLED = 4,
  COUNT = 5,
};
enum class CharacterGestureTypes : int32_t {
  GOOD_GAME = 0,
  THREATEN = 1,
  BOAST = 2,
  COUNT = 3,
};
enum class CharacterTauntTypes : int32_t {
  FIRST_PLACE = 0,
  COUNT = 1,
};

} // namespace lua
} // namespace ui
} // namespace game