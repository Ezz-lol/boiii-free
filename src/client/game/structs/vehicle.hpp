#pragma once

#include "ai.hpp"
#include "asm.hpp"
#include "cm.hpp"
#include "core.hpp"
#include "db/xasset/xmodel.hpp"
#include "gfx/gfx.hpp"
#include "hk/ai.hpp"
#include "math.hpp"
#include "move.hpp"
#include "phys/core.hpp"
#include "snd/snd.hpp"
#include "user.hpp"
#include <game/symbol.hpp>
#include <game/symbols/macros.hpp>

namespace game {
namespace level {
struct playerState_s;
typedef playerState_s playerState_t;

struct gentity_s;
typedef gentity_s gentity_t;
} // namespace level
namespace vehicle {

enum class vehicleFlags_e : uint32_t {
  PAUSED = 0x1,
  INITIALIZED = 0x2,
  SCRIPT_CONTROL = 0x4,
  DEBUG_RENDER = 0x8,
  PLAYERS_VEHICLE = 0x40,
  ATTACHED_PATH = 0x80,
  DRIVING_PATH = 0x100,
  ACTUATOR_DISABLED = 0x200,
  IN_WATER = 0x400,
  DISABLE_STABILIZERS = 0x800,
  STUNNED = 0x1000,
  NEEDS_UNPAUSE = 0x2000,
  NEEDS_ENTITIES_WHEELS_COLLISION = 0x4000,
  NEEDS_WHEEL_SETUP = 0x8000,
  BOOSTING = 0x10000,
  ANIMSCRIPTED = 0x20000,
};
IMPL_ENUM_OPERATORS(vehicleFlags_e);

inline constexpr const char *serialize(const vehicleFlags_e flag) {
  switch (flag) {
  case vehicleFlags_e::PAUSED:
    return "VEHICLEFLAG_PAUSED";
  case vehicleFlags_e::INITIALIZED:
    return "VEHICLEFLAG_INITIALIZED";
  case vehicleFlags_e::SCRIPT_CONTROL:
    return "VEHICLEFLAG_SCRIPT_CONTROL";
  case vehicleFlags_e::DEBUG_RENDER:
    return "VEHICLEFLAG_DEBUG_RENDER";
  case vehicleFlags_e::PLAYERS_VEHICLE:
    return "VEHICLEFLAG_PLAYERS_VEHICLE";
  case vehicleFlags_e::ATTACHED_PATH:
    return "VEHICLEFLAG_ATTACHED_PATH";
  case vehicleFlags_e::DRIVING_PATH:
    return "VEHICLEFLAG_DRIVING_PATH";
  case vehicleFlags_e::ACTUATOR_DISABLED:
    return "VEHICLEFLAG_ACTUATOR_DISABLED";
  case vehicleFlags_e::IN_WATER:
    return "VEHICLEFLAG_IN_WATER";
  case vehicleFlags_e::DISABLE_STABILIZERS:
    return "VEHICLEFLAG_DISABLE_STABILIZERS";
  case vehicleFlags_e::STUNNED:
    return "VEHICLEFLAG_STUNNED";
  case vehicleFlags_e::NEEDS_UNPAUSE:
    return "VEHICLEFLAG_NEEDS_UNPAUSE";
  case vehicleFlags_e::NEEDS_ENTITIES_WHEELS_COLLISION:
    return "VEHICLEFLAG_NEEDS_ENTITIES_WHEELS_COLLISION";
  case vehicleFlags_e::NEEDS_WHEEL_SETUP:
    return "VEHICLEFLAG_NEEDS_WHEEL_SETUP";
  case vehicleFlags_e::BOOSTING:
    return "VEHICLEFLAG_BOOSTING";
  case vehicleFlags_e::ANIMSCRIPTED:
    return "VEHICLEFLAG_ANIMSCRIPTED";
  default:
    return "VEHICLEFLAG_INVALID";
  }
}

template <typename T>
concept VehicleFlagLike = IntegralLike<T, uint32_t>;

#pragma pack(push, 1)
union VehicleFlags {
  uint32_t _raw;

  struct {
    uint32_t paused : 1;                          // B0
    uint32_t initialized : 1;                     // B1
    uint32_t script_control : 1;                  // B2
    uint32_t debug_render : 1;                    // B3
    uint32_t reserved_b4 : 1;                     // B4
    uint32_t reserved_b5 : 1;                     // B5
    uint32_t players_vehicle : 1;                 // B6
    uint32_t attached_path : 1;                   // B7
    uint32_t driving_path : 1;                    // B8
    uint32_t actuator_disabled : 1;               // B9
    uint32_t in_water : 1;                        // B10
    uint32_t disable_stabilizers : 1;             // B11
    uint32_t stunned : 1;                         // B12
    uint32_t needs_unpause : 1;                   // B13
    uint32_t needs_entities_wheels_collision : 1; // B14
    uint32_t needs_wheel_setup : 1;               // B15
    uint32_t boosting : 1;                        // B16
    uint32_t animscripted : 1;                    // B17
    uint32_t reserved_b18_31 : 14;
  };

  inline constexpr operator uint32_t() const noexcept { return _raw; }

  template <VehicleFlagLike T>
  static constexpr VehicleFlags from(T val) noexcept {
    return VehicleFlags{static_cast<uint32_t>(val)};
  }
  template <VehicleFlagLike T> inline constexpr void add(T flags) noexcept {
    _raw |= static_cast<uint32_t>(flags);
  }

  template <VehicleFlagLike T> inline constexpr void remove(T flags) noexcept {
    _raw &= static_cast<uint32_t>(~flags);
  }

  inline constexpr void clear() noexcept { _raw = 0; }

  template <VehicleFlagLike T> inline constexpr void set(T flags) noexcept {
    _raw = static_cast<uint32_t>(flags);
  }

  template <VehicleFlagLike T>
  inline constexpr VehicleFlags add(T flags) const noexcept {
    return VehicleFlags{_raw | static_cast<uint32_t>(flags)};
  }

  template <VehicleFlagLike T>
  inline constexpr VehicleFlags remove(T flags) const noexcept {
    return VehicleFlags{_raw & static_cast<uint32_t>(~flags)};
  }

  inline constexpr VehicleFlags clear() const noexcept {
    return VehicleFlags{0};
  }

  template <VehicleFlagLike T>
  inline constexpr VehicleFlags set(T flags) const noexcept {
    return VehicleFlags{static_cast<uint32_t>(flags)};
  }

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"paused\": {}, \"initialized\": {}, \"script_control\": {}, "
        "\"debug_render\": {}, \"reserved_b4\": {}, \"reserved_b5\": {}, "
        "\"players_vehicle\": {}, \"attached_path\": {}, \"driving_path\": {}, "
        "\"actuator_disabled\": {}, \"in_water\": {}, \"disable_stabilizers\": "
        "{}, "
        "\"stunned\": {}, \"needs_unpause\": {}, "
        "\"needs_entities_wheels_collision\": {}, "
        "\"needs_wheel_setup\": {}, \"boosting\": {}, \"animscripted\": {}, "
        "\"reserved_b18_31\": 0x{:08X} }}",
        paused, initialized, script_control, debug_render, reserved_b4,
        reserved_b5, players_vehicle, attached_path, driving_path,
        actuator_disabled, in_water, disable_stabilizers, stunned,
        needs_unpause, needs_entities_wheels_collision, needs_wheel_setup,
        boosting, animscripted, reserved_b18_31);
  }
};
#pragma pack(pop)

inline constexpr VehicleFlags operator~(VehicleFlags flag) noexcept {
  return VehicleFlags{~flag._raw};
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags &operator|=(VehicleFlags &lhs, T rhs) noexcept {
  lhs._raw |= static_cast<uint32_t>(rhs);
  return lhs;
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags &operator&=(VehicleFlags &lhs, T rhs) noexcept {
  lhs._raw &= static_cast<uint32_t>(rhs);
  return lhs;
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags &operator^=(VehicleFlags &lhs, T rhs) noexcept {
  lhs._raw ^= static_cast<uint32_t>(rhs);
  return lhs;
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags operator|(VehicleFlags lhs, T rhs) noexcept {
  return VehicleFlags{lhs._raw | static_cast<uint32_t>(rhs)};
}

template <
    VehicleFlagLike T,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, VehicleFlags>>>
inline constexpr VehicleFlags operator|(T lhs, VehicleFlags rhs) noexcept {
  return VehicleFlags{static_cast<uint32_t>(lhs) | rhs._raw};
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags operator&(VehicleFlags lhs, T rhs) noexcept {
  return VehicleFlags{lhs._raw & static_cast<uint32_t>(rhs)};
}

template <
    VehicleFlagLike T,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, VehicleFlags>>>
inline constexpr VehicleFlags operator&(T lhs, VehicleFlags rhs) noexcept {
  return VehicleFlags{static_cast<uint32_t>(lhs) & rhs._raw};
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags operator^(VehicleFlags lhs, T rhs) noexcept {
  return VehicleFlags{lhs._raw ^ static_cast<uint32_t>(rhs)};
}
template <
    VehicleFlagLike T,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, VehicleFlags>>>
inline constexpr VehicleFlags operator^(T lhs, VehicleFlags rhs) noexcept {
  return VehicleFlags{static_cast<uint32_t>(lhs) ^ rhs._raw};
}

template <VehicleFlagLike T>
inline constexpr bool operator==(VehicleFlags lhs, T rhs) noexcept {
  return lhs._raw == static_cast<uint32_t>(rhs);
}

template <
    VehicleFlagLike T,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, VehicleFlags>>>
inline constexpr bool operator==(T lhs, VehicleFlags rhs) noexcept {
  return static_cast<uint32_t>(lhs) == rhs._raw;
}

template <VehicleFlagLike T>
inline constexpr bool operator!=(VehicleFlags lhs, T rhs) noexcept {
  return lhs._raw != static_cast<uint32_t>(rhs);
}

template <
    VehicleFlagLike T,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, VehicleFlags>>>
inline constexpr bool operator!=(T lhs, VehicleFlags rhs) noexcept {
  return static_cast<uint32_t>(lhs) != rhs._raw;
};
ASSERT_SIZE(VehicleFlags, sizeof(uint32_t));
ASSERT_CPP03_POD(VehicleFlags);

PACKED(struct WheelState {
  int32_t m_state;
  int32_t m_last_state;
  float m_rate[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_state\": {}, \"m_last_state\": {}, \"m_rate\": {} }}", m_state,
        m_last_state, ::serialize(m_rate));
  }
});
ASSERT_SIZE(WheelState, 0x18);

enum class VehicleCategory : int32_t {
  NONE = 0x0,
  QUAD_TANK = 0x1,
  AMWS = 0x2,
  WASP = 0x3,
  ROCKET_WASP = 0x4,
  HUNTER = 0x5,
  SIEGE_BOT = 0x6,
  COUNT = 0x7,
};

inline constexpr const char *serialize(const VehicleCategory cat) {
  switch (cat) {
  case VehicleCategory::NONE:
    return "VEHICLECATEGORY_NONE";
  case VehicleCategory::QUAD_TANK:
    return "VEHICLECATEGORY_QUAD_TANK";
  case VehicleCategory::AMWS:
    return "VEHICLECATEGORY_AMWS";
  case VehicleCategory::WASP:
    return "VEHICLECATEGORY_WASP";
  case VehicleCategory::ROCKET_WASP:
    return "VEHICLECATEGORY_ROCKET_WASP";
  case VehicleCategory::HUNTER:
    return "VEHICLECATEGORY_HUNTER";
  case VehicleCategory::SIEGE_BOT:
    return "VEHICLECATEGORY_SIEGE_BOT";
  default:
    return "VEHICLECATEGORY_INVALID";
  }
}

PACKED(struct view_limits_t {
  float horizSpanLeft;
  float horizSpanRight;
  float vertSpanUp;
  float vertSpanDown;
  float horizResistLeft;
  float horizResistRight;
  float vertResistUp;
  float vertResistDown;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"horizSpanLeft\": {}, \"horizSpanRight\": {}, \"vertSpanUp\": {}, "
        "\"vertSpanDown\": {}, \"horizResistLeft\": {}, \"horizResistRight\": "
        "{}, "
        "\"vertResistUp\": {}, \"vertResistDown\": {} }}",
        horizSpanLeft, horizSpanRight, vertSpanUp, vertSpanDown,
        horizResistLeft, horizResistRight, vertResistUp, vertResistDown);
  }
});
ASSERT_SIZE(view_limits_t, 0x20);

PACKED(struct VehicleGearData {
  float minRPM;
  float maxRPM;
  float ratio;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"minRPM\": {}, \"maxRPM\": {}, \"ratio\": {} }}",
                       minRPM, maxRPM, ratio);
  }
});
ASSERT_SIZE(VehicleGearData, 0xC);

PACKED(struct VehicleEngineSound {
  const char *name;
  snd::SndAliasId alias;
  float params[7];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"alias\": 0x{:08X}, \"params\": {} }}",
        ::serialize(name), alias, ::serialize(params));
  }
});
ASSERT_SIZE(VehicleEngineSound, 0x28);

PACKED(struct VehicleEngine {
  bool simpleEngine;
  uint8_t _padding01[3];
  float idleRpms;
  float maxRpms;
  float maxTorque;
  float brakingCoeff;
  vec4_t loadFadeParams;
  vec4_t loadVolumeRange;
  float loadScale;
  float loadSmoothing;
  float throttleLag;
  float pitchScale;
  float rpmSmoothing;
  float differentialRatio;
  uint8_t _padding4C[4];
  VehicleEngineSound onload[5];
  VehicleEngineSound offload[5];
  int32_t numGears;
  bool loopLastGear;
  uint8_t _padding1E5[3];
  VehicleGearData gears[10];
  bool useAudioMaxSpeed;
  uint8_t _padding261[3];
  float audioMaxSpeed;
  const char *eng_idle_snd_name;
  snd::SndAliasId eng_idle_snd;
  float eng_idle_start_speed;
  float eng_idle_end_speed;
  float eng_idle_start_pitch;
  float eng_idle_end_pitch;
  uint8_t _padding284[4];
  const char *eng_move_snd_name;
  snd::SndAliasId eng_move_snd;
  float eng_move_start_speed;
  float eng_move_end_speed;
  float eng_move_start_pitch;
  float eng_move_end_pitch;
  uint8_t _padding2A4[4];
  const char *eng_offload_snd_name;
  snd::SndAliasId eng_offload_snd;
  float eng_offload_start_pitch;
  float eng_offload_end_pitch;
  uint8_t _padding2BC[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"simpleEngine\": {}, \"idleRpms\": {}, \"maxRpms\": {}, "
        "\"maxTorque\": {}, \"brakingCoeff\": {}, \"loadFadeParams\": {}, "
        "\"loadVolumeRange\": {}, \"loadScale\": {}, \"loadSmoothing\": {}, "
        "\"throttleLag\": {}, \"pitchScale\": {}, \"rpmSmoothing\": {}, "
        "\"differentialRatio\": {}, \"onload\": {}, \"offload\": {}, "
        "\"numGears\": {}, \"loopLastGear\": {}, \"gears\": {}, "
        "\"useAudioMaxSpeed\": {}, \"audioMaxSpeed\": {} }}",
        ::serialize(simpleEngine), idleRpms, maxRpms, maxTorque, brakingCoeff,
        loadFadeParams.serialize(), loadVolumeRange.serialize(), loadScale,
        loadSmoothing, throttleLag, pitchScale, rpmSmoothing, differentialRatio,
        ::serialize(onload), ::serialize(offload), numGears,
        ::serialize(loopLastGear), ::serialize(gears),
        ::serialize(useAudioMaxSpeed), audioMaxSpeed);
  }
});
ASSERT_SIZE(VehicleEngine, 0x2C0);

struct DevGraph;

enum class DevEventType : int32_t {
  ACTIVATE = 0x0,
  DEACTIVATE = 0x1,
  ACCEPT = 0x2,
  UPDATE = 0x3,
  DRAW = 0x4,
  SAVE = 0x5,
};

inline constexpr const char *serialize(const DevEventType type) {
  switch (type) {
  case DevEventType::ACTIVATE:
    return "DEVEVENT_ACTIVATE";
  case DevEventType::DEACTIVATE:
    return "DEVEVENT_DEACTIVATE";
  case DevEventType::ACCEPT:
    return "DEVEVENT_ACCEPT";
  case DevEventType::UPDATE:
    return "DEVEVENT_UPDATE";
  case DevEventType::DRAW:
    return "DEVEVENT_DRAW";
  case DevEventType::SAVE:
    return "DEVEVENT_SAVE";
  default:
    return "DEVEVENT_INVALID";
  }
}

typedef thiscallPtr_t<void(const DevGraph *, DevEventType type,
                           LocalClientNum_t localClientNum)>
    DevGraphEventCallback;
typedef thiscallPtr_t<void(const DevGraph *, const float, const float,
                           char *text, const int)>
    DevGraphTextCallback;

PACKED(struct DevGraph {
  vec2_t *knots;
  int32_t *knotCount;
  int32_t knotCountMax;
  int32_t selectedKnot;
  DevGraphEventCallback eventCallback;
  DevGraphTextCallback textCallback;
  void *data;
  bool disableEditingEndPoints;
  uint8_t _padding31[7];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"knots\": {}, \"knotCount\": {}, \"knotCountMax\": "
                       "{}, \"selectedKnot\": {}, "
                       "\"disableEditingEndPoints\": {} }}",
                       ::serialize(knots), ::serialize(knotCount), knotCountMax,
                       selectedKnot, ::serialize(disableEditingEndPoints));
  }
});
ASSERT_SIZE(DevGraph, 0x38);

PACKED(struct GraphFloat {
  char name[64];
  vec2_t knots[32];
  int32_t knotCount;
  float scale;
  DevGraph devguiGraph; // TODO: is this used in release?
  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"knots\": {}, \"knotCount\": {}, "
                       "\"scale\": {}, \"devguiGraph\": {} }}",
                       name, ::serialize(knots), knotCount, scale,
                       devguiGraph.serialize());
  }
});
ASSERT_SIZE(GraphFloat, 0x180);

PACKED(struct AircraftPreset {
  const char *presetName;
  int32_t maxSpeedVol;
  float minVol;
  float maxVol;
  int32_t maxSpeedPitch;
  float minPitch;
  float maxPitch;
  bool xAxis;
  bool yAxis;
  bool zAxis;
  uint8_t _padding23[5];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"presetName\": {}, \"maxSpeedVol\": {}, \"minVol\": {}, "
        "\"maxVol\": {}, \"maxSpeedPitch\": {}, \"minPitch\": {}, "
        "\"maxPitch\": {}, \"xAxis\": {}, \"yAxis\": {}, \"zAxis\": {} }}",
        ::serialize(presetName), maxSpeedVol, minVol, maxVol, maxSpeedPitch,
        minPitch, maxPitch, ::serialize(xAxis), ::serialize(yAxis),
        ::serialize(zAxis));
  }
});
ASSERT_SIZE(AircraftPreset, 0x28);

