#pragma once

#include <game/structs/macros.hpp>
#include <game/structs/db/xasset/core.hpp>

#include <cstdint>
#include <str.hpp>
#include <hash.hpp>

namespace game {
namespace bg {
namespace cache {
enum class bgCacheInstance : uint32_t {
  SERVER = 0x0,
  CLIENT = 0x1,
  COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(bgCacheInstance);

template <typename T> union BGCacheInstancePool {

  T pool[+bgCacheInstance::COUNT];
  struct {
    T server;
    T client;
  };
  using index_t = uint8_t;

  inline constexpr void assert_range(size_t index) const {
    assert(index < +bgCacheInstance::COUNT &&
           "index to BGCacheInstancePool must be within range "
           "bgCacheInstance::SERVER <= index < bgCacheInstance::COUNT");
  }

  inline constexpr void assert_range(size_t index) volatile {
    assert(index < +bgCacheInstance::COUNT &&
           "index to BGCacheInstancePool must be within range "
           "bgCacheInstance::SERVER <= index < bgCacheInstance::COUNT");
  }

  template <IntegralLike Index>
  inline constexpr const T &get(Index index_arg) const {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike Index>
  inline constexpr const T &operator[](Index index) const {
    return get(index);
  }

  template <IntegralLike Index> inline constexpr T &get(Index index_arg) {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }
  template <IntegralLike Index> inline constexpr T &operator[](Index index) {
    return get(index);
  }

  template <IntegralLike Index>
  inline constexpr volatile T &get(Index index_arg) volatile {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike Index>
  inline constexpr volatile T &operator[](Index index) volatile {
    return get(index);
  }

  inline constexpr auto size() const noexcept {
    return +bgCacheInstance::COUNT;
  }

  inline constexpr auto size() volatile noexcept {
    return +bgCacheInstance::COUNT;
  }
};

enum class ConfigStringStart : int32_t {
  TriggerString = 0x1AF,
  LocalizedString = 0x2A9,
  DebugString = 0xAA9,
  Invalid = -1 // Any other value
};
IMPL_ENUM_OPERATORS(ConfigStringStart);

enum class BGCacheTypes : uint32_t {
  INVALID = 0x0,
  VEHICLE = 0x1,
  MODEL = 0x2,
  PLAYER_CHARACTER = 0x3,
  AITYPE = 0x4,
  CHARACTER = 0x5,
  XMODELALIAS = 0x6,
  WEAPON = 0x7,
  ZBARRIER = 0x8,
  RUMBLE = 0x9,
  SHELLSHOCK = 0xA,
  XCAM = 0xB,
  DESTRUCTIBLE = 0xC,
  STREAMERHINT = 0xD,
  HEAD_ICON = 0xE,
  STATUS_ICON = 0xF,
  LOCATION_SELECTOR = 0x10,
  SCRIPT_MENU = 0x11,
  SERVER_MATERIAL = 0x12,
  LOCSTRING = 0x13,
  DEBUGSTRING = 0x14,
  EVENT_STRING = 0x15,
  TRIGGER_STRING = 0x16,
  OBJECTIVE_STRING = 0x17,
  FX = 0x18,
  LUI_MENU = 0x19,
  LUI_MENU_DATA = 0x1A,
  CLIENT_FX = 0x1B,
  CLIENT_TAGFXSET = 0x1C,
  COUNT = 0x1D,
};
IMPL_ENUM_OPERATORS(BGCacheTypes);

typedef fastcallPtr_t<int32_t(bgCacheInstance inst, const char *name)>
    BG_Cache_RegisterFunc;
typedef fastcallPtr_t<void(bgCacheInstance inst, const char *name)>
    BG_Cache_UnregisterFunc;
PACKED(struct bgCacheTypeInfo {
  const char *name;
  db::xasset::XAssetType assetType;
  uint8_t _padding0C[4];
  BG_Cache_RegisterFunc registerFunc;
  BG_Cache_UnregisterFunc unregisterFunc;
  int32_t configStringStart;
  uint8_t _padding24[4];
});

template <typename T> union BGCacheTypePool {
  T pool[+BGCacheTypes::COUNT];

  struct {
    T invalid;
    T vehicle;
    T model;
    T player_character;
    T aitype;
    T character;
    T xmodelalias;
    T weapon;
    T zbarrier;
    T rumble;
    T shellshock;
    T xcam;
    T destructible;
    T streamerhint;
    T head_icon;
    T status_icon;
    T location_selector;
    T script_menu;
    T server_material;
    T locstring;
    T debugstring;
    T event_string;
    T trigger_string;
    T objective_string;
    T fx;
    T lui_menu;
    T lui_menu_data;
    T client_fx;
    T client_tagfxset;
  };

  // Minimum, optimized primitive for indexing the pool that can contain all
  // values 0 < index < BGCachedypes::COUNT
  using index_t = uint8_t;

  static inline constexpr void assert_range(size_t index) {
    assert(index > +BGCacheTypes::INVALID && index < +BGCacheTypes::COUNT &&
           "index to BGCacheTypePool must be within range "
           "BGCacheTypes::INVALID < index < BGCacheTypes::COUNT");
  }

  template <IntegralLike Index>
  inline constexpr const T &get(Index index_arg) const {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }
  template <IntegralLike Index>
  inline constexpr const T &operator[](Index index) const {
    return get(index);
  }

  template <IntegralLike Index> inline constexpr T &get(Index index_arg) {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }
  template <IntegralLike Index> inline constexpr T &operator[](Index index) {
    return get(index);
  }

  template <IntegralLike Index>
  inline constexpr volatile T &get(Index index_arg) volatile {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }
  template <IntegralLike Index>
  inline constexpr volatile T &operator[](Index index) volatile {
    return get(index);
  }

  inline constexpr auto size() const noexcept { return +BGCacheTypes::COUNT; }

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

PACKED(struct bgCacheChecksum {
  volatile bool isDirty;
  uint8_t _padding01[3];
  volatile uint32_t value;
});

template <typename Asset> struct bgCachedData {
  Asset *def;
  int32_t nameHash;
  volatile uint8_t refCount;
  uint8_t _padding0D[3];

  inline void reset() noexcept {
    memset(this, 0, sizeof(std::remove_pointer_t<decltype(this)>));
  }
};
typedef bgCachedData<db::xasset::XModel> bgCachedModels;
typedef bgCachedData<db::xasset::RumbleInfo> bgCachedRumbles;
typedef bgCachedData<db::xasset::ShellshockParams> bgCachedShellshocks;
typedef bgCachedData<db::xasset::XCam> bgCachedXCams;
typedef bgCachedData<db::xasset::DestructibleDef> bgCachedDestructibles;
typedef bgCachedData<db::xasset::StreamerHint> bgCachedStreamerHint;
typedef bgCachedData<db::xasset::FxEffectDefHandle> bgCachedFX;
typedef bgCachedData<db::xasset::TagFxSet> bgCachedTagFxSet;

typedef djb2Hash_t BGCacheNameHash;
PACKED(struct bgCachedGenericData {
  str1024_t name;
  int32_t nameHash;
  volatile uint8_t refCount;
  uint8_t _padding0D[3];

  static inline constexpr BGCacheNameHash BGCACHE_NAMEHASH_NULLPTR = 0;
  static inline constexpr BGCacheNameHash BGCACHE_NAMEHASH_DJB2_INITIAL_SEED =
      0x1505;
  static inline constexpr BGCacheNameHash BGCACHE_NAMEHASH_DJB2_CONSTANT = 0x21;

  inline constexpr void clearName() volatile {
    nameHash = BGCACHE_NAMEHASH_NULLPTR;
    for (size_t i = 0; i < std::size(name); ++i) {
      name[i] = '\0';
    }
  }

  inline constexpr void setName(const char *new_name) volatile {
    if (new_name) {
      nameHash = djb2<BGCACHE_NAMEHASH_DJB2_INITIAL_SEED,
                      BGCACHE_NAMEHASH_DJB2_CONSTANT>(new_name);
      strscpy(name, new_name);
    } else {
      clearName();
    }
  }

  inline void reset() volatile noexcept {
    memset(const_cast<bgCachedGenericData *>(this), 0,
           sizeof(std::remove_pointer_t<decltype(this)>));
  }

  inline uint32_t remove_ref() volatile noexcept {
#if defined(__clang__) || defined(__GNUC__)
    __atomic_sub_fetch(&refCount, 1, __ATOMIC_SEQ_CST);
#elif defined(_MSC_VER)
    _InterlockedDecrement8(reinterpret_cast<volatile char*>(&refCount));
#else
       // Unsupported, but we have an obvious fallback for this case.
      --refCount;
#endif
    return refCount;
  }

  inline uint32_t add_ref() volatile noexcept {
#if defined(__clang__) || defined(__GNUC__)
    __atomic_add_fetch(&refCount, 1, __ATOMIC_SEQ_CST);
#elif defined(_MSC_VER)
    _InterlockedIncrement8(reinterpret_cast<volatile char*>(&refCount));
#else
       // Unsupported, but we have an obvious fallback for this case.
      ++refCount;
#endif
    return refCount;
  }
});
ASSERT_SIZE(bgCachedGenericData, 0x408);
// Verified
struct bgCacheDataSet {
  bgCachedModels models[0x4000];
  qboolean weaponsRegistered[0x200];
  bgCachedRumbles rumbles[0x80];
  bgCachedShellshocks shellshocks[0x40];
  bgCachedXCams xcams[0x100];
  bgCachedDestructibles destructibles[0x40];
  bgCachedStreamerHint streamerHints[0x40];
  bgCachedFX serverFX[0x100];
  bgCachedFX clientFX[0x400];
  bgCachedTagFxSet clientTagFxSet[0x40];
  bgCachedGenericData headIcons[0xF];
  bgCachedGenericData statusIcons[0x8];
  bgCachedGenericData locationSelectorIcons[0xF];
  bgCachedGenericData scriptMenus[0x40];
  bgCachedGenericData materials[0x200];
  bgCachedGenericData localizedStrings[0x800];
  bgCachedGenericData debugStrings[0x200];
  bgCachedGenericData eventStrings[0x100];
  bgCachedGenericData triggerStrings[0xFA];
  bgCachedGenericData objectiveStrings[0x100];
  bgCachedGenericData luiMenus[0x40];
  bgCachedGenericData luiMenuData[0x80];
  bgCachedGenericData subXCamNames[0x1F];
};
ASSERT_OFFSET(bgCacheDataSet, localizedStrings, 0xE2B30);
ASSERT_SIZE(bgCacheDataSet, 0x45FDF8);

struct bgCache {
  bgCacheDataSet dataSet;
  bgCacheChecksum checksum;
};
ASSERT_SIZE(bgCache, 0x45FE00); // Correct

typedef BGCacheInstancePool<bgCache> bgCachePool;

struct bgCacheChecksumInfo {
  volatile uint32_t serverChecksum;
  volatile uint32_t clientChecksum;
};

} // namespace cache
} // namespace bg
} // namespace game