#pragma once

#include <cstdint>
#include "../core.hpp"
#include "../quake/core.hpp"
#include <structs/enum.hpp>

namespace game {
namespace ui {
namespace he {

constexpr uint32_t MAX_HUDELEM_MESSAGE_LEN = 0x400;
constexpr auto HUD_ELEMENT_POOL_SIZE = 0x45C;
template <typename T> struct HudElementPool {
  static inline constexpr auto SIZE = HUD_ELEMENT_POOL_SIZE;
  // Minimum, optimized primitive for indexing the pool that can contain all
  // values 0 < index < SIZE
  using index_t = uint16_t;
  array<T, SIZE> pool;

  template <IntegralLike Index>
  inline constexpr bool valid_index(Index index) const noexcept {
    return static_cast<size_t>(index) < SIZE;
  }
  inline constexpr void assert_range(size_t index) const {
    assert(valid_index(index) &&
           "index to HudElementPool must be within range 0 <= index < 0x45C");
  }

  template <IntegralLike Index>
  inline constexpr const T &operator[](Index index_arg) const {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike Index>
  inline constexpr const T &get(Index index_arg) const {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike Index> inline constexpr T &get(Index index_arg) {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike Index> inline constexpr T &operator[](Index index) {
    return get(index);
  }

  inline constexpr auto size() const noexcept { return SIZE; }

  template <IntegralLike Index>
  inline constexpr void clear(Index index_arg) noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    const T default_val = {};
    this->pool[index] = default_val;
  }

  inline constexpr void clear() noexcept {
    for (index_t i = 0; i < size(); ++i) {
      clear(i);
    }
  }
};

template <typename T>
struct AtomicHudElementPool : public HudElementPool<std::atomic<T>> {
  // Minimum, optimized primitive for indexing the pool that can contain all
  // values 0 < index < SIZE
  using index_t = uint16_t;

  template <IntegralLike Index>
  inline constexpr void clear(Index index) noexcept {
    const T default_val = {};
    this->pool[index].store(default_val, std::memory_order_release);
  }

  inline constexpr void clear() noexcept {
    for (index_t i = 0; i < this->size(); ++i) {
      clear(i);
    }
  }
};

template <typename T>
struct AtomicOptionalHudElementPool
    : public HudElementPool<atomic_optional<T>> {
  // Minimum, optimized primitive for indexing the pool that can contain all
  // values 0 < index < SIZE
  using index_t = uint16_t;

  template <IntegralLike Index>
  inline constexpr void clear(Index index) noexcept {
    this->pool[index].store(std::nullopt, std::memory_order_release);
  }

  inline constexpr void clear() noexcept {
    for (index_t i = 0; i < this->size(); ++i) {
      clear(i);
    }
  }
};

union hudelem_color_t {
  struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
  };
  int32_t rgba;
};
ASSERT_SIZE(hudelem_color_t, 0x4);

template <IntegralLike Underlying> struct HEType : public Enum<Underlying> {
public:
  static inline constexpr Enum<Underlying> FREE = {0x0};
  static inline constexpr Enum<Underlying> TEXT = {0x1};
  static inline constexpr Enum<Underlying> VALUE = {0x2};
  static inline constexpr Enum<Underlying> PLAYERNAME = {0x3};
  static inline constexpr Enum<Underlying> MAPNAME = {0x4};
  static inline constexpr Enum<Underlying> GAMETYPE = {0x5};
  static inline constexpr Enum<Underlying> DAMAGE_INDICATOR = {0x6};
  static inline constexpr Enum<Underlying> SCORE = {0x7};
  static inline constexpr Enum<Underlying> MATERIAL = {0x8};
  static inline constexpr Enum<Underlying> TIMER_DOWN = {0x9};
  static inline constexpr Enum<Underlying> TIMER_UP = {0xA};
  static inline constexpr Enum<Underlying> TENTHS_TIMER_DOWN = {0xB};
  static inline constexpr Enum<Underlying> TENTHS_TIMER_UP = {0xC};
  static inline constexpr Enum<Underlying> CLOCK_DOWN = {0xD};
  static inline constexpr Enum<Underlying> CLOCK_UP = {0xE};
  static inline constexpr Enum<Underlying> WAYPOINT = {0xF};
  static inline constexpr Enum<Underlying> PERKS = {0x10};
  static inline constexpr Enum<Underlying> COUNT = {0x11};
};

typedef HEType<uint32_t> he_type_t;
ASSERT_SIZE(he_type_t, sizeof(uint32_t));
ASSERT_CPP03_POD(he_type_t);

typedef HEType<uint8_t> he_type_field_t;
ASSERT_SIZE(he_type_field_t, sizeof(uint8_t));
ASSERT_CPP03_POD(he_type_field_t);

// sizeof=0x7C
PACKED(struct hudelem_s {
  float x;
  float y;
  float z;
  float fontScale;
  float fromFontScale;
  int32_t fontScaleStartTime;
  hudelem_color_t color;
  hudelem_color_t fromColor;
  int32_t fadeStartTime;
  int32_t scaleStartTime;
  float fromX;
  float fromY;
  int32_t moveStartTime;
  int32_t time;
  int32_t duration;
  float value;
  float sort;
  hudelem_color_t glowColor;
  int32_t fxBirthTime;
  uint32_t flags;
  short targetEntNum;
  short fontScaleTime;
  short fadeTime;
  short label;
  short width;
  short height;
  short fromWidth;
  short fromHeight;
  short scaleTime;
  short moveTime;
  short text;
  uint16_t fxLetterTime;
  uint16_t fxDecayStartTime;
  uint16_t fxDecayDuration;
  uint16_t fxRedactDecayStartTime;
  uint16_t fxRedactDecayDuration;
  he_type_field_t type;
  uint8_t font;
  uint8_t alignOrg;
  uint8_t alignScreen;
  uint16_t materialIndex;
  uint16_t offscreenMaterialIdx;
  uint8_t fromAlignOrg;
  uint8_t fromAlignScreen;
  uint8_t soundID;
  uint8_t _padding[1];

  inline constexpr void reset_value() noexcept {
    width = 0;
    height = 0;
    fromWidth = 0;
    fromHeight = 0;
    materialIndex = 0;
    fromX = 0.0;
    fromY = 0.0;
    fromAlignOrg = 0;
    fromAlignScreen = 0;
    soundID = 0;
    // TODO: Is this needed? Added for now in case this is a new field
    _padding[0] = 0;
    scaleStartTime = 0;
    scaleTime = 0;
    time = 0;
    duration = 0;
    value = 0.0;
    text = 0;
  }
});
ASSERT_SIZE(hudelem_s, 0x7C);

typedef hudelem_s hudelem_t;

struct game_hudelem_s {
  hudelem_t elem;
  ClientNum_t clientNum;
  team_t team;
  qboolean archived;

  inline constexpr void reset_value() noexcept { elem.reset_value(); }
};
ASSERT_SIZE(game_hudelem_s, 0x88);
typedef game_hudelem_s game_hudelem_t;

#pragma pack(push, 1)
struct serverHudMenu_t {
  UIModelData modelData[16];
  int32_t menuNameIndex;
  int32_t modelNames[16];
  bool persistent;
  uint8_t _padding145[11];
};
ASSERT_SIZE(serverHudMenu_t, 0x150);
#pragma pack(pop)

struct hudElemSoundInfo_t {
  int lastPlayedTime;
};
} // namespace he
} // namespace ui
} // namespace game