PACKED(struct AircraftSound {
  const char *presetName;
  AircraftPreset *preset;
  const char *aliasName;
  snd::SndAliasId alias;
  uint8_t _padding1C[4];
  const char *tagName;
  scr::ScrString_t tagScriptString;
  uint8_t _padding2C[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"presetName\": {}, \"preset\": {}, \"aliasName\": {}, "
        "\"alias\": 0x{:08X}, \"tagName\": {} }}",
        ::serialize(presetName), ::serialize(preset), ::serialize(aliasName),
        alias, ::serialize(tagName));
  }
});
ASSERT_SIZE(AircraftSound, 0x30);

PACKED(struct VehicleSurfaceSound {
  const char *surfaceSoundName;
  snd::SndAliasId alias;
  uint8_t _padding0C[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"surfaceSoundName\": {}, \"alias\": 0x{:08X} }}",
                       ::serialize(surfaceSoundName), alias);
  }
});
ASSERT_SIZE(VehicleSurfaceSound, 0x10);

PACKED(struct VehicleDriveBySound {
  int32_t apex;
  uint8_t _padding04[4];
  const char *name;
  snd::SndAliasId alias;
  uint8_t _padding14[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"apex\": {}, \"name\": {}, \"alias\": 0x{:08X} }}",
                       apex, ::serialize(name), alias);
  }
});

PACKED(struct VehicleSoundDef {
  const char *name;
  VehicleEngine engine;
  XString engineGraphName;
  GraphFloat *engineGraph;
  float driveBySoundRadius[2];
  VehicleDriveBySound driveBySounds[40];
  AircraftPreset aircraftPresets[20];
  AircraftSound aircraftSounds[20];
  VehicleSurfaceSound surfaceSounds[40];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"engine\": {}, \"engineGraph\": {:p}, "
        "\"driveBySoundRadius\": {}, \"driveBySounds\": {}, "
        "\"aircraftPresets\": {}, \"aircraftSounds\": {}, \"surfaceSounds\": "
        "{} }}",
        ::serialize(name), engine.serialize(),
        reinterpret_cast<const void *>(engineGraph),
        ::serialize(driveBySoundRadius), ::serialize(driveBySounds),
        ::serialize(aircraftPresets), ::serialize(aircraftSounds),
        ::serialize(surfaceSounds));
  }
});
// Verified
ASSERT_SIZE(VehicleSoundDef, 0x1000);

typedef VehicleSoundDef *VehicleSoundDefPtr;

enum class VehicleFxDefType : int32_t {
  NITROUS = 0x0,
  GROUND = 0x1,
  AIRCRAFT = 0x2,
  BOAT = 0x3,
  TANK = 0x4,
  COUNT = 0x5,
};

inline constexpr const char *serialize(const VehicleFxDefType type) {
  switch (type) {
  case VehicleFxDefType::NITROUS:
    return "VEHICLEFXDEF_NITROUS";
  case VehicleFxDefType::GROUND:
    return "VEHICLEFXDEF_GROUND";
  case VehicleFxDefType::AIRCRAFT:
    return "VEHICLEFXDEF_AIRCRAFT";
  case VehicleFxDefType::BOAT:
    return "VEHICLEFXDEF_BOAT";
  case VehicleFxDefType::TANK:
    return "VEHICLEFXDEF_TANK";
  default:
    return "VEHICLEFXDEF_INVALID";
  }
}

PACKED(struct VehicleFxDef {
  const char *name;
  char *csvInclude;
  VehicleFxDefType type;
  uint8_t _padding14[4];
  db::xasset::FxEffectDefHandle treadFx[40];
  db::xasset::FxEffectDefHandle peelFx[40];
  db::xasset::FxEffectDefHandle skidFx[40];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"type\": {}, \"treadFx\": {}, "
        "\"peelFx\": {}, \"skidFx\": {} }}",
        ::serialize(name), game::vehicle::serialize(type),
        ::serialize<uintptr_t>(
            reinterpret_cast<const uintptr_t *const>(treadFx),
            std::size(treadFx)),
        ::serialize<uintptr_t>(reinterpret_cast<const uintptr_t *const>(peelFx),
                               std::size(peelFx)),
        ::serialize<uintptr_t>(reinterpret_cast<const uintptr_t *const>(skidFx),
                               std::size(skidFx)));
  }
});
// Verified
ASSERT_SIZE(VehicleFxDef, 0x3D8);

typedef VehicleFxDef *VehicleFxDefPtr;

enum class TractionType : int32_t {
  FRONT = 0x0,
  BACK = 0x1,
  ALL_WD = 0x2,
  COUNT = 0x3,
};

inline constexpr const char *serialize(const TractionType type) {
  switch (type) {
  case TractionType::FRONT:
    return "TRACTION_FRONT";
  case TractionType::BACK:
    return "TRACTION_BACK";
  case TractionType::ALL_WD:
    return "TRACTION_ALL_WD";
  default:
    return "TRACTION_INVALID";
  }
}

PACKED(struct VehicleParameter {
  float m_speed_max;
  float m_accel_max;
  float m_reverse_scale;
  float m_steer_angle_max;
  float m_steer_angle_speed_scale;
  float m_steer_speed;
  float m_wheel_radius;
  float m_susp_spring_k;
  float m_susp_damp_k;
  float m_susp_adj;
  float m_susp_hard_limit;
  float m_susp_min_height;
  float m_tire_fric;
  float m_tire_fric_assist;
  float m_tire_fric_assist_threshold;
  float m_tire_fric_hand_brake_fwd;
  float m_tire_fric_hand_brake_side;
  float m_tire_fric_load_factor_min;
  float m_tire_fric_load_factor_max;
  float m_hand_brake_fake_rotation_force;
  float m_hand_brake_redirect_force;
  float m_body_mass;
  float m_roll_stability;
  float m_pitch_stability;
  float m_pitch_roll_resistance;
  float m_yaw_resistance;
  float m_upright_strength;
  float m_tire_damp_coast;
  float m_tire_damp_brake;
  float m_tire_damp_hand;
  float m_auto_hand_brake_min_speed;
  float m_max_climb_angle;
  float m_hand_brake_slide_time;
  float m_brake_to_reverse_time;
  float m_hover_trace_start_offset;
  float m_hover_trace_length_from_ground;
  float m_hover_trace_length_from_wall;
  float m_hover_to_wall_gravity_scale;
  float m_hover_to_wall_upright_strength_multiplier;
  float m_hover_to_wall_gravity_lerp_rate;
  float m_hover_to_ground_gravity_lerp_rate;
  float m_hover_to_ground_upright_strength_multiplier;
  float m_hover_min_speed_to_apply_brakes;
  float m_hover_brake_scale_factor;
  TractionType m_traction_type;
  uint8_t _paddingB4[4];
  const char *m_name;
  vec3_t m_wheel_offset;
  vec3_t m_bbox_min;
  vec3_t m_bbox_max;
  vec3_t m_mass_center_offset;
  vec3_t m_buoyancybox_min;
  vec3_t m_buoyancybox_max;
  float m_water_speed_max;
  float m_water_accel_max;
  float m_water_turn_accel;
  float m_water_turn_speed_max;
  float m_water_ebrake_power;
  vec3_t m_boat_motor_offset;
  float m_boat_speed_rise;
  float m_boat_speed_tilt;
  float m_boat_side_fric_scale;
  float m_boat_forward_fric_scale;
  float m_boat_vertical_fric_scale;
  float m_motorcycle_max_lean;
  float m_jump_force;
  float m_jump_force_in_air;
  float m_jump_force_from_wall;
  float m_jump_force_from_wall_max;
  float m_jump_force_from_wall_extra_z;
  float m_jump_gravity_scale_upwards;
  float m_jump_gravity_scale_transition_z;
  float m_jump_gravity_scale_downwards;
  float m_jump_from_wall_stick_angle;
  bool m_jumps_require_button_release;
  bool m_drive_on_walls;
  bool m_charge_jump;
  uint8_t _padding167[1];
  float m_linear_drag_scale;
  float m_angular_drag_scale;
  float m_gravity_scale;
  uint8_t _padding174[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_speed_max\": {}, \"m_accel_max\": {}, \"m_reverse_scale\": {}, "
        "\"m_steer_angle_max\": {}, \"m_steer_angle_speed_scale\": {}, "
        "\"m_steer_speed\": {}, "
        "\"m_wheel_radius\": {}, \"m_susp_spring_k\": {}, \"m_susp_damp_k\": "
        "{}, "
        "\"m_susp_adj\": {}, \"m_susp_hard_limit\": {}, \"m_susp_min_height\": "
        "{}, "
        "\"m_tire_fric\": {}, \"m_tire_fric_assist\": {}, "
        "\"m_tire_fric_assist_threshold\": {}, "
        "\"m_tire_fric_hand_brake_fwd\": {}, \"m_tire_fric_hand_brake_side\": "
        "{}, "
        "\"m_tire_fric_load_factor_min\": {}, \"m_tire_fric_load_factor_max\": "
        "{}, "
        "\"m_hand_brake_fake_rotation_force\": {}, "
        "\"m_hand_brake_redirect_force\": {}, "
        "\"m_body_mass\": {}, \"m_roll_stability\": {}, \"m_pitch_stability\": "
        "{}, "
        "\"m_pitch_roll_resistance\": {}, \"m_yaw_resistance\": {}, "
        "\"m_upright_strength\": {}, "
        "\"m_tire_damp_coast\": {}, \"m_tire_damp_brake\": {}, "
        "\"m_tire_damp_hand\": {}, "
        "\"m_auto_hand_brake_min_speed\": {}, \"m_max_climb_angle\": {}, "
        "\"m_hand_brake_slide_time\": {}, "
        "\"m_brake_to_reverse_time\": {}, \"m_hover_trace_start_offset\": {}, "
        "\"m_hover_trace_length_from_ground\": {}, "
        "\"m_hover_trace_length_from_wall\": {}, "
        "\"m_hover_to_wall_gravity_scale\": {}, "
        "\"m_hover_to_wall_upright_strength_multiplier\": {}, "
        "\"m_hover_to_wall_gravity_lerp_rate\": {}, "
        "\"m_hover_to_ground_gravity_lerp_rate\": {}, "
        "\"m_hover_to_ground_upright_strength_multiplier\": {}, "
        "\"m_hover_min_speed_to_apply_brakes\": {}, "
        "\"m_hover_brake_scale_factor\": {}, \"m_traction_type\": {}, "
        "\"m_name\": {}, "
        "\"m_wheel_offset\": {}, \"m_bbox_min\": {}, \"m_bbox_max\": {}, "
        "\"m_mass_center_offset\": {}, \"m_buoyancybox_min\": {}, "
        "\"m_buoyancybox_max\": {}, "
        "\"m_water_speed_max\": {}, \"m_water_accel_max\": {}, "
        "\"m_water_turn_accel\": {}, "
        "\"m_water_turn_speed_max\": {}, \"m_water_ebrake_power\": {}, "
        "\"m_boat_motor_offset\": {}, "
        "\"m_boat_speed_rise\": {}, \"m_boat_speed_tilt\": {}, "
        "\"m_boat_side_fric_scale\": {}, "
        "\"m_boat_forward_fric_scale\": {}, \"m_boat_vertical_fric_scale\": "
        "{}, \"m_motorcycle_max_lean\": {}, "
        "\"m_jump_force\": {}, \"m_jump_force_in_air\": {}, "
        "\"m_jump_force_from_wall\": {}, "
        "\"m_jump_force_from_wall_max\": {}, "
        "\"m_jump_force_from_wall_extra_z\": {}, "
        "\"m_jump_gravity_scale_upwards\": {}, "
        "\"m_jump_gravity_scale_transition_z\": {}, "
        "\"m_jump_gravity_scale_downwards\": {}, "
        "\"m_jump_from_wall_stick_angle\": {}, "
        "\"m_jumps_require_button_release\": {}, \"m_drive_on_walls\": {}, "
        "\"m_charge_jump\": {}, "
        "\"m_linear_drag_scale\": {}, \"m_angular_drag_scale\": {}, "
        "\"m_gravity_scale\": {} }}",
        m_speed_max, m_accel_max, m_reverse_scale, m_steer_angle_max,
        m_steer_angle_speed_scale, m_steer_speed, m_wheel_radius,
        m_susp_spring_k, m_susp_damp_k, m_susp_adj, m_susp_hard_limit,
        m_susp_min_height, m_tire_fric, m_tire_fric_assist,
        m_tire_fric_assist_threshold, m_tire_fric_hand_brake_fwd,
        m_tire_fric_hand_brake_side, m_tire_fric_load_factor_min,
        m_tire_fric_load_factor_max, m_hand_brake_fake_rotation_force,
        m_hand_brake_redirect_force, m_body_mass, m_roll_stability,
        m_pitch_stability, m_pitch_roll_resistance, m_yaw_resistance,
        m_upright_strength, m_tire_damp_coast, m_tire_damp_brake,
        m_tire_damp_hand, m_auto_hand_brake_min_speed, m_max_climb_angle,
        m_hand_brake_slide_time, m_brake_to_reverse_time,
        m_hover_trace_start_offset, m_hover_trace_length_from_ground,
        m_hover_trace_length_from_wall, m_hover_to_wall_gravity_scale,
        m_hover_to_wall_upright_strength_multiplier,
        m_hover_to_wall_gravity_lerp_rate, m_hover_to_ground_gravity_lerp_rate,
        m_hover_to_ground_upright_strength_multiplier,
        m_hover_min_speed_to_apply_brakes, m_hover_brake_scale_factor,
        game::vehicle::serialize(m_traction_type), ::serialize(m_name),
        m_wheel_offset.serialize(), m_bbox_min.serialize(),
        m_bbox_max.serialize(), m_mass_center_offset.serialize(),
        m_buoyancybox_min.serialize(), m_buoyancybox_max.serialize(),
        m_water_speed_max, m_water_accel_max, m_water_turn_accel,
        m_water_turn_speed_max, m_water_ebrake_power,
        m_boat_motor_offset.serialize(), m_boat_speed_rise, m_boat_speed_tilt,
        m_boat_side_fric_scale, m_boat_forward_fric_scale,
        m_boat_vertical_fric_scale, m_motorcycle_max_lean, m_jump_force,
        m_jump_force_in_air, m_jump_force_from_wall, m_jump_force_from_wall_max,
        m_jump_force_from_wall_extra_z, m_jump_gravity_scale_upwards,
        m_jump_gravity_scale_transition_z, m_jump_gravity_scale_downwards,
        m_jump_from_wall_stick_angle,
        ::serialize(m_jumps_require_button_release),
        ::serialize(m_drive_on_walls), ::serialize(m_charge_jump),
        m_linear_drag_scale, m_angular_drag_scale, m_gravity_scale);
  }
});
ASSERT_PACKED(VehicleParameter);
ASSERT_SIZE(VehicleParameter, 0x178);

PACKED(struct VehicleAntenna {
  float springK;
  float damp;
  float length;
  float gravity;
  bool useXAxis;
  uint8_t _padding11[3];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"springK\": {}, \"damp\": {}, \"length\": {}, "
                       "\"gravity\": {}, \"useXAxis\": {} }}",
                       springK, damp, length, gravity, ::serialize(useXAxis));
  }
});
ASSERT_SIZE(VehicleAntenna, 0x14);

enum class VehicleType : uint16_t {
  WHEELS_4 = 0x0,
  MOTORCYCLE = 0x1,
  TANK = 0x2,
  PLANE = 0x3,
  BOAT = 0x4,
  ARTILLERY = 0x5,
  HELICOPTER = 0x6,
  COUNT = 0x7,
};

inline constexpr const char *serialize(const VehicleType type) {
  switch (type) {
  case VehicleType::WHEELS_4:
    return "VEHICLETYPE_WHEELS_4";
  case VehicleType::MOTORCYCLE:
    return "VEHICLETYPE_MOTORCYCLE";
  case VehicleType::TANK:
    return "VEHICLETYPE_TANK";
  case VehicleType::PLANE:
    return "VEHICLETYPE_PLANE";
  case VehicleType::BOAT:
    return "VEHICLETYPE_BOAT";
  case VehicleType::ARTILLERY:
    return "VEHICLETYPE_ARTILLERY";
  case VehicleType::HELICOPTER:
    return "VEHICLETYPE_HELICOPTER";
  default:
    return "VEHICLETYPE_INVALID";
  }
}

