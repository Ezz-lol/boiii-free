#pragma once

#include <cstdint>
namespace game {
namespace live {

enum class ProfileSetting : uint32_t {
  PROFILE_SETTING_INVALID = 0xFFFFFFFF,
  PROFILE_LAST_PLAYED_MODE = 0x0,
  PROFILE_GPAD_RUMBLE = 0x1,
  PROFILE_SESSION_REJOIN = 0x2,
  PROFILE_INVERT_PITCH = 0x3,
  PROFILE_VIEW_SENSITIVITY = 0x4,
  PROFILE_VIEW_SENSITIVITY_HORIZONTAL = 0x5,
  PROFILE_VIEW_SENSITIVITY_VERTICAL = 0x6,
  PROFILE_AUTO_AIM = 0x7,
  PROFILE_TEAM_INDICATOR = 0x8,
  PROFILE_UI_CAMO_UNLOCKS = 0x9,
  PROFILE_UI_ATTACH_UNLOCKS = 0xA,
  PROFILE_MATURE_CONTENT = 0xB,
  PROFILE_SHOW_BLOOD = 0xC,
  PROFILE_SHOW_GIBS = 0xD,
  PROFILE_GPAD_BUTTONS_CONFIG = 0xE,
  PROFILE_GPAD_STICKS_CONFIG = 0xF,
  PROFILE_GPAD_BUTTON_L_STICK_DEFLECT = 0x10,
  PROFILE_GPAD_BUTTON_R_STICK_DEFLECT = 0x11,
  PROFILE_BRIGHTNESS = 0x12,
  PROFILE_SND_MENU_VOICE = 0x13,
  PROFILE_SND_MENU_MUSIC = 0x14,
  PROFILE_SND_MENU_SFX = 0x15,
  PROFILE_SND_MENU_MASTER = 0x16,
  PROFILE_SND_MENU_CINEMATIC = 0x17,
  PROFILE_SND_MENU_HEARING_IMPAIRED = 0x18,
  PROFILE_SND_MENU_PRESETS = 0x19,
  PROFILE_SND_MENU_SPEAKER_SETUP = 0x1A,
  PROFILE_SND_MENU_MULTIPLAYER_CHARACTER_VOICE = 0x1B,
  PROFILE_SND_MENU_MULTIPLAYER_MUSIC = 0x1C,
  PROFILE_SND_MENU_MULTIPLAYER_MUSIC_TRACK = 0x1D,
  PROFILE_SND_MENU_CONTROLLER_SOUNDS = 0x1E,
  PROFILE_SPEAKER_LEFT_FRONT = 0x1F,
  PROFILE_SPEAKER_LEFT = 0x20,
  PROFILE_SPEAKER_CENTER = 0x21,
  PROFILE_SPEAKER_RIGHT_FRONT = 0x22,
  PROFILE_SPEAKER_RIGHT = 0x23,
  PROFILE_SPEAKER_LEFT_SURROUND = 0x24,
  PROFILE_SPEAKER_LEFT_SURROUND_BACK = 0x25,
  PROFILE_SPEAKER_RIGHT_SURROUND = 0x26,
  PROFILE_SPEAKER_RIGHT_SURROUND_BACK = 0x27,
  PROFILE_SPEAKER_LFE = 0x28,
  PROFILE_GEOGRAPHICAL_MATCHMAKING = 0x29,
  PROFILE_SAFE_AREA_HORIZONTAL = 0x2A,
  PROFILE_SAFE_AREA_VERTICAL = 0x2B,
  PROFILE_DEMO_CONTROLLER_CONFIG = 0x2C,
  PROFILE_DRAW_CROSSHAIR_3D = 0x2D,
  PROFILE_START_IN_3D = 0x2E,
  PROFILE_FLIPPED_CONTROL_CONFIG = 0x2F,
  PROFILE_COMMON_INITIALIZED = 0x31,
  PROFILE_COLOR_BLIND_ASSIST = 0x32,
  PROFILE_CHARACTER_CLASS = 0x33,
  PROFILE_CHEAT_COUNT = 0x34,
  PROFILE_EMAIL_ADDRESS = 0x35,
  PROFILE_SHOW_SUBTITLES = 0x36,
  PROFILE_LAST_SAVE_GAME = 0x37,
  PROFILE_GAMESKILL = 0x38,
  PROFILE_MISSION_HIGHEST_DIFFICULTY = 0x39,
  PROFILE_MISSION = 0x3a,
  PROFILE_TAKE_COVER_WARNINGS = 0x3b,
  PROFILE_COLLECTIBLES = 0x3c,
  PROFILE_SKILL_SET = 0x3d,
  PROFILE_MISSION_GLOBALS = 0x3e,
  PROFILE_TARGET_ASSIST = 0x3f,
  PROFILE_SPECTATE_THIRD_PERSON = 0x40,
  PROFILE_MOTD = 0x41,
  PROFILE_PLAYLIST_LEAGUEMATCH = 0x42,
  PROFILE_PLAYLIST_PLAYERMATCH = 0x43,
  PROFILE_PLAYLIST_SOLOMATCH = 0x44,
  PROFILE_CLASS_LOADOUTS = 0x45,
  PROFILE_SELECTED_CLASS_LOADOUT_SYSTEMLINK = 0x46,
  PROFILE_SELECTED_CLASS_LOADOUT_ONLINE = 0x47,
  PROFILE_SELECTED_CLASS_LOADOUT_LEAGUE = 0x48,
  PROFILE_LOADOUT_INITIALIZED = 0x49,
  PROFILE_PARTY_MAX_PLAYERS = 0x4a,
  PROFILE_MAP = 0x4b,
  PROFILE_GAMETYPE = 0x4c,
  PROFILE_MAP_ZM = 0x4d,
  PROFILE_GAMETYPE_ZM = 0x4e,
  PROFILE_MAP_CP = 0x4f,
  PROFILE_GAMETYPE_CP = 0x50,
  PROFILE_UNLOCK_CRUMBS_ZM = 0x51,
  PROFILE_EMBLEM = 0x52,
  PROFILE_FIRST_TIME_MISSION_SELECT = 0x53,
  PROFILE_FIRST_TIME_PRIVATEGAME_HOST_ZM = 0x54,
  PROFILE_ELITE_EMBLEM_UPLOAD_REQUESTED = 0x55,
  PROFILE_ELITE_WELCOME_POPUP_SHOWN = 0x56,
  PROFILE_ELITE_POSTMATCHRECORD_LAST_UPLOAD_TIME = 0x57,
  PROFILE_ELITE_REGISTRATION_POPUP_LAST_SHOWN_TIME = 0x58,
  PROFILE_ELITE_REGISTRATION_POPUP_TIMES_SHOWN = 0x59,
  PROFILE_ELITE_REGISTRATION_SIGNIN_COUNTER = 0x5a,
  PROFILE_TACTICAL_MODE_SHOW_DAMAGE = 0x5b,
  PROFILE_TACTICAL_MODE_SHOW_ARCHETYPE = 0x5c,
  PROFILE_TACTICAL_MODE_SHOW_DISTANCE = 0x5d,
  PROFILE_TACTICAL_MODE_SHOW_STATUS = 0x5e,
  PROFILE_LAST_SYSTEM_SURVEY_CAPTURE_TIME = 0x5f,
  PROFILE_MUTE_ALL_EXCEPT_PARTY = 0x60,
  PROFILE_PAINTSHOP_GRID_SIZE = 0x61,
  PROFILE_EMBLEM_GRID_SIZE = 0x62,
  PROFILE_DEFAULT_EMBLEM_INDEX = 0x63,
  PROFILE_DEFAULT_BACKGROUND_INDEX = 0x64,
  PROFILE_SHOW_CONNECTION_METER = 0x65,
  PROFILE_ENABLED_USER_GENERATED_CONTENT = 0x66,
  PROFILE_PRIVATEGAME_RANKED_ZM = 0x67,
  PROFILE_FIRSTTIME_BLACKMARKET = 0x68,
  PROFILE_TACTICAL_MODE_AUTO_ON = 0x69,
  PROFILE_COLOR_BLIND_MODE = 0x6a,
  PROFILE_DEMO_RECORD_WITH_AUTO_PLAY = 0x6b,
  PROFILE_DEMO_TOGGLE_GAME_HUD = 0x6c,
  PROFILE_DEMO_AUTO_DOLLY_RECORD = 0x6d,
  PROFILE_DEMO_AUTO_DOLLY_KEYFRAME_INTERVAL = 0x6e,
  PROFILE_SND_MUTE_MP_FRONTEND_MUSIC = 0x6f,
  PROFILE_SND_FRONTEND_MUSIC_STATE = 0x70,
  PROFILE_SETTINGS_COUNT = 0x71,

};

union CachedSettingsProfileKeys {
  struct {
    const char *quick_launch_mode;
    const char *gpad_rumble;
    const char *session_rejoin;
    const char *input_invertpitch;
    const char *input_viewSensitivity;
    const char *input_viewSensitivityHorizontal;
    const char *input_viewSensitivityVertical;
    const char *input_autoAim;
    const char *team_indicator;
    const char *ui_menuCamoUnlocks;
    const char *ui_menuAttachUnlocks;
    const char *cg_mature;
    const char *cg_blood;
    const char *cg_gibs;
    const char *gpad_buttonsConfig;
    const char *gpad_sticksConfig;
    const char *gpad_button_lstick_deflect_max;
    const char *gpad_button_rstick_deflect_max;
    const char *r_sceneBrightness;
    const char *snd_menu_voice;
    const char *snd_menu_music;
    const char *snd_menu_sfx;
    const char *snd_menu_master;
    const char *snd_menu_cinematic;
    const char *snd_menu_hearing_impaired;
    const char *snd_menu_presets;
    const char *snd_menu_speaker_setup;
    const char *snd_multiplayer_character_voice;
    const char *snd_multiplayer_music;
    const char *snd_multiplayer_music_track;
    const char *snd_controller_sounds;
    const char *speaker_left_front;
    const char *speaker_left;
    const char *speaker_center;
    const char *speaker_right_front;
    const char *speaker_right;
    const char *speaker_left_surround;
    const char *speaker_left_surround_back;
    const char *speaker_right_surround;
    const char *speaker_right_surround_back;
    const char *speaker_lfe;
    const char *party_privacyStatus;
    const char *geographicalMatchmaking;
    const char *safeAreaTweakable_horizontal;
    const char *safeAreaTweakable_vertical;
    const char *demo_controllerConfig;
    const char *cg_drawCrosshair3D;
    const char *start_in_3d;
    const char *flipped_control_config;
    const char *commonInitialized;
    const char *colorblind_assist;
    const char *snd_voicechat_open_mic;
    const char *snd_voicechat_volume;
    const char *snd_voicechat_record_level;
    const char *textchat_enable;
    const char *textchat_show_timestamps;
    const char *textchat_show_channel;
    const char *textchat_filter;
    const char *character_class;
    const char *com_first_time_account_linked;
    const char *com_first_time;
    const char *cg_playerState;
    const char *email_address;
    const char *cg_subtitles;
    const char *sv_lastSaveGame;
    const char *g_gameskill;
    const char *missionHighestDifficulty;
    const char *missions_unlocked;
    const char *takeCoverWarnings;
    const char *bg_collectibles;
    const char *skillSet;
    const char *missionGlobals;
    const char *input_targetAssist;
    const char *cg_spectateThirdPerson;
    const char *motd;
    const char *playlist_leaguematch;
    const char *playlist_playermatch;
    const char *playlist_solomatch;
    const char *cacLoadouts;
    const char *lastLoadoutSystemlink;
    const char *lastLoadoutOnline;
    const char *lastLoadoutLeague;
    const char *loadoutInitialized;
    const char *party_maxplayers;
    const char *map;
    const char *gametype;
    const char *map_ZM;
    const char *gametype_ZM;
    const char *map_CP;
    const char *gametype_CP;
    const char *unlock_crumbs_zm;
    const char *emblem;
    const char *com_first_time_mission_select;
    const char *com_first_time_privategame_host_zm;
    const char *eliteEmblemUploadRequested;
    const char *eliteWelcomePopupShown;
    const char *elitePostMatchRecordLastUploadTime;
    const char *eliteRegistrationPopupLastShownTime;
    const char *eliteRegistrationPopupTimesShown;
    const char *eliteRegistratonSigninCounter;
    const char *tacticalModeShowDamage;
    const char *tacticalModeShowArchetype;
    const char *tacticalModeShowDistance;
    const char *tacticalModeShowStatus;
    const char *lastSystemSurveyCaptureTime;
    const char *mute_all_except_party;
    const char *paintshop_grid_size;
    const char *emblem_grid_size;
    const char *default_emblem_index;
    const char *default_background_index;
    const char *showConnectionMeter;
    const char *cg_enableUGC;
    const char *languageSelected;
    const char *com_firsttime_freerun;
    const char *freerunHighestTrack;
    const char *com_privategame_ranked_zm;
    const char *com_firsttime_blackmarket;
    const char *tacticalModeAutoOn;
    const char *colorblindMode;
    const char *demo_recordWithAutoPlay;
    const char *demo_toggleGameHud;
    const char *demo_autoDollyRecord;
    const char *demo_autoDollyKeyframeInterval;
    const char *snd_muteMPFrontendMusic;
    const char *snd_frontendTracksState;
  } name;

  const char *names[125];
};
static_assert(
    sizeof(CachedSettingsProfileKeys) == 125 * sizeof(const char *),
    "CachedSettingsProfileKeys must be a simple array of 125 const char*");

union SettingsGlob {
  struct {
    bool cg_mature;
    bool cg_blood;
    bool cg_gibs;
    bool start_in_3d;
  } setting;

  bool glob[4];
};

} // namespace live
} // namespace game