PACKED(struct VehicleDef {
  const char *name;
  VehicleType type;
  uint8_t _padding0A[2];
  scr::ScrString_t scriptVehicleType;
  scr::ScrString_t archeType;
  scr::ScrString_t scoreType;
  scr::ScrString_t playerDrivenVersion;
  VehicleCategory category;
  qboolean nonstick;
  qboolean remoteControl;
  qboolean bulletDamage;
  qboolean armorPiercingDamage;
  qboolean grenadeDamage;
  qboolean projectileDamage;
  qboolean projectileSplashDamage;
  qboolean heavyExplosiveDamage;
  float physicsLaunchGrenadeScale;
  float physicsLaunchProjectileScale;
  float physicsLaunchExplosiveScale;
  float physicsLaunchDeathScale;
  float grenadeDamageMultiplier;
  float projectileDamageMultiplier;
  float explosiveDamageMultiplier;
  float bulletDamageMultiplier;
  float meleeDamageMultiplier;
  int16_t cameraMode;
  uint8_t _padding66[2];
  qboolean autoRecenterOnAccel;
  qboolean thirdPersonDriver;
  qboolean thirdPersonUseVehicleRoll;
  qboolean thirdPersonCameraPitchVehicleRelative;
  qboolean thirdPersonCameraHeightWorldRelative;
  float thirdPersonCameraRange;
  float thirdPersonCameraMinPitchClamp;
  float thirdPersonCameraMaxPitchClamp;
  float thirdPersonCameraHeight[2];
  float thirdPersonCameraPitch[2];
  float thirdPersonCameraDriveOnWallHeight;
  qboolean cameraAlwaysAutoCenter;
  float cameraAutoCenterLerpRate;
  float cameraAutoCenterMaxLerpRate;
  qboolean lodNoCull;
  float thirdPersonCameraSpringDistance;
  float thirdPersonCameraSpringTime;
  float thirdPersonCameraHandbrakeTurnRateInc;
  float cameraRollFraction;
  scr::ScrString_t cameraTag;
  vec3_t tagPlayerOffset;
  float cameraMinFOV;
  float cameraMaxFOV;
  float cameraMinFOVSpeed;
  float cameraMaxFOVSpeed;
  float blurMin;
  float blurMax;
  float blurMinSpeed;
  float blurMaxSpeed;
  float blurInnerRadius;
  float blurOuterRadius;
  qboolean killcamCollision;
  float killcamDist;
  float killcamZDist;
  float killcamMinDist;
  float killcamZTargetOffset;
  float killcamFOV;
  float killcamNearBlur;
  float killcamNearBlurStart;
  float killcamNearBlurEnd;
  float killcamFarBlur;
  float killcamFarBlurStart;
  float killcamFarBlurEnd;
  qboolean isDrivable;
  int32_t numberOfSeats;
  int32_t numberOfGunners;
  int32_t seatSwitchOrder[11];
  int32_t driverControlledGunPos;
  float entryPointRadius[5];
  float texScrollScale;
  float wheelRotRate;
  float extraWheelRotScale;
  qboolean wheelChildTakesSteerYaw;
  uint8_t _unknown184[4];
  qboolean useHeliBoneControllers;
  float minSpeed;
  float maxSpeed;
  float defaultSpeed;
  float maxSpeedVertical;
  float accel;
  float accelVertical;
  float rotRate;
  float rotAccel;
  float angDampening;
  float maxAngVelocityPitch;
  float maxTorquePitch;
  float rotMomentum;
  qboolean rotYawFromInput;
  float maxBodyPitch;
  float maxBodyRoll;
  float collisionDamage;
  float collisionSpeed;
  float suspensionTravel;
  qboolean aiSlidingTurn;
  qboolean meleeAvoidance;
  float heliCollisionScalar;
  float viewPitchOffset;
  float viewInfluence;
  float tiltFromAcceleration[2];
  float tiltFromDeceleration[2];
  float tiltFromVelocity[2];
  float tiltFromPrevious[2];
  float tiltSpeed[2];
  float vehHelicopterHoverRadius;
  float vehHelicopterHoverSpeed;
  float vehHelicopterHoverAccel;
  uint8_t _padding21C[4];
  const char *turretWeapon;
  view_limits_t turretViewLimits;
  qboolean turretClampPlayerView;
  qboolean turretLockTurretToPlayerView;
  qboolean turretFireFromCamera;
  uint8_t _padding254[4];
  const char *gunnerWeapon[4];
  uint16_t gunnerWeaponIndex[4];
  view_limits_t passengerViewLimits[6];
  const char *sndNames[7];
  snd::SndAliasId sndIndices[7];
  uint8_t _padding394[4];
  const char *sndMaterialNames[3];
  float skidSpeedMin;
  float skidSpeedMax;
  float peelSpeedMin;
  float peelSpeedMax;
  const char *futzName;
  float futzBlend;
  uint8_t _padding3CC[4];
  VehicleSoundDefPtr soundDef;
  int32_t animType;
  uint8_t _padding3DC[4];
  const char *animSet;
  qboolean scriptedAnimationEntry;
  float mantleAngles[4];
  scr::ScrString_t extraWheelTags[4];
  scr::ScrString_t rotorArmTags[4];
  int32_t rotorArmFlapType[8];
  float rotorArmFlapInfluence[8];
  scr::ScrString_t driverHideTag;
  scr::ScrString_t driverOtherHideTags[4];
  db::xasset::xmodel::XModelPtr attachmentModels[4];
  scr::ScrString_t attachmentTags[4];
  db::xasset::xmodel::XModelPtr deathAttachmentModels[4];
  scr::ScrString_t deathAttachmentTags[4];
  uint32_t targetingImmunePerk;
  scr::ScrString_t targetTags[4];
  vec3_t aimAssistMins;
  vec3_t aimAssistMaxs;
  float tracerOffset[2];
  uint8_t _padding504[4];
  db::xasset::xmodel::XModelPtr model;
  db::xasset::xmodel::XModelPtr viewModel;
  db::xasset::xmodel::XModelPtr deathModel;
  db::xasset::xmodel::XModelPtr enemyModel;
  float modelSwapDelay;
  uint8_t _padding52C[4];
  db::xasset::FxEffectDefHandle exhaustFx;
  scr::ScrString_t exhaustFxTag1;
  scr::ScrString_t exhaustFxTag2;
  VehicleFxDefPtr surfaceFx;
  db::xasset::FxEffectDefHandle deathFx;
  scr::ScrString_t deathFxTag;
  uint8_t _padding554[4];
  const char *deathFxSound;
  db::xasset::FxEffectDefHandle lightFx[4];
  scr::ScrString_t lightFxTag[4];
  float radiusDamageMin;
  float radiusDamageMax;
  float radiusDamageRadius;
  uint8_t _padding59C[4];
  const char *shootShock;
  float deathQuakeScale;
  float deathQuakeDuration;
  float deathQuakeRadius;
  uint8_t _padding5B4[4];
  const char *rumbleType;
  float rumbleScale;
  float rumbleDuration;
  float rumbleRadius;
  float rumbleBaseTime;
  float rumbleAdditionalTime;
  int32_t healthDefault;
  int32_t eTeam;
  int32_t boostAccelMultiplier;
  float boostDuration;
  float boostDurationMin;
  float boostRecoveryScalar;
  float boostSpeedIncrease;
  qboolean addToCompass;
  qboolean addToCompassEnemy;
  qboolean addToCompassEnemyFiring;
  uint8_t _padding5FC[4];
  const char *compassIcon;
  db::xasset::MaterialHandle compassIconMaterial;
  scr::ScrString_t compassIconTag;
  float compassScale;
  const char *steerAxisName;
  int32_t steerAxis;
  uint8_t _padding624[4];
  const char *gasAxisName;
  int32_t gasAxis;
  uint8_t _padding634[4];
  const char *gasButtonName;
  int32_t gasButton;
  uint8_t _padding644[4];
  const char *reverseBrakeButtonName;
  int32_t reverseBrakeButton;
  uint8_t _padding654[4];
  const char *handBrakeButtonName;
  int32_t handBrakeButton;
  uint8_t _padding664[4];
  const char *attackButtonName;
  int32_t attackButton;
  uint8_t _padding674[4];
  const char *attackSecondaryButtonName;
  int32_t attackSecondaryButton;
  uint8_t _padding684[4];
  const char *boostButtonName;
  int32_t boostButton;
  uint8_t _padding694[4];
  const char *moveUpButtonName;
  int32_t moveUpButton;
  uint8_t _padding6A4[4];
  const char *moveDownButtonName;
  int32_t moveDownButton;
  uint8_t _padding6B4[4];
  const char *switchSeatButtonName;
  int32_t switchSeatButton;
  qboolean noButtonAutoRemap;
  XString steerGraphName;
  GraphFloat *steerGraph;
  XString accelGraphName;
  GraphFloat *accelGraph;
  qboolean isNitrous;
  qboolean isFourWheelSteering;
  qboolean isMiddleWheelSteeringEnabled;
  qboolean useCollmap;
  float radius;
  float minHeight;
  float maxHeight;
  float lightCollisionSpeed;
  db::xasset::RumbleInfoPtr lightCollisionRumble;
  float heavyCollisionSpeed;
  uint8_t _padding714[4];
  db::xasset::RumbleInfoPtr heavyCollisionRumble;
  db::xasset::RumbleInfoPtr jumpLandingRumble;
  qboolean noDirectionalDamage;
  qboolean fakeBodyStabilizer;
  qboolean turnInPlace;
  qboolean alwaysDriveForward;
  qboolean smallVehicleCollision;
  float vehHelicopterBoundsRadius;
  float vehHelicopterDecelerationFwd;
  float vehHelicopterDecelerationSide;
  float vehHelicopterDecelerationUp;
  float vehHelicopterTiltFromControllerAxes;
  qboolean vehHelicopterPitchFromLookAxis;
  float vehHelicopterTiltFromFwdAndYaw;
  float vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt;
  float vehHelicopterTiltMomentum;
  float vehHelicopterJitterMinTime;
  float vehHelicopterJitterMaxTime;
  vec3_t vehHelicopterJitterMinAccel;
  vec3_t vehHelicopterJitterMaxAccel;
  float vehHelicopterJitterVelocityThreshold;
  float vehHelicopterJitterTiltPitchScale;
  float vehHelicopterJitterTiltRollScale;
  float vehHelicopterDogfightRollRate;
  vec3_t vehHelicopterDogfightLocalOffset;
  float vehHelicopterDogfightVelocityOffsetScalar;
  float vehHelicopterDogfightOvershootScalar;
  float vehHelicopterDogfightDesiredLocationLerpRate;
  float vehHelicopterDogfightSpeedFalloffDistance;
  float vehHelicopterDogfightSpeedFalloffDistanceExponent;
  float vehHelicopterDogfightControlLeewayAngle;
  float vehHelicopterDogfightFocusDampingAngle;
  float vehHelicopterDogfightFocusDampingAngleExponent;
  float vehHelicopterDogfightFocusAngleDeltaMaxRoll;
  float vehHelicopterDogfightCameraShakeScalar;
  vec3_t vehHelicopterDogfightCameraOffset;
  float vehHelicopterDogfightCameraLerpTime;
  qboolean vehHelicopterFlapRotor;
  qboolean vehHelicopterQuadRotor;
  qboolean vehHelicopterAccelTwardsView;
  qboolean vehHelicopterAccelTwardsViewWhenFiring;
  float maxRotorRotationSpeed;
  float idleRotorRotationSpeed;
  qboolean rotorArmRotateAroundY;
  float maxRotorArmMovementAngle;
  float maxRotorArmRotationAngle;
  vec2_t rotorArmMovementInfluence[4];
  qboolean vehHelicopterMaintainHeight;
  qboolean vehHelicopterMaintainMaxHeight;
  float vehHelicopterMaintainHeightLimit;
  float vehHelicopterMaintainHeightAccel;
  float vehHelicopterMaintainHeightMinimum;
  float vehHelicopterMaintainHeightMaximum;
  float vehHelicopterMaintainCeilingMinimum;
  qboolean joltVehicle;
  qboolean joltVehicleDriver;
  float joltMaxTime;
  float joltTime;
  float joltWaves;
  float joltIntensity;
  uint8_t _padding84C[4];
  VehicleParameter nitrousVehParams;
  qboolean doFootSteps;
  qboolean ignoreVortices;
  qboolean isSentient;
  qboolean isPathfinder;
  db::xasset::ScriptBundlePtr scriptBundleSettings;
  db::xasset::ScriptBundlePtr vehicleRidersBundle;
  db::xasset::ScriptBundlePtr vehicleRidersRobotBundle;
  db::xasset::ScriptBundlePtr assassinationBundle;
  anim::AnimStateMachinePtr animStateMachine;
  anim::AnimSelectorTableSetPtr animSelectorTable;
  anim::AnimMappingTablePtr animMappingTable;
  VehicleAntenna antenna[4];
  char *csvInclude;
  float customFloat0;
  float customFloat1;
  float customFloat2;
  qboolean customBool0;
  qboolean customBool1;
  qboolean customBool2;
  snd::SurfaceSoundDefPtr vehicleFootstepTable;
  db::xasset::SurfaceFXTableDefPtr vehicleFootstepFXTable;
  db::xasset::destructible::DestructibleDefPtr destructibleDef;
  gfx::GfxImage *tacticalModeIcon;
  float tacticalModeHeight;
  uint8_t _paddingAA4[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"type\": {}, "
        "\"scriptVehicleType\": 0x{:08X}, "
        "\"archeType\": 0x{:08X}, "
        "\"scoreType\": 0x{:08X}, \"playerDrivenVersion\": 0x{:08X}, "
        "\"category\": {}, "
        "\"nonstick\": {}, "
        "\"remoteControl\": {}, \"bulletDamage\": {}, \"armorPiercingDamage\": "
        "{}, "
        "\"grenadeDamage\": {}, \"projectileDamage\": {}, "
        "\"projectileSplashDamage\": {}, "
        "\"heavyExplosiveDamage\": {}, \"physicsLaunchGrenadeScale\": {}, "
        "\"physicsLaunchProjectileScale\": {}, "
        "\"physicsLaunchExplosiveScale\": {}, \"physicsLaunchDeathScale\": {}, "
        "\"grenadeDamageMultiplier\": {}, "
        "\"projectileDamageMultiplier\": {}, \"explosiveDamageMultiplier\": "
        "{}, \"bulletDamageMultiplier\": {}, "
        "\"meleeDamageMultiplier\": {}, \"cameraMode\": {}, "
        "\"autoRecenterOnAccel\": {}, \"thirdPersonDriver\": {}, "
        "\"thirdPersonUseVehicleRoll\": {}, "
        "\"thirdPersonCameraPitchVehicleRelative\": {}, "
        "\"thirdPersonCameraHeightWorldRelative\": {}, "
        "\"thirdPersonCameraRange\": {}, "
        "\"thirdPersonCameraMinPitchClamp\": {}, "
        "\"thirdPersonCameraMaxPitchClamp\": {}, "
        "\"thirdPersonCameraHeight\": {}, \"thirdPersonCameraPitch\": {}, "
        "\"thirdPersonCameraDriveOnWallHeight\": {}, "
        "\"cameraAlwaysAutoCenter\": {}, \"cameraAutoCenterLerpRate\": {}, "
        "\"cameraAutoCenterMaxLerpRate\": {}, "
        "\"lodNoCull\": {}, \"thirdPersonCameraSpringDistance\": {}, "
        "\"thirdPersonCameraSpringTime\": {}, "
        "\"thirdPersonCameraHandbrakeTurnRateInc\": {}, "
        "\"cameraRollFraction\": {}, \"cameraTag\": 0x{:08X}, "
        "\"tagPlayerOffset\": {}, \"cameraMinFOV\": {}, \"cameraMaxFOV\": {}, "
        "\"cameraMinFOVSpeed\": {}, "
        "\"cameraMaxFOVSpeed\": {}, \"blurMin\": {}, \"blurMax\": {}, "
        "\"blurMinSpeed\": {}, \"blurMaxSpeed\": {}, "
        "\"blurInnerRadius\": {}, \"blurOuterRadius\": {}, "
        "\"killcamCollision\": {}, \"killcamDist\": {}, "
        "\"killcamZDist\": {}, \"killcamMinDist\": {}, "
        "\"killcamZTargetOffset\": {}, \"killcamFOV\": {}, "
        "\"killcamNearBlur\": {}, \"killcamNearBlurStart\": {}, "
        "\"killcamNearBlurEnd\": {}, \"killcamFarBlur\": {}, "
        "\"killcamFarBlurStart\": {}, \"killcamFarBlurEnd\": {}, "
        "\"isDrivable\": {}, \"numberOfSeats\": {}, "
        "\"numberOfGunners\": {}, \"seatSwitchOrder\": {}, "
        "\"driverControlledGunPos\": {}, \"entryPointRadius\": {}, "
        "\"texScrollScale\": {}, \"wheelRotRate\": {}, \"extraWheelRotScale\": "
        "{}, \"wheelChildTakesSteerYaw\": {}, "
        "\"useHeliBoneControllers\": {}, \"minSpeed\": {}, \"maxSpeed\": {}, "
        "\"defaultSpeed\": {}, "
        "\"maxSpeedVertical\": {}, \"accel\": {}, \"accelVertical\": {}, "
        "\"rotRate\": {}, \"rotAccel\": {}, "
        "\"angDampening\": {}, \"maxAngVelocityPitch\": {}, "
        "\"maxTorquePitch\": {}, \"rotMomentum\": {}, "
        "\"rotYawFromInput\": {}, \"maxBodyPitch\": {}, \"maxBodyRoll\": {}, "
        "\"collisionDamage\": {}, "
        "\"collisionSpeed\": {}, \"suspensionTravel\": {}, \"aiSlidingTurn\": "
        "{}, \"meleeAvoidance\": {}, "
        "\"heliCollisionScalar\": {}, \"viewPitchOffset\": {}, "
        "\"viewInfluence\": {}, \"tiltFromAcceleration\": {}, "
        "\"tiltFromDeceleration\": {}, \"tiltFromVelocity\": {}, "
        "\"tiltFromPrevious\": {}, \"tiltSpeed\": {}, "
        "\"vehHelicopterHoverRadius\": {}, \"vehHelicopterHoverSpeed\": {}, "
        "\"vehHelicopterHoverAccel\": {}, "
        "\"turretWeapon\": {}, \"turretViewLimits\": {}, "
        "\"turretClampPlayerView\": {}, "
        "\"turretLockTurretToPlayerView\": {}, \"turretFireFromCamera\": {}, "
        "\"gunnerWeapon\": {}, "
        "\"gunnerWeaponIndex\": {}, \"passengerViewLimits\": {}, \"sndNames\": "
        "{}, \"sndIndices\": {}, "
        "\"sndMaterialNames\": {}, \"skidSpeedMin\": {}, \"skidSpeedMax\": {}, "
        "\"peelSpeedMin\": {}, "
        "\"peelSpeedMax\": {}, \"futzName\": {}, \"futzBlend\": {}, "
        "\"soundDef\": {}, \"animType\": {}, "
        "\"animSet\": {}, \"scriptedAnimationEntry\": {}, \"mantleAngles\": "
        "{}, \"extraWheelTags\": {}, "
        "\"rotorArmTags\": {}, \"rotorArmFlapType\": {}, "
        "\"rotorArmFlapInfluence\": {}, \"driverHideTag\": 0x{:08X}, "
        "\"driverOtherHideTags\": {}, \"attachmentModels\": {}, "
        "\"attachmentTags\": {}, \"deathAttachmentModels\": {}, "
        "\"deathAttachmentTags\": {}, \"targetingImmunePerk\": {}, "
        "\"targetTags\": {}, \"aimAssistMins\": {}, "
        "\"aimAssistMaxs\": {}, \"tracerOffset\": {}, \"model\": {}, "
        "\"viewModel\": {}, \"deathModel\": {}, "
        "\"enemyModel\": {}, \"modelSwapDelay\": {}, \"exhaustFx\": {}, "
        "\"exhaustFxTag1\": 0x{:08X}, \"exhaustFxTag2\": 0x{:08X}, "
        "\"surfaceFx\": {}, \"deathFx\": {}, \"deathFxTag\": 0x{:08X}, "
        "\"deathFxSound\": {}, \"lightFx\": {}, "
        "\"lightFxTag\": {}, \"radiusDamageMin\": {}, \"radiusDamageMax\": "
        "{}, "
        "\"radiusDamageRadius\": {}, "
        "\"shootShock\": {}, \"deathQuakeScale\": {}, \"deathQuakeDuration\": "
        "{}, \"deathQuakeRadius\": {}, "
        "\"rumbleType\": {}, \"rumbleScale\": {}, \"rumbleDuration\": {}, "
        "\"rumbleRadius\": {}, "
        "\"rumbleBaseTime\": {}, \"rumbleAdditionalTime\": {}, "
        "\"healthDefault\": {}, \"eTeam\": {}, "
        "\"boostAccelMultiplier\": {}, \"boostDuration\": {}, "
        "\"boostDurationMin\": {}, \"boostRecoveryScalar\": {}, "
        "\"boostSpeedIncrease\": {}, \"addToCompass\": {}, "
        "\"addToCompassEnemy\": {}, \"addToCompassEnemyFiring\": {}, "
        "\"compassIcon\": {}, \"compassIconMaterial\": {:p}, "
        "\"compassIconTag\": "
        "{:08X}, \"compassScale\": {}, "
        "\"steerAxisName\": {}, \"steerAxis\": {}, \"gasAxisName\": {}, "
        "\"gasAxis\": {}, \"gasButtonName\": {}, "
        "\"gasButton\": {}, \"reverseBrakeButtonName\": {}, "
        "\"reverseBrakeButton\": {}, \"handBrakeButtonName\": {}, "
        "\"handBrakeButton\": {}, \"attackButtonName\": {}, \"attackButton\": "
        "{}, \"attackSecondaryButtonName\": {}, "
        "\"attackSecondaryButton\": {}, \"boostButtonName\": {}, "
        "\"boostButton\": {}, \"moveUpButtonName\": {}, "
        "\"moveUpButton\": {}, \"moveDownButtonName\": {}, \"moveDownButton\": "
        "{}, \"switchSeatButtonName\": {}, "
        "\"switchSeatButton\": {}, \"noButtonAutoRemap\": {}, "
        "\"steerGraphName\": {}, \"steerGraph\": {}, "
        "\"accelGraphName\": {}, \"accelGraph\": {}, \"isNitrous\": {}, "
        "\"isFourWheelSteering\": {}, "
        "\"isMiddleWheelSteeringEnabled\": {}, \"useCollmap\": {}, \"radius\": "
        "{}, \"minHeight\": {}, "
        "\"maxHeight\": {}, \"lightCollisionSpeed\": {}, "
        "\"lightCollisionRumble\": {}, \"heavyCollisionSpeed\": {}, "
        "\"heavyCollisionRumble\": {:p}, \"jumpLandingRumble\": {:p}, "
        "\"noDirectionalDamage\": {}, "
        "\"fakeBodyStabilizer\": {}, \"turnInPlace\": {}, "
        "\"alwaysDriveForward\": {}, \"smallVehicleCollision\": {}, "
        "\"vehHelicopterBoundsRadius\": {}, \"vehHelicopterDecelerationFwd\": "
        "{}, \"vehHelicopterDecelerationSide\": {}, "
        "\"vehHelicopterDecelerationUp\": {}, "
        "\"vehHelicopterTiltFromControllerAxes\": {}, "
        "\"vehHelicopterPitchFromLookAxis\": {}, "
        "\"vehHelicopterTiltFromFwdAndYaw\": {}, "
        "\"vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt\": {}, "
        "\"vehHelicopterTiltMomentum\": {}, "
        "\"vehHelicopterJitterMinTime\": {}, \"vehHelicopterJitterMaxTime\": "
        "{}, \"vehHelicopterJitterMinAccel\": {}, "
        "\"vehHelicopterJitterMaxAccel\": {}, "
        "\"vehHelicopterJitterVelocityThreshold\": {}, "
        "\"vehHelicopterJitterTiltPitchScale\": {}, "
        "\"vehHelicopterJitterTiltRollScale\": {}, "
        "\"vehHelicopterDogfightRollRate\": {}, "
        "\"vehHelicopterDogfightLocalOffset\": {}, "
        "\"vehHelicopterDogfightVelocityOffsetScalar\": {}, "
        "\"vehHelicopterDogfightOvershootScalar\": {}, "
        "\"vehHelicopterDogfightDesiredLocationLerpRate\": {}, "
        "\"vehHelicopterDogfightSpeedFalloffDistance\": {}, "
        "\"vehHelicopterDogfightSpeedFalloffDistanceExponent\": {}, "
        "\"vehHelicopterDogfightControlLeewayAngle\": {}, "
        "\"vehHelicopterDogfightFocusDampingAngle\": {}, "
        "\"vehHelicopterDogfightFocusDampingAngleExponent\": {}, "
        "\"vehHelicopterDogfightFocusAngleDeltaMaxRoll\": {}, "
        "\"vehHelicopterDogfightCameraShakeScalar\": {}, "
        "\"vehHelicopterDogfightCameraOffset\": {}, "
        "\"vehHelicopterDogfightCameraLerpTime\": {}, "
        "\"vehHelicopterFlapRotor\": {}, \"vehHelicopterQuadRotor\": {}, "
        "\"vehHelicopterAccelTwardsView\": {}, "
        "\"vehHelicopterAccelTwardsViewWhenFiring\": {}, "
        "\"maxRotorRotationSpeed\": {}, \"idleRotorRotationSpeed\": {}, "
        "\"rotorArmRotateAroundY\": {}, \"maxRotorArmMovementAngle\": {}, "
        "\"maxRotorArmRotationAngle\": {}, "
        "\"rotorArmMovementInfluence\": {}, \"vehHelicopterMaintainHeight\": "
        "{}, \"vehHelicopterMaintainMaxHeight\": {}, "
        "\"vehHelicopterMaintainHeightLimit\": {}, "
        "\"vehHelicopterMaintainHeightAccel\": {}, "
        "\"vehHelicopterMaintainHeightMinimum\": {}, "
        "\"vehHelicopterMaintainHeightMaximum\": {}, "
        "\"vehHelicopterMaintainCeilingMinimum\": {}, \"joltVehicle\": {}, "
        "\"joltVehicleDriver\": {}, "
        "\"joltMaxTime\": {}, \"joltTime\": {}, \"joltWaves\": {}, "
        "\"joltIntensity\": {}, \"nitrousVehParams\": {}, "
        "\"doFootSteps\": {}, \"ignoreVortices\": {}, \"isSentient\": {}, "
        "\"isPathfinder\": {}, "
        "\"scriptBundleSettings\": {:p}, \"vehicleRidersBundle\": {:p}, "
        "\"vehicleRidersRobotBundle\": {:p}, "
        "\"assassinationBundle\": {:p}, "
        "\"animStateMachine\": {}, "
        "\"animSelectorTable\": {}, \"animMappingTable\": {}, "
        "\"antenna\": {}, \"csvInclude\": {}, \"customFloat0\": {}, "
        "\"customFloat1\": {}, \"customFloat2\": {}, "
        "\"customBool0\": {}, \"customBool1\": {}, \"customBool2\": {}, "
        "\"vehicleFootstepTable\": {}, "
        "\"vehicleFootstepFXTable\": {:p}, \"destructibleDef\": {:p}, "
        "\"tacticalModeIcon\": {}, \"tacticalModeHeight\": {} }}",
        ::serialize(name), game::vehicle::serialize(type), scriptVehicleType,
        archeType, scoreType, playerDrivenVersion,
        game::vehicle::serialize(category), nonstick.serialize(),
        remoteControl.serialize(), bulletDamage.serialize(),
        armorPiercingDamage.serialize(), grenadeDamage.serialize(),
        projectileDamage.serialize(), projectileSplashDamage.serialize(),
        heavyExplosiveDamage.serialize(), physicsLaunchGrenadeScale,
        physicsLaunchProjectileScale, physicsLaunchExplosiveScale,
        physicsLaunchDeathScale, grenadeDamageMultiplier,
        projectileDamageMultiplier, explosiveDamageMultiplier,
        bulletDamageMultiplier, meleeDamageMultiplier, cameraMode,
        autoRecenterOnAccel.serialize(), thirdPersonDriver.serialize(),
        thirdPersonUseVehicleRoll.serialize(),
        thirdPersonCameraPitchVehicleRelative.serialize(),
        thirdPersonCameraHeightWorldRelative.serialize(),
        thirdPersonCameraRange, thirdPersonCameraMinPitchClamp,
        thirdPersonCameraMaxPitchClamp, ::serialize(thirdPersonCameraHeight),
        ::serialize(thirdPersonCameraPitch), thirdPersonCameraDriveOnWallHeight,
        cameraAlwaysAutoCenter.serialize(), cameraAutoCenterLerpRate,
        cameraAutoCenterMaxLerpRate, lodNoCull.serialize(),
        thirdPersonCameraSpringDistance, thirdPersonCameraSpringTime,
        thirdPersonCameraHandbrakeTurnRateInc, cameraRollFraction, cameraTag,
        tagPlayerOffset.serialize(), cameraMinFOV, cameraMaxFOV,
        cameraMinFOVSpeed, cameraMaxFOVSpeed, blurMin, blurMax, blurMinSpeed,
        blurMaxSpeed, blurInnerRadius, blurOuterRadius,
        killcamCollision.serialize(), killcamDist, killcamZDist, killcamMinDist,
        killcamZTargetOffset, killcamFOV, killcamNearBlur, killcamNearBlurStart,
        killcamNearBlurEnd, killcamFarBlur, killcamFarBlurStart,
        killcamFarBlurEnd, isDrivable.serialize(), numberOfSeats,
        numberOfGunners, ::serialize(seatSwitchOrder), driverControlledGunPos,
        ::serialize(entryPointRadius), texScrollScale, wheelRotRate,
        extraWheelRotScale, wheelChildTakesSteerYaw.serialize(),
        useHeliBoneControllers.serialize(), minSpeed, maxSpeed, defaultSpeed,
        maxSpeedVertical, accel, accelVertical, rotRate, rotAccel, angDampening,
        maxAngVelocityPitch, maxTorquePitch, rotMomentum,
        rotYawFromInput.serialize(), maxBodyPitch, maxBodyRoll, collisionDamage,
        collisionSpeed, suspensionTravel, aiSlidingTurn.serialize(),
        meleeAvoidance.serialize(), heliCollisionScalar, viewPitchOffset,
        viewInfluence, ::serialize(tiltFromAcceleration),
        ::serialize(tiltFromDeceleration), ::serialize(tiltFromVelocity),
        ::serialize(tiltFromPrevious), ::serialize(tiltSpeed),
        vehHelicopterHoverRadius, vehHelicopterHoverSpeed,
        vehHelicopterHoverAccel, ::serialize(turretWeapon),
        turretViewLimits.serialize(), turretClampPlayerView.serialize(),
        turretLockTurretToPlayerView.serialize(),
        turretFireFromCamera.serialize(), ::serialize(gunnerWeapon),
        ::serialize(gunnerWeaponIndex), ::serialize(passengerViewLimits),
        ::serialize(sndNames), ::serialize(sndIndices),
        ::serialize(sndMaterialNames), skidSpeedMin, skidSpeedMax, peelSpeedMin,
        peelSpeedMax, ::serialize(futzName), futzBlend, ::serialize(soundDef),
        animType, ::serialize(animSet), scriptedAnimationEntry.serialize(),
        ::serialize(mantleAngles), ::serialize(extraWheelTags),
        ::serialize(rotorArmTags), ::serialize(rotorArmFlapType),
        ::serialize(rotorArmFlapInfluence), driverHideTag,
        ::serialize(driverOtherHideTags),
        ::serialize(reinterpret_cast<const uintptr_t *const>(attachmentModels),
                    std::size(attachmentModels)),
        ::serialize(attachmentTags),
        ::serialize(
            reinterpret_cast<const uintptr_t *const>(deathAttachmentModels),
            std::size(deathAttachmentModels)),
        ::serialize(deathAttachmentTags), targetingImmunePerk,
        ::serialize(targetTags), aimAssistMins.serialize(),
        aimAssistMaxs.serialize(), ::serialize(tracerOffset),
        ::serialize(model), ::serialize(viewModel), ::serialize(deathModel),
        ::serialize(enemyModel), modelSwapDelay, ::serialize(exhaustFx),
        exhaustFxTag1, exhaustFxTag2, ::serialize(surfaceFx),
        ::serialize(deathFx), deathFxTag, ::serialize(deathFxSound),
        ::serialize(reinterpret_cast<const uintptr_t *const>(lightFx),
                    std::size(lightFx)),
        ::serialize(lightFxTag), radiusDamageMin, radiusDamageMax,
        radiusDamageRadius, ::serialize(shootShock), deathQuakeScale,
        deathQuakeDuration, deathQuakeRadius, ::serialize(rumbleType),
        rumbleScale, rumbleDuration, rumbleRadius, rumbleBaseTime,
        rumbleAdditionalTime, healthDefault, eTeam, boostAccelMultiplier,
        boostDuration, boostDurationMin, boostRecoveryScalar,
        boostSpeedIncrease, addToCompass.serialize(),
        addToCompassEnemy.serialize(), addToCompassEnemyFiring.serialize(),
        ::serialize(compassIcon),
        reinterpret_cast<const void *>(compassIconMaterial), compassIconTag,
        compassScale, ::serialize(steerAxisName), steerAxis,
        ::serialize(gasAxisName), gasAxis, ::serialize(gasButtonName),
        gasButton, ::serialize(reverseBrakeButtonName), reverseBrakeButton,
        ::serialize(handBrakeButtonName), handBrakeButton,
        ::serialize(attackButtonName), attackButton,
        ::serialize(attackSecondaryButtonName), attackSecondaryButton,
        ::serialize(boostButtonName), boostButton,
        ::serialize(moveUpButtonName), moveUpButton,
        ::serialize(moveDownButtonName), moveDownButton,
        ::serialize(switchSeatButtonName), switchSeatButton,
        noButtonAutoRemap.serialize(), ::serialize(steerGraphName),
        ::serialize(steerGraph), ::serialize(accelGraphName),
        ::serialize(accelGraph), isNitrous.serialize(),
        isFourWheelSteering.serialize(),
        isMiddleWheelSteeringEnabled.serialize(), useCollmap.serialize(),
        radius, minHeight, maxHeight, lightCollisionSpeed,
        reinterpret_cast<const void *>(lightCollisionRumble),
        heavyCollisionSpeed,
        reinterpret_cast<const void *>(heavyCollisionRumble),
        reinterpret_cast<const void *>(jumpLandingRumble),
        noDirectionalDamage.serialize(), fakeBodyStabilizer.serialize(),
        turnInPlace.serialize(), alwaysDriveForward.serialize(),
        smallVehicleCollision.serialize(), vehHelicopterBoundsRadius,
        vehHelicopterDecelerationFwd, vehHelicopterDecelerationSide,
        vehHelicopterDecelerationUp, vehHelicopterTiltFromControllerAxes,
        vehHelicopterPitchFromLookAxis.serialize(),
        vehHelicopterTiltFromFwdAndYaw,
        vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt, vehHelicopterTiltMomentum,
        vehHelicopterJitterMinTime, vehHelicopterJitterMaxTime,
        vehHelicopterJitterMinAccel.serialize(),
        vehHelicopterJitterMaxAccel.serialize(),
        vehHelicopterJitterVelocityThreshold, vehHelicopterJitterTiltPitchScale,
        vehHelicopterJitterTiltRollScale, vehHelicopterDogfightRollRate,
        vehHelicopterDogfightLocalOffset.serialize(),
        vehHelicopterDogfightVelocityOffsetScalar,
        vehHelicopterDogfightOvershootScalar,
        vehHelicopterDogfightDesiredLocationLerpRate,
        vehHelicopterDogfightSpeedFalloffDistance,
        vehHelicopterDogfightSpeedFalloffDistanceExponent,
        vehHelicopterDogfightControlLeewayAngle,
        vehHelicopterDogfightFocusDampingAngle,
        vehHelicopterDogfightFocusDampingAngleExponent,
        vehHelicopterDogfightFocusAngleDeltaMaxRoll,
        vehHelicopterDogfightCameraShakeScalar,
        vehHelicopterDogfightCameraOffset.serialize(),
        vehHelicopterDogfightCameraLerpTime, vehHelicopterFlapRotor.serialize(),
        vehHelicopterQuadRotor.serialize(),
        vehHelicopterAccelTwardsView.serialize(),
        vehHelicopterAccelTwardsViewWhenFiring.serialize(),
        maxRotorRotationSpeed, idleRotorRotationSpeed,
        rotorArmRotateAroundY.serialize(), maxRotorArmMovementAngle,
        maxRotorArmRotationAngle, ::serialize(rotorArmMovementInfluence),
        vehHelicopterMaintainHeight.serialize(),
        vehHelicopterMaintainMaxHeight.serialize(),
        vehHelicopterMaintainHeightLimit, vehHelicopterMaintainHeightAccel,
        vehHelicopterMaintainHeightMinimum, vehHelicopterMaintainHeightMaximum,
        vehHelicopterMaintainCeilingMinimum, joltVehicle.serialize(),
        joltVehicleDriver.serialize(), joltMaxTime, joltTime, joltWaves,
        joltIntensity, nitrousVehParams.serialize(), doFootSteps.serialize(),
        ignoreVortices.serialize(), isSentient.serialize(),
        isPathfinder.serialize(),
        reinterpret_cast<const void *>(scriptBundleSettings),
        reinterpret_cast<const void *>(vehicleRidersBundle),
        reinterpret_cast<const void *>(vehicleRidersRobotBundle),
        reinterpret_cast<const void *>(assassinationBundle),
        ::serialize(animStateMachine), ::serialize(animSelectorTable),
        ::serialize(animMappingTable), ::serialize(antenna),
        ::serialize(csvInclude), customFloat0, customFloat1, customFloat2,
        customBool0.serialize(), customBool1.serialize(),
        customBool2.serialize(), ::serialize(vehicleFootstepTable),
        reinterpret_cast<const void *>(vehicleFootstepFXTable),
        reinterpret_cast<const void *>(destructibleDef),
        ::serialize(tacticalModeIcon), tacticalModeHeight);
  }
});
ASSERT_OFFSET(VehicleDef, useHeliBoneControllers, 0x188);
ASSERT_OFFSET(VehicleDef, isSentient, 0x9D0);
// Verified
ASSERT_SIZE(VehicleDef, 0xAA8);

PACKED(struct VehicleStun {
  bool m_changed_this_frame;
  uint8_t _padding01[3];
  float m_stun_time;
  float m_last_throttle;
  float m_throttle_stun_time;
  float m_last_brake;
  float m_brake_stun_time;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_changed_this_frame\": {}, \"m_stun_time\": {}, "
                       "\"m_last_throttle\": {}, "
                       "\"m_throttle_stun_time\": {}, \"m_last_brake\": {}, "
                       "\"m_brake_stun_time\": {} }}",
                       ::serialize(m_changed_this_frame), m_stun_time,
                       m_last_throttle, m_throttle_stun_time, m_last_brake,
                       m_brake_stun_time);
  }
});
ASSERT_SIZE(VehicleStun, 0x18);

PACKED(struct NitrousVehicleController {
public:
  enum class VehicleHorseControlState : int32_t {
    LEFT_STICK_STEERS_HORSE_AND_CAMERA = 0x0,
    LEFT_STICK_STEERS_HORSE_RIGHT_STICK_CAMERA = 0x1,
    RIGHT_STICK_STEERS_HORSE_AND_CAMERA = 0x2,
  };

  static inline constexpr const char *serialize(
      const VehicleHorseControlState state) {
    switch (state) {
    case NitrousVehicleController::VehicleHorseControlState::
        LEFT_STICK_STEERS_HORSE_AND_CAMERA:
      return "VHCS_LEFT_STICK_STEERS_HORSE_AND_CAMERA";
    case NitrousVehicleController::VehicleHorseControlState::
        LEFT_STICK_STEERS_HORSE_RIGHT_STICK_CAMERA:
      return "VHCS_LEFT_STICK_STEERS_HORSE_RIGHT_STICK_CAMERA";
    case NitrousVehicleController::VehicleHorseControlState::
        RIGHT_STICK_STEERS_HORSE_AND_CAMERA:
      return "VHCS_RIGHT_STICK_STEERS_HORSE_AND_CAMERA";
    default:
      return "VHCS_INVALID";
    }
  }

  math::Dir3 m_script_goal_position;
  float m_script_goal_radius;
  float m_script_goal_speed;
  float m_desired_yaw;
  bool m_has_desired_yaw;
  uint8_t _padding1D[3];
  float m_horse_target_speed;
  VehicleHorseControlState m_horse_control_state;
  float m_stopped_time;
  float m_braking_time;
  float m_stuck_time;
  uint8_t _padding34[4];
  math::Dir3 m_stuck_position;
  user::usercmd_t m_cmd;
  vec3_t m_cmd_viewangles;
  uint8_t _paddingAC[4];
  uint64_t m_pm_flags;
  bool m_cmd_valid;
  bool m_stop_at_goal;
  uint8_t _paddingBA[2];
  float m_jump_held_time;
  float m_stick_off_wall_time;
  vec3_t m_last_jump_normal;
  vec2_t m_jump_input;
  bool m_jump_awaiting_button_release;
  bool m_last_jump_button_press_initiated;
  bool m_last_jump_initiated_in_air;
  uint8_t _paddingDB[1];
  VehicleStun m_stun;
  bool m_hover_wall_targetting;
  uint8_t _paddingF5[3];
  game_button_bits_t m_last_update_button_bits;
  uint8_t _unknown106[0x2];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_script_goal_position\": {}, \"m_script_goal_radius\": {}, "
        "\"m_script_goal_speed\": {}, \"m_desired_yaw\": {}, "
        "\"m_has_desired_yaw\": {}, "
        "\"m_horse_target_speed\": {}, \"m_horse_control_state\": {}, "
        "\"m_stopped_time\": {}, "
        "\"m_braking_time\": {}, \"m_stuck_time\": {}, \"m_stuck_position\": "
        "{}, "
        "\"m_cmd\": {}, \"m_cmd_viewangles\": {}, \"m_pm_flags\": 0x{:016X}, "
        "\"m_cmd_valid\": {}, \"m_stop_at_goal\": {}, \"m_jump_held_time\": "
        "{}, "
        "\"m_stick_off_wall_time\": {}, \"m_last_jump_normal\": {}, "
        "\"m_jump_input\": {}, "
        "\"m_jump_awaiting_button_release\": {}, "
        "\"m_last_jump_button_press_initiated\": {}, "
        "\"m_last_jump_initiated_in_air\": {}, \"m_stun\": {}, "
        "\"m_hover_wall_targetting\": {}, "
        "\"m_last_update_button_bits\": {} }}",
        m_script_goal_position.serialize(), m_script_goal_radius,
        m_script_goal_speed, m_desired_yaw, ::serialize(m_has_desired_yaw),
        m_horse_target_speed,
        game::vehicle::NitrousVehicleController::serialize(
            m_horse_control_state),
        m_stopped_time, m_braking_time, m_stuck_time,
        m_stuck_position.serialize(), m_cmd.serialize(),
        m_cmd_viewangles.serialize(), m_pm_flags, ::serialize(m_cmd_valid),
        ::serialize(m_stop_at_goal), m_jump_held_time, m_stick_off_wall_time,
        m_last_jump_normal.serialize(), m_jump_input.serialize(),
        ::serialize(m_jump_awaiting_button_release),
        ::serialize(m_last_jump_button_press_initiated),
        ::serialize(m_last_jump_initiated_in_air), m_stun.serialize(),
        ::serialize(m_hover_wall_targetting),
        m_last_update_button_bits.serialize());
  }
});
ASSERT_OFFSET(NitrousVehicleController, m_script_goal_speed, 0x14);
ASSERT_OFFSET(NitrousVehicleController, m_script_goal_radius, 0x10);
/*
   Server and client have different `NitrousVehicleController` structs :(.

   NitrousVehicleController is added to NitrousVehicle as (non-pointer) field,
   so we can't just use a simple `EngineDependentNitrousVehicleController` union
   to abstract this.

   NitrousVehicle can be abstracted this way though. This will be necessary
   if/when we begin using `NitrousVehicle`s in code executed in client.
*/
// Server
ASSERT_OFFSET(NitrousVehicleController, m_stop_at_goal, 0xC1);
// Client
// ASSERT_OFFSET(NitrousVehicleController, m_stop_at_goal, 0xB9);
ASSERT_SIZE(NitrousVehicleController, 0x10e);

PACKED(struct NitrousVehicleController_cl {
  math::Dir3 m_script_goal_position;
  float m_script_goal_radius;
  float m_script_goal_speed;
  float m_desired_yaw;
  bool m_has_desired_yaw;
  uint8_t _padding1D[3];
  float m_horse_target_speed;
  NitrousVehicleController::VehicleHorseControlState m_horse_control_state;
  float m_stopped_time;
  float m_braking_time;
  float m_stuck_time;
  uint8_t _padding34[8];
  math::Dir3 m_stuck_position;
  user::usercmd_t m_cmd;
  uint8_t _paddingAC[4];
  uint64_t m_pm_flags;
  bool m_cmd_valid;
  bool m_stop_at_goal;
  uint8_t _paddingBA[2];
  float m_jump_held_time;
  float m_stick_off_wall_time;
  vec3_t m_last_jump_normal;
  vec2_t m_jump_input;
  bool m_jump_awaiting_button_release;
  bool m_last_jump_button_press_initiated;
  bool m_last_jump_initiated_in_air;
  uint8_t _paddingDB[1];
  VehicleStun m_stun;
  bool m_hover_wall_targetting;
  uint8_t _paddingF5[3];
  game_button_bits_t m_last_update_button_bits;
  uint8_t _unknown106[28];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_script_goal_position\": {}, \"m_script_goal_radius\": {}, "
        "\"m_script_goal_speed\": {}, \"m_desired_yaw\": {}, "
        "\"m_has_desired_yaw\": {}, "
        "\"m_horse_target_speed\": {}, \"m_horse_control_state\": {}, "
        "\"m_stopped_time\": {}, "
        "\"m_braking_time\": {}, \"m_stuck_time\": {}, \"m_stuck_position\": "
        "{}, "
        "\"m_cmd\": {}, \"m_pm_flags\": 0x{:016X}, \"m_cmd_valid\": {}, "
        "\"m_stop_at_goal\": {}, \"m_jump_held_time\": {}, "
        "\"m_stick_off_wall_time\": {}, "
        "\"m_last_jump_normal\": {}, \"m_jump_input\": {}, "
        "\"m_jump_awaiting_button_release\": {}, "
        "\"m_last_jump_button_press_initiated\": {}, "
        "\"m_last_jump_initiated_in_air\": {}, "
        "\"m_stun\": {}, \"m_hover_wall_targetting\": {}, "
        "\"m_last_update_button_bits\": {} }}",
        m_script_goal_position.serialize(), m_script_goal_radius,
        m_script_goal_speed, m_desired_yaw, ::serialize(m_has_desired_yaw),
        m_horse_target_speed,
        game::vehicle::NitrousVehicleController::serialize(
            m_horse_control_state),
        m_stopped_time, m_braking_time, m_stuck_time,
        m_stuck_position.serialize(), m_cmd.serialize(), m_pm_flags,
        ::serialize(m_cmd_valid), ::serialize(m_stop_at_goal), m_jump_held_time,
        m_stick_off_wall_time, m_last_jump_normal.serialize(),
        m_jump_input.serialize(), ::serialize(m_jump_awaiting_button_release),
        ::serialize(m_last_jump_button_press_initiated),
        ::serialize(m_last_jump_initiated_in_air), m_stun.serialize(),
        ::serialize(m_hover_wall_targetting),
        m_last_update_button_bits.serialize());
  }
});
ASSERT_OFFSET(NitrousVehicleController_cl, m_script_goal_speed, 0x14);
ASSERT_OFFSET(NitrousVehicleController_cl, m_script_goal_radius, 0x10);
ASSERT_OFFSET(NitrousVehicleController_cl, m_stop_at_goal, 0xB9);
ASSERT_SIZE(NitrousVehicleController_cl, 0x120); // ?

PACKED(struct mover_record_t {
public:
  vec3_t m_origin;
  vec3_t m_angles;
  int32_t m_time;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_origin\": {}, \"m_angles\": {}, \"m_time\": {} }}",
        m_origin.serialize(), m_angles.serialize(), m_time);
  }
});
ASSERT_SIZE(mover_record_t, 0x1C);

PACKED(struct mover_info_t {
  mover_record_t m_records[32];
  int32_t m_last_record;
  uint8_t _padding384[4];
  hk::ai::hkaiNavMeshInstance *m_meshInstance;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_records\": {}, \"m_last_record\": {}, "
                       "\"m_meshInstance\": {:p} }}",
                       ::serialize(m_records), m_last_record,
                       reinterpret_cast<const void *>(m_meshInstance));
  }
});
ASSERT_SIZE(mover_info_t, 0x390);

PACKED(struct minspec_mutex {
public:
  volatile uint32_t m_token;

  inline std::string serialize() const noexcept {
    const uint32_t m_token_copy = m_token;
    return std::format("{{ \"m_token\": {} }}", m_token_copy);
  }
});
ASSERT_SIZE(minspec_mutex, 0x4);

PACKED(struct NitrousVehicle {
  struct syms {
    static constexpr symbol<thiscall_t<void(NitrousVehicle *, float throttle)>>
        set_throttle{0x141BB1100, 0x141BBD4D0, 0x140321840};
    static constexpr symbol<thiscall_t<float(NitrousVehicle *, double delta_t)>>
        get_stuck_accel_factor{0x14230A9C0, 0x142383B30, 0x1405C2030};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, LocalClientNum_t localClientNum)>>
        setup_wheels{0x14230AB10, 0x142383C80, 0x1405C2180};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const double delta_t)>>
        update_boost{0x14230B060, 0x1423841D0, 0x1405C26D0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, float inTq, float spinVel0,
                        float spinVel1, float *outTq0, float *outTq1)>>
        update_differential{0x14230B150, 0x1423842C0, 0x1405C27C0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const double delta_t)>>
        update_fakey_stuff{0x14230B250, 0x1423843C0, 0x1405C2AA0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const double delta_t)>>
        update_gravity{0x14230B840, 0x1423849B0, 0x1405C3090};
    static constexpr symbol<thiscall_t<void(NitrousVehicle *)>>
        update_orientation_constraint{0x14230C4D0, 0x142385640, 0x1405C3D20};
    static constexpr symbol<thiscall_t<void(NitrousVehicle *)>> update_pause{
        0x14230C6A0, 0x142385810, 0x1405C3EF0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const double delta_t)>>
        update_prolog{0x14230C800, 0x142385970, 0x1405C4050};
    static constexpr symbol<fastcall_t<NitrousVehicle *(int32_t id)>>
        add_vehicle{0x14230CCC0, 0x142385E30, 0x1405C4510};
    static constexpr symbol<fastcall_t<void(double delta_t)>>
        frame_Epilog_All_Systems{0x14230DD50, 0x142386EC0, 0x1405C55A0};
    static constexpr symbol<fastcall_t<void(const double delta_t)>>
        frame_prolog_all_systems{0x14230DF90, 0x142387100, 0x1405C5610};
    static constexpr symbol<thiscall_t<float(const NitrousVehicle *)>>
        get_throttle{0x14230E220, 0x142387390, 0x1405C58B0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, level::gentity_t *owner,
                        const VehicleParameter *parameter)>>
        init{0x14230E310, 0x142387480, 0x1405C5980};
    static constexpr symbol<thiscall_t<bool(NitrousVehicle *)>> is_path_moving{
        0x14230E6F0, 0x142387860, 0x1405C5D60};
    static constexpr symbol<thiscall_t<void(NitrousVehicle *, bool shutdown)>>
        pause_physics{0x1423118B0, 0x14238AA20, 0x1405C5EF0};
    static constexpr symbol<thiscall_t<void(const NitrousVehicle *v)>>
        remove_vehicle{0x142311C00, 0x14238AD70, 0x1405C6240};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, int32_t attach_mode)>>
        start_path{0x142311FE0, 0x14238B150, 0x1405C6470};
    static constexpr symbol<thiscall_t<void(NitrousVehicle *)>> unpause_physics{
        0x142312160, 0x14238B2D0, 0x1405C65F0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const float desired_speed_factor)>>
        update_braking_and_acceleration{0x142315990, 0x14238EB00, 0x1405C69D0};
    static constexpr symbol<thiscall_t<void(
        NitrousVehicle *, const VehicleDef *vehicleDef, bool initialization)>>
        update_parms{0x142316820, 0x14238F990, 0x1405C7860};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const float acceleration_factor)>>
        update_steering{0x142317200, 0x142390370, 0x1405C82B0};
  };

  inline void set_throttle(float throttle) {
    return syms::set_throttle(this, throttle);
  }
  inline float get_stuck_accel_factor(double delta_t) {
    return syms::get_stuck_accel_factor(this, delta_t);
  }
  inline void setup_wheels(LocalClientNum_t localClientNum) {
    return syms::setup_wheels(this, localClientNum);
  }
  inline void update_boost(const double delta_t) {
    return syms::update_boost(this, delta_t);
  }
  inline void update_differential(float inTq, float spinVel0, float spinVel1,
                                  float *outTq0, float *outTq1) {
    return syms::update_differential(this, inTq, spinVel0, spinVel1, outTq0,
                                     outTq1);
  }
  inline void update_fakey_stuff(const double delta_t) {
    return syms::update_fakey_stuff(this, delta_t);
  }
  inline void update_gravity(const double delta_t) {
    return syms::update_gravity(this, delta_t);
  }
  inline void update_orientation_constraint() {
    return syms::update_orientation_constraint(this);
  }
  inline void update_pause() { return syms::update_pause(this); }
  inline void update_prolog(const double delta_t) {
    return syms::update_prolog(this, delta_t);
  }
  static inline NitrousVehicle *add_vehicle(int32_t id) {
    return syms::add_vehicle(id);
  }
  static inline void frame_Epilog_All_Systems(double delta_t) {
    return syms::frame_Epilog_All_Systems(delta_t);
  }
  static inline void frame_prolog_all_systems(const double delta_t) {
    return syms::frame_prolog_all_systems(delta_t);
  }
  inline float get_throttle() { return syms::get_throttle(this); }
  inline void init(level::gentity_t * owner,
                   const VehicleParameter *parameter) {
    return syms::init(this, owner, parameter);
  }
  inline bool is_path_moving() { return syms::is_path_moving(this); }
  inline void pause_physics(bool shutdown) {
    return syms::pause_physics(this, shutdown);
  }
  static inline void remove_vehicle(const NitrousVehicle *v) {
    return syms::remove_vehicle(v);
  }
  inline void start_path(int32_t attach_mode) {
    return syms::start_path(this, attach_mode);
  }
  inline void unpause_physics() { return syms::unpause_physics(this); }
  inline void update_braking_and_acceleration(
      const float desired_speed_factor) {
    return syms::update_braking_and_acceleration(this, desired_speed_factor);
  }

  inline void update_parms(const VehicleDef *vehicleDef, bool initialization) {
    return syms::update_parms(this, vehicleDef, initialization);
  }
  inline void update_steering(const float acceleration_factor) {
    return syms::update_steering(this, acceleration_factor);
  }

  phys::PhysObjUserData *m_phys_user_data;
  WheelState m_wheel_state[6];
  uint8_t _padding98[8];
  math::RotTranMat43 m_wheel_orig_relpo[6];
  int32_t m_wheel_damage[6];
  int32_t m_wheel_surf_types[6];
  float m_wheel_yaw[6];
  float m_steer_factor;
  float m_steer_scale;
  const VehicleParameter *m_parameter;
  float m_throttle;
  float m_brake;
  float m_hand_brake;
  float m_script_brake;
  int64_t m_boost_start_time;
  float m_boost_time_pool;
  float m_forward_vel;
  uint8_t _padding298[8];
  math::Dir3 m_ground_vel;
  math::Dir3 m_last_known_velocity;
  float m_hand_brake_friction_time;
  uint8_t _padding2C4[4];
  level::gentity_t *m_owner;
  int32_t m_entnum;
  uint8_t _padding2D4[4];
  const VehicleDef *m_vehicle_def;
  db::xasset::xmodel::XModel *m_xmodel;
  phys::rigid_body_constraint_custom_orientation *m_orientation_constraint;
  phys::rigid_body_constraint_custom_path *m_vpc;
  VehicleFlags m_flags;
  int32_t m_notify_flags;
  int32_t m_server_notify_flags;
  vec3_t m_collision_hitp;
  vec3_t m_collision_hitn;
  float m_collision_intensity;
  int32_t m_collision_stype;
  int32_t m_collision_entnum;
  float m_fake_rpm;
  float m_current_side_fric_scale;
  float m_current_fwd_fric_scale;
  float m_stuck_time;
  uint8_t _padding328[8];
  math::RotTranMat43 m_mat;
  NitrousVehicleController mVehicleController;
  uint8_t _unknown494[6];
  int32_t m_num_colliding_wheels;
  uint8_t _unknown49C[12];
  int32_t m_lastNetworkTime;
  int32_t m_lastErrorReductionTime;
  vec3_t m_networkErrorOrigin;
  vec3_t m_networkErrorAngles;
  int32_t m_trackDistAccumLeftInches;
  int32_t m_trackDistAccumRightInches;
  int32_t m_sfx_state_flags;
  float m_speed_scale;
  float m_drivepath_scale;
  float m_accel_scale;
  int32_t m_throttle_held_down;
  bool m_throttle_time_started;
  bool m_last_all_wheel_contact_was_wall;
  bool m_is_being_drop_deployed;
  uint8_t _padding4E3[1];
  vec3_t m_last_origin;
  vec3_t m_last_angles;
  float m_buoyancyOffset;
  mover_info_t m_mover_info;
  minspec_mutex m_mutex;
  int32_t id;
  int32_t refcount;
  uint8_t _padding89C[4];
  phys::rigid_body_constraint_wheel *m_wheels[6];
  float m_desired_speed_factor;
  float m_acceleration_factor;
  float m_power_braking_factor;
  float m_braking_factor;
  float m_coasting_factor;
  float m_reference_wheel_radius;
  float m_steer_current_angle;
  float m_steer_max_angle;
  float m_steer_speed;
  uint8_t _padding8F4[12];
  math::Dir3 m_steer_front_pt_loc;
  float m_steer_front_back_length;
  float m_min_turning_radius;
  uint32_t m_state_flags;
  uint8_t _padding91C[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_phys_user_data\": {:p}, \"m_wheel_state\": {}, "
        "\"m_wheel_orig_relpo\": {}, "
        "\"m_wheel_damage\": {}, \"m_wheel_surf_types\": {}, \"m_wheel_yaw\": "
        "{}, "
        "\"m_steer_factor\": {}, \"m_steer_scale\": {}, \"m_parameter\": {:p}, "
        "\"m_throttle\": {}, \"m_brake\": {}, \"m_hand_brake\": {}, "
        "\"m_script_brake\": {}, "
        "\"m_boost_start_time\": {}, \"m_boost_time_pool\": {}, "
        "\"m_forward_vel\": {}, "
        "\"m_ground_vel\": {}, \"m_last_known_velocity\": {}, "
        "\"m_hand_brake_friction_time\": {}, "
        "\"m_owner\": {:p}, \"m_entnum\": {}, \"m_vehicle_def\": {:p}, "
        "\"m_xmodel\": {:p}, "
        "\"m_orientation_constraint\": {:p}, \"m_vpc\": {:p}, \"m_flags\": {}, "
        "\"m_notify_flags\": 0x{:08X}, "
        "\"m_server_notify_flags\": 0x{:08X}, \"m_collision_hitp\": {}, "
        "\"m_collision_hitn\": {}, "
        "\"m_collision_intensity\": {}, \"m_collision_stype\": {}, "
        "\"m_collision_entnum\": {}, "
        "\"m_fake_rpm\": {}, \"m_current_side_fric_scale\": {}, "
        "\"m_current_fwd_fric_scale\": {}, "
        "\"m_stuck_time\": {}, \"m_mat\": {}, \"mVehicleController\": {}, "
        "\"m_num_colliding_wheels\": {}, "
        "\"m_lastNetworkTime\": {}, \"m_lastErrorReductionTime\": {}, "
        "\"m_networkErrorOrigin\": {}, "
        "\"m_networkErrorAngles\": {}, \"m_trackDistAccumLeftInches\": {}, "
        "\"m_trackDistAccumRightInches\": {}, "
        "\"m_sfx_state_flags\": 0x{:08X}, \"m_speed_scale\": {}, "
        "\"m_drivepath_scale\": {}, \"m_accel_scale\": {}, "
        "\"m_throttle_held_down\": {}, \"m_throttle_time_started\": {}, "
        "\"m_last_all_wheel_contact_was_wall\": {}, "
        "\"m_is_being_drop_deployed\": {}, \"m_last_origin\": {}, "
        "\"m_last_angles\": {}, \"m_buoyancyOffset\": {}, "
        "\"m_mover_info\": {}, \"m_mutex\": {}, \"id\": {}, \"refcount\": {}, "
        "\"m_wheels\": {:p}, "
        "\"m_desired_speed_factor\": {}, \"m_acceleration_factor\": {}, "
        "\"m_power_braking_factor\": {}, "
        "\"m_braking_factor\": {}, \"m_coasting_factor\": {}, "
        "\"m_reference_wheel_radius\": {}, "
        "\"m_steer_current_angle\": {}, \"m_steer_max_angle\": {}, "
        "\"m_steer_speed\": {}, "
        "\"m_steer_front_pt_loc\": {}, \"m_steer_front_back_length\": {}, "
        "\"m_min_turning_radius\": {}, "
        "\"m_state_flags\": 0x{:08X} }}",
        reinterpret_cast<const void *>(m_phys_user_data),
        ::serialize(m_wheel_state), ::serialize(m_wheel_orig_relpo),
        ::serialize(m_wheel_damage), ::serialize(m_wheel_surf_types),
        ::serialize(m_wheel_yaw), m_steer_factor, m_steer_scale,
        reinterpret_cast<const void *>(m_parameter), m_throttle, m_brake,
        m_hand_brake, m_script_brake, m_boost_start_time, m_boost_time_pool,
        m_forward_vel, m_ground_vel.serialize(),
        m_last_known_velocity.serialize(), m_hand_brake_friction_time,
        reinterpret_cast<const void *>(m_owner), m_entnum,
        reinterpret_cast<const void *>(m_vehicle_def),
        reinterpret_cast<const void *>(m_xmodel),
        reinterpret_cast<const void *>(m_orientation_constraint),
        reinterpret_cast<const void *>(m_vpc), ::serialize(m_flags),
        m_notify_flags, m_server_notify_flags, m_collision_hitp.serialize(),
        m_collision_hitn.serialize(), m_collision_intensity, m_collision_stype,
        m_collision_entnum, m_fake_rpm, m_current_side_fric_scale,
        m_current_fwd_fric_scale, m_stuck_time, m_mat.serialize(),
        mVehicleController.serialize(), m_num_colliding_wheels,
        m_lastNetworkTime, m_lastErrorReductionTime,
        m_networkErrorOrigin.serialize(), m_networkErrorAngles.serialize(),
        m_trackDistAccumLeftInches, m_trackDistAccumRightInches,
        m_sfx_state_flags, m_speed_scale, m_drivepath_scale, m_accel_scale,
        m_throttle_held_down, ::serialize(m_throttle_time_started),
        ::serialize(m_last_all_wheel_contact_was_wall),
        ::serialize(m_is_being_drop_deployed), m_last_origin.serialize(),
        m_last_angles.serialize(), m_buoyancyOffset, ::serialize(m_mover_info),
        ::serialize(m_mutex), id, refcount,
        reinterpret_cast<const void *>(m_wheels), m_desired_speed_factor,
        m_acceleration_factor, m_power_braking_factor, m_braking_factor,
        m_coasting_factor, m_reference_wheel_radius, m_steer_current_angle,
        m_steer_max_angle, m_steer_speed, m_steer_front_pt_loc.serialize(),
        m_steer_front_back_length, m_min_turning_radius, m_state_flags);
  }
});
ASSERT_OFFSET(NitrousVehicle, m_flags, 0x2F8);
ASSERT_OFFSET(NitrousVehicle, m_vehicle_def, 0x2D8);
ASSERT_OFFSET(NitrousVehicle, m_wheels, 0x8A0);
ASSERT_OFFSET(NitrousVehicle, m_mat, 0x340);
ASSERT_OFFSET(NitrousVehicle, m_num_colliding_wheels, 0x494);
ASSERT_OFFSET(NitrousVehicle, mVehicleController, 0x380);

PACKED(struct vehicle_spline_node_t {
  int16_t nextIdx;
  int16_t prevIdx;
  float length;
  vec3_t dir;
  float t;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"nextIdx\": {}, \"prevIdx\": {}, \"length\": {}, "
                       "\"dir\": {}, \"t\": {} }}",
                       nextIdx, prevIdx, length, dir.serialize(), t);
  }
});
ASSERT_SIZE(vehicle_spline_node_t, 0x18);

PACKED(struct vehicle_node_t {
  scr::ScrString_t name;
  scr::ScrString_t target;
  scr::ScrString_t target2;
  scr::ScrString_t script_linkname;
  scr::ScrString_t script_noteworthy;
  int16_t index;
  uint8_t _padding16[2];
  int32_t flags;
  float speed;
  float lookAhead;
  vec3_t origin;
  vec3_t angles;
  float radius;
  float tension;
  vehicle_spline_node_t splineNode;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": 0x{:08X}, \"target\": 0x{:08X}, \"target2\": 0x{:08X}, "
        "\"script_linkname\": {}, "
        "\"script_noteworthy\": 0x{:08X}, \"index\": {}, \"flags\": 0x{:08X}, "
        "\"speed\": {}, "
        "\"lookAhead\": {}, \"origin\": {}, \"angles\": {}, \"radius\": {}, "
        "\"tension\": {}, \"splineNode\": {} }}",
        name, target, target2, script_linkname, script_noteworthy, index, flags,
        speed, lookAhead, origin.serialize(), angles.serialize(), radius,
        tension, splineNode.serialize());
  }
});
ASSERT_SIZE(vehicle_node_t, 0x5C);

enum class path_type_e : int32_t {
  NONE = 0x0,
  OLD_SPLINE = 0x1,
  SPLINE = 0x2,
  AI_PATH = 0x3,
};
inline constexpr const char *serialize(const path_type_e type) {
  switch (type) {
  case path_type_e::NONE:
    return "path_type_e::NONE";
  case path_type_e::OLD_SPLINE:
    return "path_type_e::OLD_SPLINE";
  case path_type_e::SPLINE:
    return "path_type_e::SPLINE";
  case path_type_e::AI_PATH:
    return "path_type_e::AI_PATH";
  default:
    return "path_type_e::INVALID";
  }
}
PACKED(struct vehicle_pathpos_t {
public:
  int16_t nodeIdx;
  int16_t prevIdx;
  int16_t endOfPath;
  uint8_t _padding06[2];
  float frac;
  float speed;
  float lookAhead;
  float slide;
  float width;
  float manualTime;
  float widthLookAheadFrac;
  float distanceTraveled;
  vec3_t origin;
  vec3_t angles;
  vec3_t lookPos;
  vec3_t origin_on_spline;
  vec3_t angles_on_spline;
  vec3_t lookPos_on_spline;
  qboolean use_spline_speed;
  float manualSpeed;
  vehicle_node_t switchNode[2];
  int32_t flags;
  float customGoalLength;
  vec3_t customGoalDir;
  vec3_t aiGoalPos;
  path_type_e path_type;
  int16_t startNodeIdx;
  uint8_t _padding156[2];
  float t;
  float tot_len;
  float tot_time;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"nodeIdx\": {}, \"prevIdx\": {}, \"endOfPath\": {}, \"frac\": {}, "
        "\"speed\": {}, \"lookAhead\": {}, \"slide\": {}, \"width\": {}, "
        "\"manualTime\": {}, \"widthLookAheadFrac\": {}, \"distanceTraveled\": "
        "{}, "
        "\"origin\": {}, \"angles\": {}, \"lookPos\": {}, "
        "\"origin_on_spline\": {}, "
        "\"angles_on_spline\": {}, \"lookPos_on_spline\": {}, "
        "\"use_spline_speed\": {}, "
        "\"manualSpeed\": {}, \"switchNode\": {}, \"flags\": 0x{:08X}, "
        "\"customGoalLength\": {}, "
        "\"customGoalDir\": {}, \"aiGoalPos\": {}, \"path_type\": {}, "
        "\"startNodeIdx\": {}, "
        "\"t\": {}, \"tot_len\": {}, \"tot_time\": {} }}",
        nodeIdx, prevIdx, endOfPath, frac, speed, lookAhead, slide, width,
        manualTime, widthLookAheadFrac, distanceTraveled, origin.serialize(),
        angles.serialize(), lookPos.serialize(), origin_on_spline.serialize(),
        angles_on_spline.serialize(), lookPos_on_spline.serialize(),
        use_spline_speed.serialize(), manualSpeed, ::serialize(switchNode),
        flags, customGoalLength, customGoalDir.serialize(),
        aiGoalPos.serialize(), game::vehicle::serialize(path_type),
        startNodeIdx, t, tot_len, tot_time);
  }
});
ASSERT_SIZE(vehicle_pathpos_t, 0x164);

PACKED(struct vehicle_physic_t {
  vec3_t origin;
  vec3_t prevOrigin;
  vec3_t angles;
  vec3_t prevAngles;
  vec3_t maxAngleVel;
  float yawAccel;
  float yawDecel;
  vec3_t mins;
  vec3_t maxs;
  vec3_t vel;
  vec3_t bodyVel;
  vec3_t rotVel;
  vec3_t accel;
  float maxPitchAngle;
  float maxRollAngle;
  float wheelZVel[6];
  float wheelZPos[6];
  int32_t wheelSurfType[6];
  vec3_t bodyTilt;
  vec3_t worldTilt;
  vec3_t worldTiltVel;
  float heliLockHeight;
  float curveLength;
  int32_t curveID;
  float curveStep;
  float curveTime;
  float timeStep;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"origin\": {}, \"prevOrigin\": {}, \"angles\": {}, "
        "\"prevAngles\": {}, "
        "\"maxAngleVel\": {}, \"yawAccel\": {}, \"yawDecel\": {}, \"mins\": "
        "{}, "
        "\"maxs\": {}, \"vel\": {}, \"bodyVel\": {}, \"rotVel\": {}, "
        "\"accel\": {}, "
        "\"maxPitchAngle\": {}, \"maxRollAngle\": {}, \"wheelZVel\": {}, "
        "\"wheelZPos\": {}, "
        "\"wheelSurfType\": {}, \"bodyTilt\": {}, \"worldTilt\": {}, "
        "\"worldTiltVel\": {}, "
        "\"heliLockHeight\": {}, \"curveLength\": {}, \"curveID\": {}, "
        "\"curveStep\": {}, "
        "\"curveTime\": {}, \"timeStep\": {} }}",
        origin.serialize(), prevOrigin.serialize(), angles.serialize(),
        prevAngles.serialize(), maxAngleVel.serialize(), yawAccel, yawDecel,
        mins.serialize(), maxs.serialize(), vel.serialize(),
        bodyVel.serialize(), rotVel.serialize(), accel.serialize(),
        maxPitchAngle, maxRollAngle, ::serialize(wheelZVel),
        ::serialize(wheelZPos), ::serialize(wheelSurfType),
        bodyTilt.serialize(), worldTilt.serialize(), worldTiltVel.serialize(),
        heliLockHeight, curveLength, curveID, curveStep, curveTime, timeStep);
  }
});
ASSERT_SIZE(vehicle_physic_t, 0x118);

enum class VehicleTargetType : int32_t {

  NONE = 0x0,
  ENTITY = 0x1,
  POSITION = 0x2,
  RELATIVE_ANGLES = 0x3,
};

inline constexpr const char *serialize(const VehicleTargetType type) {
  switch (type) {
  case VehicleTargetType::NONE:
    return "VEHICLETARGETTYPE_NONE";
  case VehicleTargetType::ENTITY:
    return "VEHICLETARGETTYPE_ENTITY";
  case VehicleTargetType::POSITION:
    return "VEHICLETARGETTYPE_POSITION";
  case VehicleTargetType::RELATIVE_ANGLES:
    return "VEHICLETARGETTYPE_RELATIVE_ANGLES";
  default:
    return "VEHICLETARGETTYPE_INVALID";
  }
}

enum class VehicleTurretState : int32_t {

  STOPPED = 0x0,
  STOPPING = 0x1,
  MOVING_CW = 0x2,
  MOVING_CCW = 0x3,
};

inline constexpr const char *serialize(const VehicleTurretState state) {
  switch (state) {
  case VehicleTurretState::STOPPED:
    return "VEHICLETURRETSTATE_STOPPED";
  case VehicleTurretState::STOPPING:
    return "VEHICLETURRETSTATE_STOPPING";
  case VehicleTurretState::MOVING_CW:
    return "VEHICLETURRETSTATE_MOVING_CW";
  case VehicleTurretState::MOVING_CCW:
    return "VEHICLETURRETSTATE_MOVING_CCW";
  default:
    return "VEHICLETURRETSTATE_INVALID";
  }
}

enum class VehicleMoveState : int32_t {
  STOP = 0x0,
  MOVE = 0x1,
  HOVER = 0x2,
  PLANE_ONCURVE = 0x3,
  PLANE_FREE = 0x4,
};

inline constexpr const char *serialize(const VehicleMoveState state) {
  switch (state) {
  case VehicleMoveState::STOP:
    return "VEHICLEMOVESTATE_STOP";
  case VehicleMoveState::MOVE:
    return "VEHICLEMOVESTATE_MOVE";
  case VehicleMoveState::HOVER:
    return "VEHICLEMOVESTATE_HOVER";
  case VehicleMoveState::PLANE_ONCURVE:
    return "VEHICLEMOVESTATE_PLANE_ONCURVE";
  case VehicleMoveState::PLANE_FREE:
    return "VEHICLEMOVESTATE_PLANE_FREE";
  default:
    return "VEHICLEMOVESTATE_INVALID";
  }
}

enum class AwarenessLevel : int32_t {

  UNAWARE = 0x0,
  LOW_ALERT = 0x1,
  HIGH_ALERT = 0x2,
  COMBAT = 0x3,
  COUNT = 0x4,
};

inline constexpr const char *serialize(const AwarenessLevel level) {
  switch (level) {
  case AwarenessLevel::UNAWARE:
    return "AWARENESSLEVEL_UNAWARE";
  case AwarenessLevel::LOW_ALERT:
    return "AWARENESSLEVEL_LOW_ALERT";
  case AwarenessLevel::HIGH_ALERT:
    return "AWARENESSLEVEL_HIGH_ALERT";
  case AwarenessLevel::COMBAT:
    return "AWARENESSLEVEL_COMBAT";
  default:
    return "AWARENESSLEVEL_INVALID";
  }
}

enum class behaviorTreeNodeResult_t : int32_t {

  INVALID = 0x0,
  DEFAULT = 0x0,
  TERMINATE = 0x1,
  TERMINATED = 0x2,
  FAILURE = 0x3,
  SUCCESS = 0x4,
  RUNNING = 0x5,
  COUNT = 0x6,
};

inline constexpr const char *serialize(const behaviorTreeNodeResult_t result) {
  switch (result) {
  case behaviorTreeNodeResult_t::INVALID:
    return "BEHAVIORTREENODE_INVALID";
  case behaviorTreeNodeResult_t::TERMINATE:
    return "BEHAVIORTREENODE_TERMINATE";
  case behaviorTreeNodeResult_t::TERMINATED:
    return "BEHAVIORTREENODE_TERMINATED";
  case behaviorTreeNodeResult_t::FAILURE:
    return "BEHAVIORTREENODE_FAILURE";
  case behaviorTreeNodeResult_t::SUCCESS:
    return "BEHAVIORTREENODE_SUCCESS";
  case behaviorTreeNodeResult_t::RUNNING:
    return "BEHAVIORTREENODE_RUNNING";
  default:
    return "BEHAVIORTREENODE_INVALID";
  }
}

enum class CombatMode : int32_t {
  COVER = 0x0,
  NO_COVER = 0x1,
  COVER_ONLY = 0x2,
  AMBUSH = 0x3,
  AMBUSH_NODES_ONLY = 0x4,
  EXPOSED_NODES_ONLY = 0x5,
  ANY_EXPOSED_NODES_ONLY = 0x6,
  COUNT = 0x7,
};

inline constexpr const char *serialize(const CombatMode mode) {
  switch (mode) {
  case CombatMode::COVER:
    return "AI_COMBAT_COVER";
  case CombatMode::NO_COVER:
    return "AI_COMBAT_NO_COVER";
  case CombatMode::COVER_ONLY:
    return "AI_COMBAT_COVER_ONLY";
  case CombatMode::AMBUSH:
    return "AI_COMBAT_AMBUSH";
  case CombatMode::AMBUSH_NODES_ONLY:
    return "AI_COMBAT_AMBUSH_NODES_ONLY";
  case CombatMode::EXPOSED_NODES_ONLY:
    return "AI_COMBAT_EXPOSED_NODES_ONLY";
  case CombatMode::ANY_EXPOSED_NODES_ONLY:
    return "AI_COMBAT_ANY_EXPOSED_NODES_ONLY";
  default:
    return "AI_COMBAT_INVALID";
  }
}

enum class AlertLevel : int32_t {
  ASLEEP = 0x0,
  NONCOMBAT = 0x1,
  LOW_ALERT = 0x2,
  HIGH_ALERT = 0x3,
  COMBAT = 0x4,
  COUNT = 0x5,
};
inline constexpr const char *serialize(const AlertLevel level) {
  switch (level) {
  case AlertLevel::ASLEEP:
    return "AI_ALERTNESS_ASLEEP";
  case AlertLevel::NONCOMBAT:
    return "AI_ALERTNESS_NONCOMBAT";
  case AlertLevel::LOW_ALERT:
    return "AI_ALERTNESS_LOW_ALERT";
  case AlertLevel::HIGH_ALERT:
    return "AI_ALERTNESS_HIGH_ALERT";
  case AlertLevel::COMBAT:
    return "AI_ALERTNESS_COMBAT";
  default:
    return "AI_ALERTNESS_INVALID";
  }
}

enum class ai_badplace_t : int32_t {
  NONE = 0x0,
  NORMAL = 0x1,
  REALLYBAD = 0x2,
};

inline constexpr const char *serialize(const ai_badplace_t badplace) {
  switch (badplace) {
  case ai_badplace_t::NONE:
    return "AI_BADPLACE_NONE";
  case ai_badplace_t::NORMAL:
    return "AI_BADPLACE_NORMAL";
  case ai_badplace_t::REALLYBAD:
    return "AI_BADPLACE_REALLYBAD";
  default:
    return "AI_BADPLACE_INVALID";
  }
}

enum class ai_animmode_t : int32_t {
  UNKNOWN = 0x0,
  MOVE_CODE = 0x1,
  MOVE_CODE_NOGRAVITY = 0x2,
  USE_POS_DELTAS = 0x3,
  USE_ANGLE_DELTAS = 0x4,
  PROCEDURAL_MODES = 0x4,
  USE_ANGLE_DELTAS_NOCLIP = 0x5,
  USE_BOTH_DELTAS = 0x6,
  USE_BOTH_DELTAS_NOCLIP_STAIRS = 0x7,
  USE_BOTH_DELTAS_NOCLIP = 0x8,
  USE_BOTH_DELTAS_NOGRAVITY = 0x9,
  USE_BOTH_DELTAS_ZONLY_PHYSICS = 0xA,
  NOPHYSICS = 0xB,
  POINT_RELATIVE = 0xC,
  SLIDE = 0xD,
  PRETRAVERSAL = 0xE,
  COUNT = 0xF,
};

inline constexpr const char *serialize(const ai_animmode_t mode) {
  switch (mode) {
  case ai_animmode_t::UNKNOWN:
    return "AI_ANIM_UNKNOWN";
  case ai_animmode_t::MOVE_CODE:
    return "AI_ANIM_MOVE_CODE";
  case ai_animmode_t::MOVE_CODE_NOGRAVITY:
    return "AI_ANIM_MOVE_CODE_NOGRAVITY";
  case ai_animmode_t::USE_POS_DELTAS:
    return "AI_ANIM_USE_POS_DELTAS";
  case ai_animmode_t::USE_ANGLE_DELTAS:
    return "AI_ANIM_USE_ANGLE_DELTAS";
  case ai_animmode_t::USE_ANGLE_DELTAS_NOCLIP:
    return "AI_ANIM_USE_ANGLE_DELTAS_NOCLIP";
  case ai_animmode_t::USE_BOTH_DELTAS:
    return "AI_ANIM_USE_BOTH_DELTAS";
  case ai_animmode_t::USE_BOTH_DELTAS_NOCLIP_STAIRS:
    return "AI_ANIM_USE_BOTH_DELTAS_NOCLIP_STAIRS";
  case ai_animmode_t::USE_BOTH_DELTAS_NOCLIP:
    return "AI_ANIM_USE_BOTH_DELTAS_NOCLIP";
  case ai_animmode_t::USE_BOTH_DELTAS_NOGRAVITY:
    return "AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY";
  case ai_animmode_t::USE_BOTH_DELTAS_ZONLY_PHYSICS:
    return "AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS";
  case ai_animmode_t::NOPHYSICS:
    return "AI_ANIM_NOPHYSICS";
  case ai_animmode_t::POINT_RELATIVE:
    return "AI_ANIM_POINT_RELATIVE";
  case ai_animmode_t::SLIDE:
    return "AI_ANIM_SLIDE";
  case ai_animmode_t::PRETRAVERSAL:
    return "AI_ANIM_PRETRAVERSAL";
  default:
    return "AI_ANIM_INVALID";
  }
}

enum class ai_traverse_mode_t : int32_t {
  INVALID = 0x0,
  GRAVITY = 0x1,
  NOGRAVITY = 0x2,
  NOCLIP = 0x3,
  COUNT = 0x4,
};

inline constexpr const char *serialize(const ai_traverse_mode_t mode) {
  switch (mode) {
  case ai_traverse_mode_t::INVALID:
    return "AI_TRAVERSE_INVALID";
  case ai_traverse_mode_t::GRAVITY:
    return "AI_TRAVERSE_GRAVITY";
  case ai_traverse_mode_t::NOGRAVITY:
    return "AI_TRAVERSE_NOGRAVITY";
  case ai_traverse_mode_t::NOCLIP:
    return "AI_TRAVERSE_NOCLIP";
  default:
    return "AI_TRAVERSE_INVALID";
  }
}

enum class ai_stance_t : int32_t {

  BAD = 0x0,
  STAND = 0x1,
  CROUCH = 0x2,
  PRONE = 0x4,
  ANY = 0x7,
  NONE = 0x0,
};

inline constexpr const char *serialize(const ai_stance_t stance) {
  switch (stance) {
  case ai_stance_t::BAD:
    return "STANCE_BAD";
  case ai_stance_t::STAND:
    return "STANCE_STAND";
  case ai_stance_t::CROUCH:
    return "STANCE_CROUCH";
  case ai_stance_t::PRONE:
    return "STANCE_PRONE";
  case ai_stance_t::ANY:
    return "STANCE_ANY";
  default:
    return "STANCE_INVALID";
  }
}

enum class DamageMode : int32_t {
  NORMAL = 0x0,
  NEXT_SHOT_KILLS = 0x1,
  COUNT = 0x2,
};

inline constexpr const char *serialize(const DamageMode mode) {
  switch (mode) {
  case DamageMode::NORMAL:
    return "AI_DAMAGE_NORMAL";
  case DamageMode::NEXT_SHOT_KILLS:
    return "AI_DAMAGE_NEXT_SHOT_KILLS";
  default:
    return "AI_DAMAGE_INVALID";
  }
}

enum class ai_orient_mode_t : int32_t {
  INVALID = 0x0,
  DONT_CHANGE = 0x1,
  TO_MOTION = 0x2,
  TO_ENEMY = 0x3,
  TO_ENEMY_OR_MOTION_SIDESTEP = 0x4,
  TO_GOAL = 0x5,
  COUNT = 0x6,
};

inline constexpr const char *serialize(const ai_orient_mode_t mode) {
  switch (mode) {
  case ai_orient_mode_t::INVALID:
    return "AI_ORIENT_INVALID";
  case ai_orient_mode_t::DONT_CHANGE:
    return "AI_ORIENT_DONT_CHANGE";
  case ai_orient_mode_t::TO_MOTION:
    return "AI_ORIENT_TO_MOTION";
  case ai_orient_mode_t::TO_ENEMY:
    return "AI_ORIENT_TO_ENEMY";
  case ai_orient_mode_t::TO_ENEMY_OR_MOTION_SIDESTEP:
    return "AI_ORIENT_TO_ENEMY_OR_MOTION_SIDESTEP";
  case ai_orient_mode_t::TO_GOAL:
    return "AI_ORIENT_TO_GOAL";
  default:
    return "AI_ORIENT_INVALID";
  }
}

enum class ai_state_transition_t : int32_t {
  CANONICAL = -1,
  NONE = 0x0,
  SET = 0x1,
  PUSH = 0x2,
  POP = 0x3,
};

inline constexpr const char *serialize(const ai_state_transition_t transition) {
  switch (transition) {
  case ai_state_transition_t::CANONICAL:
    return "AIS_TRANSITION_CANONICAL";
  case ai_state_transition_t::NONE:
    return "AIS_TRANSITION_NONE";
  case ai_state_transition_t::SET:
    return "AIS_TRANSITION_SET";
  case ai_state_transition_t::PUSH:
    return "AIS_TRANSITION_PUSH";
  case ai_state_transition_t::POP:
    return "AIS_TRANSITION_POP";
  default:
    return "AIS_TRANSITION_INVALID";
  }
}

enum class ai_substate_t : int32_t {

  EXPOSED_BEHAVE = 0x64,
  EXPOSED_COMBAT = 0x65,
  EXPOSED_NONCOMBAT = 0x66,
  EXPOSED_REACQUIRE_MOVE = 0x67,
  EXPOSED_REACQUIRE_DONE = 0x68,
  EXPOSED_FLASHBANGED = 0x69,
  EXPOSED_REACTION = 0x6A,
  DEATH_PRECLEANUP = 0xC8,
  DEATH_POSTCLEANUP = 0xC9,
  GRENADE_FLEE = 0x12C,
  GRENADE_TAKECOVER = 0x12D,
  GRENADE_COWER = 0x12E,
  GRENADE_COMBAT = 0x12F,
  GRENADE_COVERATTACK = 0x130,
  GRENADE_ACQUIRE = 0x131,
  GRENADE_THROWBACK = 0x132,
};

inline constexpr const char *serialize(const ai_substate_t substate) {
  switch (substate) {
  case ai_substate_t::EXPOSED_BEHAVE:
    return "STATE_EXPOSED_BEHAVE";
  case ai_substate_t::EXPOSED_COMBAT:
    return "STATE_EXPOSED_COMBAT";
  case ai_substate_t::EXPOSED_NONCOMBAT:
    return "STATE_EXPOSED_NONCOMBAT";
  case ai_substate_t::EXPOSED_REACQUIRE_MOVE:
    return "STATE_EXPOSED_REACQUIRE_MOVE";
  case ai_substate_t::EXPOSED_REACQUIRE_DONE:
    return "STATE_EXPOSED_REACQUIRE_DONE";
  case ai_substate_t::EXPOSED_FLASHBANGED:
    return "STATE_EXPOSED_FLASHBANGED";
  case ai_substate_t::EXPOSED_REACTION:
    return "STATE_EXPOSED_REACTION";
  case ai_substate_t::DEATH_PRECLEANUP:
    return "STATE_DEATH_PRECLEANUP";
  case ai_substate_t::DEATH_POSTCLEANUP:
    return "STATE_DEATH_POSTCLEANUP";
  case ai_substate_t::GRENADE_FLEE:
    return "STATE_GRENADE_FLEE";
  case ai_substate_t::GRENADE_TAKECOVER:
    return "STATE_GRENADE_TAKECOVER";
  case ai_substate_t::GRENADE_COWER:
    return "STATE_GRENADE_COWER";
  case ai_substate_t::GRENADE_COMBAT:
    return "STATE_GRENADE_COMBAT";
  case ai_substate_t::GRENADE_COVERATTACK:
    return "STATE_GRENADE_COVERATTACK";
  case ai_substate_t::GRENADE_ACQUIRE:
    return "STATE_GRENADE_ACQUIRE";
  case ai_substate_t::GRENADE_THROWBACK:
    return "STATE_GRENADE_THROWBACK";
  default:
    return "STATE_INVALID";
  }
}

enum class ai_state_t : int32_t {

  INVALID = 0x0,

  KEEPCURRENT = 0x0,
  BEHAVE = 0x1,
  DEFAULT = 0x1,
  SETABLE_FIRST = 0x1,
  SETABLE_LAST = 0x1,
  SCRIPTEDANIM = 0x2,

  PUSHABLE_FIRST = 0x2,
  PUSHABLE_LAST = 0x2,
  COUNT = 0x3,
};

inline constexpr const char *serialize(const ai_state_t state) {
  switch (state) {
  case ai_state_t::INVALID:
    return "AIS_INVALID";
  case ai_state_t::BEHAVE:
    return "AIS_BEHAVE";
  case ai_state_t::SCRIPTEDANIM:
    return "AIS_SCRIPTEDANIM";
  default:
    return "AIS_INVALID_STATE";
  }
}

enum class AISpecies : int32_t {

  HUMAN = 0x0,
  DOG = 0x1,
  ZOMBIE = 0x2,
  ZOMBIE_DOG = 0x3,
  ROBOT = 0x4,
  COUNT = 0x5,
  ALL = 0x5,
};

inline constexpr const char *serialize(const AISpecies species) {
  switch (species) {
  case AISpecies::HUMAN:
    return "AI_SPECIES_HUMAN";
  case AISpecies::DOG:
    return "AI_SPECIES_DOG";
  case AISpecies::ZOMBIE:
    return "AI_SPECIES_ZOMBIE";
  case AISpecies::ZOMBIE_DOG:
    return "AI_SPECIES_ZOMBIE_DOG";
  case AISpecies::ROBOT:
    return "AI_SPECIES_ROBOT";
  default:
    return "AI_SPECIES_INVALID";
  }
}

PACKED(struct VehicleTurret {
  int32_t fireTime;
  int32_t fireBarrel;
  float barrelOffset;
  int32_t flags;
  float heatVal;
  float turretOnTargetRange;
  int32_t weaponShotCount;
  float accuracy;
  VehicleTurretState turretState;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"fireTime\": {}, \"fireBarrel\": {}, \"barrelOffset\": {}, "
        "\"flags\": 0x{:08X}, \"heatVal\": {}, \"turretOnTargetRange\": {}, "
        "\"weaponShotCount\": {}, \"accuracy\": {}, \"turretState\": {} }}",
        fireTime, fireBarrel, barrelOffset, flags, heatVal, turretOnTargetRange,
        weaponShotCount, accuracy, game::vehicle::serialize(turretState));
  }
});
ASSERT_SIZE(VehicleTurret, 0x24);

PACKED(struct VehicleJitter {
  int32_t jitterPeriodMin;
  int32_t jitterPeriodMax;
  int32_t jitterEndTime;
  vec3_t jitterOffsetRange;
  vec3_t jitterDeltaAccel;
  vec3_t jitterAccel;
  vec3_t jitterPos;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"jitterPeriodMin\": {}, \"jitterPeriodMax\": {}, "
        "\"jitterEndTime\": {}, \"jitterOffsetRange\": {}, "
        "\"jitterDeltaAccel\": {}, \"jitterAccel\": {}, \"jitterPos\": {} }}",
        jitterPeriodMin, jitterPeriodMax, jitterEndTime,
        jitterOffsetRange.serialize(), jitterDeltaAccel.serialize(),
        jitterAccel.serialize(), jitterPos.serialize());
  }
});
ASSERT_SIZE(VehicleJitter, 0x3C);

PACKED(struct VehicleHover {
  float hoverRadius;
  float hoverSpeed;
  float hoverAccel;
  vec3_t hoverGoalPos;
  qboolean useHoverAccelForAngles;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"hoverRadius\": {}, \"hoverSpeed\": {}, \"hoverAccel\": {}, "
        "\"hoverGoalPos\": {}, \"useHoverAccelForAngles\": {} }}",
        hoverRadius, hoverSpeed, hoverAccel, hoverGoalPos.serialize(),
        useHoverAccelForAngles.serialize());
  }
});
ASSERT_SIZE(VehicleHover, 0x1C);

PACKED(struct VehicleGunnerTags {
  BoneIndex turretParent;
  BoneIndex turret;
  BoneIndex barrel;
  BoneIndex aim;
  BoneIndex flash[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"turretParent\": 0x{:04X}, \"turret\": 0x{:04X}, "
                       "\"barrel\": 0x{:04X}, "
                       "\"aim\": 0x{:04X}, \"flash\": {} }}",
                       turretParent, turret, barrel, aim, ::serialize(flash));
  }
});
ASSERT_SIZE(VehicleGunnerTags, 0x10);

PACKED(struct VehicleTags {
  BoneIndex player;
  BoneIndex popout;
  BoneIndex body;
  BoneIndex eye;
  VehicleGunnerTags turretTags;
  VehicleGunnerTags gunnerTags[4];
  BoneIndex wheel[6];
  BoneIndex seats[11];
  BoneIndex entryPoints[5];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"player\": 0x{:04X}, \"popout\": 0x{:04X}, \"body\": "
        "{:04X}, \"eye\": 0x{:04X}, "
        "\"turretTags\": {}, \"gunnerTags\": {}, \"wheel\": {}, "
        "\"seats\": {}, \"entryPoints\": {} }}",
        player, popout, body, eye, turretTags.serialize(),
        ::serialize(gunnerTags), ::serialize(wheel), ::serialize(seats),
        ::serialize(entryPoints));
  }
});
ASSERT_SIZE(VehicleTags, 0x84);

PACKED(struct VehicleCornerCache {
  vec3_t start;
  vec3_t end;
  vec3_t center;
  vec3_t travelVector;
  int32_t lastUpdateTime;
  int32_t startSlidingTime;
  float travelDistance;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"start\": {}, \"end\": {}, \"center\": {}, "
                       "\"travelVector\": {}, \"lastUpdateTime\": {}, "
                       "\"startSlidingTime\": {}, \"travelDistance\": {} }}",
                       start.serialize(), end.serialize(), center.serialize(),
                       travelVector.serialize(), lastUpdateTime,
                       startSlidingTime, travelDistance);
  }
});
ASSERT_SIZE(VehicleCornerCache, 0x3C);

PACKED(struct VehicleTarget {
  VehicleTargetType targetType;
  int32_t targetEnt;
  vec3_t targetOrigin;
  vec3_t targetOffset;
  vec3_t targetRelativeAngles;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"targetType\": {}, \"targetEnt\": {}, \"targetOrigin\": {}, "
        "\"targetOffset\": {}, \"targetRelativeAngles\": {} }}",
        game::vehicle::serialize(targetType), targetEnt,
        targetOrigin.serialize(), targetOffset.serialize(),
        targetRelativeAngles.serialize());
  }
});
ASSERT_SIZE(VehicleTarget, 0x2C);

enum class ClientFallSpeed : int32_t {
  NONE = 0x0,
  DEFAULT = 0x0,
  SLOW = 0x1,
  MED = 0x2,
  FAST = 0x3,
  RAPID = 0x4,
  COUNT = 0x5,
};

inline constexpr const char *serialize(const ClientFallSpeed speed) {
  switch (speed) {
  case ClientFallSpeed::NONE:
    return "CLIENTFALLSPEED_NONE";
  case ClientFallSpeed::SLOW:
    return "CLIENTFALLSPEED_SLOW";
  case ClientFallSpeed::MED:
    return "CLIENTFALLSPEED_MED";
  case ClientFallSpeed::FAST:
    return "CLIENTFALLSPEED_FAST";
  case ClientFallSpeed::RAPID:
    return "CLIENTFALLSPEED_RAPID";
  default:
    return "CLIENTFALLSPEED_INVALID";
  }
}

enum class ClientVisibleBodyMode : int32_t {
  UNKNOWN = 0x0,
  FULL = 0x1,
  FIRST_PERSON_CINEMATIC = 0x2,
  FIRST_PERSON_LEGS = 0x3,
};

inline constexpr const char *serialize(const ClientVisibleBodyMode mode) {
  switch (mode) {
  case ClientVisibleBodyMode::UNKNOWN:
    return "CVBM_UNKNOWN";
  case ClientVisibleBodyMode::FULL:
    return "CVBM_FULL";
  case ClientVisibleBodyMode::FIRST_PERSON_CINEMATIC:
    return "CVBM_FIRST_PERSON_CINEMATIC";
  case ClientVisibleBodyMode::FIRST_PERSON_LEGS:
    return "CVBM_FIRST_PERSON_LEGS";
  default:
    return "CVBM_INVALID";
  }
}

enum class ShockViewTypes : int32_t {
  BLURRED = 0x0,
  FLASHED = 0x1,
  CONCUSSED = 0x2,
  SHOCKED = 0x3,
  NONE = 0x4,
  COUNT = 0x5,
};

inline constexpr const char *serialize(const ShockViewTypes type) {
  switch (type) {
  case ShockViewTypes::BLURRED:
    return "SHOCKVIEW_BLURRED";
  case ShockViewTypes::FLASHED:
    return "SHOCKVIEW_FLASHED";
  case ShockViewTypes::CONCUSSED:
    return "SHOCKVIEW_CONCUSSED";
  case ShockViewTypes::SHOCKED:
    return "SHOCKVIEW_SHOCKED";
  case ShockViewTypes::NONE:
    return "SHOCKVIEW_NONE";
  default:
    return "SHOCKVIEW_INVALID";
  }
}

enum class VehicleAnimState : int32_t {
  IDLE = 0x0,
  ENTRY = 0x1,
  CHANGEPOS = 0x2,
  EXIT = 0x3,
  COUNT = 0x4,
};

inline constexpr const char *serialize(const VehicleAnimState state) {
  switch (state) {
  case VehicleAnimState::IDLE:
    return "VEHICLEANIM_IDLE";
  case VehicleAnimState::ENTRY:
    return "VEHICLEANIM_ENTRY";
  case VehicleAnimState::CHANGEPOS:
    return "VEHICLEANIM_CHANGEPOS";
  case VehicleAnimState::EXIT:
    return "VEHICLEANIM_EXIT";
  default:
    return "VEHICLEANIM_INVALID";
  }
}

typedef phys::colgeom_visitor_inlined_t<300> vehicle_proximity_data_t;
ASSERT_SIZE(vehicle_proximity_data_t, 0x1350);

PACKED(struct vehicle_cache_t {
  vec3_t lastOrigin;
  vec3_t lastAngles;
  int32_t hit_indices[6];
  int32_t hit_sflags[6];
  vec3_t hit_normals[6];
  float hit_fractions[6];
  TraceHitType hit_type[6];
  uint16_t hit_id[6];
  uint8_t _paddingCC[4];
  vehicle_proximity_data_t proximity_data;
  int32_t wheel_mask;
  uint8_t _padding1424[12];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"lastOrigin\": {}, \"lastAngles\": {}, \"hit_indices\": {}, "
        "\"hit_sflags\": {}, \"hit_normals\": {}, \"hit_fractions\": {}, "
        "\"hit_type\": {}, \"hit_id\": {}, \"proximity_data\": {}, "
        "\"wheel_mask\": 0x{:08X} }}",
        lastOrigin.serialize(), lastAngles.serialize(),
        ::serialize(hit_indices), ::serialize(hit_sflags),
        ::serialize(hit_normals), ::serialize(hit_fractions),
        ::serialize(hit_type), ::serialize(hit_id), "\"SKIPPED\"", wheel_mask);
  }
});
ASSERT_SIZE(vehicle_cache_t, 0x1430);

PACKED(struct VehicleSeat {
  int32_t occupantEntNum;
  bool scriptOccupied;
  uint8_t _padding5[3];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"occupantEntNum\": {}, \"scriptOccupied\": {} }}",
                       occupantEntNum, ::serialize(scriptOccupied));
  }
});
ASSERT_SIZE(VehicleSeat, 0x8);

PACKED(struct vehicle_t {
  vehicle_pathpos_t pathPos;
  int32_t pathOffsetFlags;
  vec3_t pathOffsetTransform[4];
  vec3_t pathFixedOffset;
  vec3_t pathVariableOffset;
  vec3_t pathVariableOffsetTargetPrev;
  vec3_t pathVariableOffsetTarget;
  float pathVariableOffsetRate;
  float pathVariableOffsetTime;
  vehicle_physic_t phys;
  int32_t entNum;
  int16_t defIndex;
  uint8_t _padding2EE[2];
  VehicleFlags flags;
  VehicleMoveState moveState;
  VehicleTurret turret;
  float turretRotScale;
  VehicleJitter jitter;
  VehicleHover hover;
  VehicleTurret gunnerTurrets[4];
  VehicleJitter gunnerJitter[4];
  float minigunRotationSpeed;
  int32_t manualMode;
  float manualSpeed;
  float manualAccel;
  float manualDecel;
  float speed;
  float maxSpeedOverride;
  float maxDragSpeed;
  float turningAbility;
  qboolean hasGoalYaw;
  qboolean stopAtGoal;
  qboolean stopping;
  qboolean hasLookAtOrigin;
  EntHandle lookAtEnt;
  vec3_t lookAtOrigin;
  vec3_t goalPosition;
  float goalYaw;
  VehicleCornerCache cornerCache;
  qboolean hasTargetYaw;
  float targetYaw;
  float minDesiredTurnYaw;
  float prevDesiredYaw;
  float pathTransitionTime;
  float maxPathTransitionTime;
  vec3_t pathTransitionOffset;
  vec3_t pathTransitionAngleOffset;
  qboolean hasDefaultPitch;
  float defaultPitch;
  float hasGoalRoll;
  float goalRoll;
  float goalRollTime;
  float currentRollTime;
  int32_t numRolls;
  VehicleTarget mainTurretTarget;
  VehicleTarget gunnerTargets[4];
  float nearGoalNotifyDist;
  vec2_t joltDir;
  float joltTime;
  float joltWave;
  float joltSpeed;
  float joltDecel;
  VehicleTags boneIndex;
  int32_t turretHitNum;
  VehicleSeat seats[11];
  qboolean secTurretAiControlled;
  qboolean addToCompass;
  uint8_t _padding7B4[4];
  NitrousVehicle *nitrousVehicle;
  vehicle_cache_t vehicle_cache;
  float heliHeightLockOffset;
  float lastViewOffset;
  int32_t driveBySoundIndex[2];
  float driveBySoundDelay[2];
  float driveBySoundTimeout[2];
  ai::sentient_t *sentient;
  ai::AIBody aiBody;
  uint8_t tmodeVehicleVisibility;
  uint8_t _padding22A1[3];
  float predictedCollisionTime;
  uint8_t _padding22A8[8];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"pathOffsetFlags\": 0x{:08X}, \"pathOffsetTransform\": {}, "
        "\"pathFixedOffset\": {}, \"pathVariableOffset\": {}, "
        "\"entNum\": {}, \"defIndex\": {}, \"flags\": {}, \"moveState\": {}, "
        "\"turret\": {}, \"turretRotScale\": {}, \"jitter\": {}, "
        "\"hover\": {}, \"gunnerTurrets\": {}, \"gunnerJitter\": {}, "
        "\"speed\": {}, \"hasGoalYaw\": {}, \"stopAtGoal\": {}, "
        "\"stopping\": {}, \"hasLookAtOrigin\": {}, \"lookAtEnt\": {}, "
        "\"cornerCache\": {}, \"hasTargetYaw\": {}, \"boneIndex\": {}, "
        "\"seats\": {}, \"secTurretAiControlled\": {}, \"addToCompass\": {}, "
        "\"nitrousVehicle\": {:p}, \"vehicle_cache\": {}, "
        "\"tmodeVehicleVisibility\": {} }}",
        pathOffsetFlags, ::serialize(pathOffsetTransform),
        pathFixedOffset.serialize(), pathVariableOffset.serialize(), entNum,
        defIndex, flags.serialize(), game::vehicle::serialize(moveState),
        turret.serialize(), turretRotScale, jitter.serialize(),
        hover.serialize(), ::serialize(gunnerTurrets),
        ::serialize(gunnerJitter), speed, hasGoalYaw.serialize(),
        stopAtGoal.serialize(), stopping.serialize(),
        hasLookAtOrigin.serialize(), lookAtEnt.serialize(),
        cornerCache.serialize(), hasTargetYaw.serialize(),
        boneIndex.serialize(), ::serialize(seats),
        secTurretAiControlled.serialize(), addToCompass.serialize(),
        reinterpret_cast<const void *>(nitrousVehicle),
        vehicle_cache.serialize(), tmodeVehicleVisibility);
  }
});
ASSERT_OFFSET(vehicle_t, nitrousVehicle, 0x7B8);
ASSERT_SIZE(vehicle_t, 0x22B8);

PACKED(struct vehicleAntilagFrame_t {
  vec3_t position[64];
  vec3_t angles[64];
  uint8_t useCount[64];
  uint8_t inUse[64];
  int32_t time;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"position\": {}, \"angles\": {}, \"useCount\": {}, "
                       "\"inUse\": {}, \"time\": {} }}",
                       ::serialize(position), ::serialize(angles),
                       ::serialize(useCount), ::serialize(inUse), time);
  }
});
ASSERT_SIZE(vehicleAntilagFrame_t, 0x684);

PACKED(struct PlayerVehicleState {
  vec3_t origin;
  vec3_t angles;
  vec3_t velocity;
  vec3_t angVelocity;
  vec2_t tilt;
  vec2_t tiltVelocity;
  float targetHeightDelta;
  float lastGroundHeight;
  int32_t entity;
  int32_t flags;
  bool fullPhysics;
  uint8_t _padding51[3];
  int32_t focusEntNum;
  int32_t focusEntTime;
  vec2_t focusAngleOffset;
  uint32_t playerHeightMapsActive;
  float boostTimeLeft;
  vec3_t dogfightDesiredLocation;
  vec3_t jitterEndTime;
  vec3_t jitterAccel;
  int32_t drivableDuration;
  int32_t drivableEndTime;
  bool weaponOverheating;
  uint8_t _padding99[3];
  float weaponHeatPercent;
  float weaponLockonPercent;
  float damageMeter;
  int32_t lockedOnByEntNum;
  int32_t weaponWaitDuration;
  int32_t weaponWaitEndTime;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"origin\": {}, \"angles\": {}, \"velocity\": {}, "
        "\"angVelocity\": {}, \"tilt\": {}, \"tiltVelocity\": {}, "
        "\"targetHeightDelta\": {}, \"lastGroundHeight\": {}, "
        "\"entity\": {}, \"flags\": 0x{:08X}, \"fullPhysics\": {}, "
        "\"focusEntNum\": {}, \"focusEntTime\": {}, \"focusAngleOffset\": "
        "{}, \"playerHeightMapsActive\": 0x{:08X}, \"boostTimeLeft\": {}, "
        "\"dogfightDesiredLocation\": {}, \"jitterEndTime\": {}, "
        "\"jitterAccel\": {}, \"drivableDuration\": {}, "
        "\"drivableEndTime\": {}, \"weaponOverheating\": {}, "
        "\"weaponHeatPercent\": {}, \"weaponLockonPercent\": {}, "
        "\"damageMeter\": {}, \"lockedOnByEntNum\": {}, "
        "\"weaponWaitDuration\": {}, \"weaponWaitEndTime\": {} }}",
        origin.serialize(), angles.serialize(), velocity.serialize(),
        angVelocity.serialize(), tilt.serialize(), tiltVelocity.serialize(),
        targetHeightDelta, lastGroundHeight, entity, flags,
        ::serialize(fullPhysics), focusEntNum, focusEntTime,
        focusAngleOffset.serialize(), playerHeightMapsActive, boostTimeLeft,
        dogfightDesiredLocation.serialize(), jitterEndTime.serialize(),
        jitterAccel.serialize(), drivableDuration, drivableEndTime,
        ::serialize(weaponOverheating), weaponHeatPercent, weaponLockonPercent,
        damageMeter, lockedOnByEntNum, weaponWaitDuration, weaponWaitEndTime);
  }
});
ASSERT_SIZE(PlayerVehicleState, 0xB4);

PACKED(struct vehicleState_t {
  VehicleFlags flags;
  int16_t animId;
  int16_t attachModelIndex[2];
  uint8_t attachTagIndex[2];
  uint8_t vehicleDefIndex;
  uint8_t _padding0D[3];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"flags\": {}, \"animId\": {}, \"attachModelIndex\": {}, "
        "\"attachTagIndex\": {}, \"vehicleDefIndex\": {} }}",
        flags.serialize(), animId, ::serialize(attachModelIndex),
        ::serialize(attachTagIndex), vehicleDefIndex);
  }
});

PACKED(struct VehicleLocalPhysics {
  cm::trace_t groundTrace;
  qboolean hasGround;
  qboolean onGround;
  uint8_t _unknown[0x2a8];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"groundTrace\": {}, \"hasGround\": {}, \"onGround\": {} }}",
        groundTrace.serialize(), hasGround.serialize(), onGround.serialize());
  }
});
ASSERT_SIZE(VehicleLocalPhysics, 0x300);

typedef fastcallPtr_t<const VehicleDef *(LocalClientNum_t localClientNum,
                                         const level::playerState_t *ps)>
    vehicle_def;

typedef fastcallPtr_t<vehicle_proximity_data_t *(
    LocalClientNum_t localClientNum, const level::playerState_t *ps)>
    get_vehicle_proximity_cache;

typedef fastcallPtr_t<qboolean(const move::pmove_t *pm, int, vec3_t *)>
    get_wheel;
typedef fastcallPtr_t<const db::xasset::phys::PhysGeomList *(
    LocalClientNum_t localClientNum, const int)>
    get_vehicle_collmap;

typedef fastcallPtr_t<void(LocalClientNum_t localClientNum, const int,
                           const vec3_t *, const vec3_t *, const int)>
    vehicle_collision_notifies;

struct VehicleHandler {
  vehicle_def vehicle;
  get_wheel wheel;
  get_vehicle_proximity_cache proximityCache;
  get_vehicle_collmap collmap;
  vehicle_collision_notifies notifies;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"vehicle\": {:p}, \"wheel\": {:p}, \"proximityCache\": {:p}, "
        "\"collmap\": {:p}, \"notifies\": {:p} }}",
        reinterpret_cast<const void *>(vehicle),
        reinterpret_cast<const void *>(wheel),
        reinterpret_cast<const void *>(proximityCache),
        reinterpret_cast<const void *>(collmap),
        reinterpret_cast<const void *>(notifies));
  }
};

PACKED(struct VehicleState {
  int32_t entity;
  vec3_t origin;
  vec3_t angles;
  vec3_t velocity;
  vec3_t angVelocity;
  vec3_t previousOrigin;
  vec3_t previousAngles;
  vec3_t bodyVelocity;
  vec2_t tilt;
  vec2_t tiltVelocity;
  vec3_t mins;
  vec3_t maxs;
  float targetHeightDelta;
  float lastGroundHeight;
  int32_t clipmask;
  vec3_t groundNormal;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"entity\": {}, \"origin\": {}, \"angles\": {}, \"velocity\": "
        "{}, \"angVelocity\": {}, \"previousOrigin\": {}, "
        "\"previousAngles\": {}, \"bodyVelocity\": {}, \"tilt\": {}, "
        "\"tiltVelocity\": {}, \"mins\": {}, \"maxs\": {}, "
        "\"targetHeightDelta\": {}, \"lastGroundHeight\": {}, "
        "\"clipmask\": 0x{:08X}, \"groundNormal\": {} }}",
        entity, origin.serialize(), angles.serialize(), velocity.serialize(),
        angVelocity.serialize(), previousOrigin.serialize(),
        previousAngles.serialize(), bodyVelocity.serialize(), tilt.serialize(),
        tiltVelocity.serialize(), mins.serialize(), maxs.serialize(),
        targetHeightDelta, lastGroundHeight, clipmask,
        groundNormal.serialize());
  }
});
ASSERT_SIZE(VehicleState, 0x98);

} // namespace vehicle
} // namespace game
