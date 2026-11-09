#pragma once

#include <cstdint>

#include "game/structs/macros.hpp"

#include "hash.hpp"
namespace game {
namespace ui {
typedef uint16_t UIModelIndex;

struct CurrentWeaponModel {
  ui::UIModelIndex currentWeaponModel;
  ui::UIModelIndex equippedWeaponReferenceModel;
  ui::UIModelIndex currentWeaponReticleWidget;
  ui::UIModelIndex currentWeaponLockOnWidget;
  ui::UIModelIndex lookingDownSights;
  ui::UIModelIndex sightIsMagnified;
  ui::UIModelIndex lockedOnEnemy;
  ui::UIModelIndex lockedOnEnemyTeamTargetting;
  ui::UIModelIndex lockedOnEnemyTeamHacking;
  ui::UIModelIndex lockedOnEnemyTimeRemaining;
  ui::UIModelIndex lockedOnEnemyMissilesRemaining;
  ui::UIModelIndex currentShotCharge;
  ui::UIModelIndex weaponDelayFiring;
  ui::UIModelIndex weaponOverEnemy;
  ui::UIModelIndex usingOffhand;
  ui::UIModelIndex weaponPrestigeUI3DTextModel;
  ui::UIModelIndex updateWeaponSelectModel;
  ui::UIModelIndex weaponNameModel;
  ui::UIModelIndex viewmodelWeaponNameModel;
  ui::UIModelIndex attachmentNameModel[8];
  ui::UIModelIndex fireRateStringModel;
  ui::UIModelIndex weaponModel;
  ui::UIModelIndex clipMaxAmmoModel;
  ui::UIModelIndex ammoInClipModel;
  ui::UIModelIndex ammoInDWClipModel;
  ui::UIModelIndex ammoStockModel;
  ui::UIModelIndex ammoClipPercentageModel;
  ui::UIModelIndex ammoLowModel;
  ui::UIModelIndex ammoDWLowModel;
  ui::UIModelIndex totalAmmoLowModel;
  ui::UIModelIndex damageFeedbackModel;
  ui::UIModelIndex damageFeedbackFlagModel;
  ui::UIModelIndex spikeLauncherCounterReadyCountModel;
  ui::UIModelIndex spikeLauncherCounterBlastingModel;
  ui::UIModelIndex lmgInfiniteStateModel;
};
struct VehicleModel {
  ui::UIModelIndex vehicleTypeModel;
  ui::UIModelIndex isVehicleOwnerModel;
  ui::UIModelIndex outOfRangeModel;
  ui::UIModelIndex zoomedModel;
  ui::UIModelIndex ammoModel;
  ui::UIModelIndex ammoReadyModel;
  ui::UIModelIndex speedModel;
  ui::UIModelIndex remainingTimeModel;
  ui::UIModelIndex damageModel;
  ui::UIModelIndex engineRpmModel;
  ui::UIModelIndex weaponRevModel;
  ui::UIModelIndex pitchModel;
  ui::UIModelIndex yawModel;
  ui::UIModelIndex rollModel;
  ui::UIModelIndex altitudeModel;
  ui::UIModelIndex partDestroyedModel[2];
  ui::UIModelIndex missileLockedOnModel[5];
  ui::UIModelIndex fullscreenFilterModel;
  ui::UIModelIndex weaponHeatModel;
  ui::UIModelIndex weaponOverheatingModel;
};
struct KillstreakRewardModel {
  ui::UIModelIndex nameModel;
  ui::UIModelIndex imageModel;
  ui::UIModelIndex slotModel;
  ui::UIModelIndex momentumCostModel;
  ui::UIModelIndex ammoModel;
};
struct PlayerGadgetModel {
  ui::UIModelIndex idModel;
  ui::UIModelIndex ammoModel;
  ui::UIModelIndex powerRatioModel;
  ui::UIModelIndex stateModel;
};
struct PlayerRoleModel {
  ui::UIModelIndex clientTeam;
  ui::UIModelIndex clientCharacter;
  ui::UIModelIndex clientHealth;
  ui::UIModelIndex clientArmor;
  ui::UIModelIndex clientArmorMax;
  ui::UIModelIndex clientArmorTier;
  ui::UIModelIndex clientArmorPct;
  ui::UIModelIndex clientLpLevel;
  ui::UIModelIndex clientPlayerName;
  ui::UIModelIndex clientClanTag;
};

struct PerkModel {
  ui::UIModelIndex nameModel;
  ui::UIModelIndex imageModel;
};

enum class UIModelDataType : uint32_t {
  INVALID = 0x0,
  BOOL = 0x1,
  UINT64 = 0x2,
  INT = 0x3,
  REAL = 0x4,
  STRING = 0x5,
  FUNCTION = 0x6,
  COUNT = 0x7,
};

inline constexpr const char *serialize(const UIModelDataType type) {
  switch (type) {
  case UIModelDataType::INVALID:
    return "UI_MODEL_INVALID";
  case UIModelDataType::BOOL:
    return "UI_MODEL_BOOL";
  case UIModelDataType::UINT64:
    return "UI_MODEL_UINT64";
  case UIModelDataType::INT:
    return "UI_MODEL_INT";
  case UIModelDataType::REAL:
    return "UI_MODEL_REAL";
  case UIModelDataType::STRING:
    return "UI_MODEL_STRING";
  case UIModelDataType::FUNCTION:
    return "UI_MODEL_FUNCTION";
  case UIModelDataType::COUNT:
    return "UI_MODEL_COUNT";
  default:
    return "UI_MODEL_DATA_TYPE_INVALID";
  }
}

struct UIModelData {
  UIModelDataType dataType;
  union {
    uint64_t uint64;
    const char *string;
    int32_t integer;
    float real;
    bool boolean;
    int functionRef;
  };
  [[nodiscard]] inline std::string serialize() const noexcept {
    std::string val_str;
    switch (dataType) {
    case UIModelDataType::BOOL:
      val_str = std::format("\"boolean\": {}", ::serialize(boolean));
      break;
    case UIModelDataType::UINT64:
      val_str = std::format("\"uint64\": {}", uint64);
      break;
    case UIModelDataType::INT:
      val_str = std::format("\"integer\": {}", integer);
      break;
    case UIModelDataType::REAL:
      val_str = std::format("\"real\": {}", real);
      break;
    case UIModelDataType::STRING:
      val_str = std::format("\"string\": {}", ::serialize(string));
      break;
    case UIModelDataType::FUNCTION:
      val_str = std::format("\"functionRef\": {}", functionRef);
      break;
    default:
      val_str = R"("value": "INVALID")";
      break;
    }
    return std::format("{{ \"dataType\": {}, {} }}", ui::serialize(dataType),
                       val_str);
  }

  [[nodiscard]] inline std::string serialize() const volatile noexcept {
    return const_cast<const UIModelData *>(this)->serialize();
  }
};

typedef uint16_t UIModelSubscription;

// Verified
PACKED(struct UIModelNode {
  djb2Hash_t keyHash;
  uint8_t _padding04[4];
  UIModelData data;
  UIModelIndex ancestor;
  UIModelIndex firstChild;
  UIModelIndex index;
  UIModelIndex nextSibling;
  UIModelSubscription firstSubscription;
  bool persistent;
  uint8_t _padding23[5];

  inline constexpr void setKey(const std::string_view &key) noexcept {
    keyHash = djb2(key);
  }

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"keyHash\": {:08X}, \"data\": {}, \"ancestor\": {}, "
        "\"firstChild\": {}, \"index\": {}, \"nextSibling\": {}, "
        "\"firstSubscription\": {}, \"persistent\": {} }}",
        keyHash, data.serialize(), ancestor, firstChild, index, nextSibling,
        firstSubscription, ::serialize(persistent));
  }

  [[nodiscard]] inline std::string serialize() const volatile noexcept {
    return const_cast<const UIModelNode *>(this)->serialize();
  }

  [[nodiscard]] inline constexpr bool isFree() const volatile noexcept {
    return keyHash == 0 && nextSibling == 0 && ancestor == 0 && firstChild == 0;
  }

  [[nodiscard]] inline constexpr bool isFree() const noexcept {
    return const_cast<const volatile UIModelNode *>(this)->isFree();
  }
});
ASSERT_SIZE(UIModelNode, 0x28);

template <typename T> struct UIModelNodePool {
  typedef UIModelIndex index_t;

  static inline constexpr auto SIZE = 0x9000;
  volatile T nodes[SIZE];

  inline static constexpr auto size() noexcept { return SIZE; }

  inline constexpr T *begin() noexcept { return nodes; }
  inline constexpr const T *begin() const noexcept { return nodes; }
  inline constexpr volatile T *begin() volatile noexcept { return nodes; }
  inline constexpr const T *cbegin() const noexcept { return nodes; }
  inline constexpr volatile T *cbegin() volatile noexcept { return nodes; }

  inline constexpr T *end() noexcept { return nodes + SIZE; }
  inline constexpr const T *end() const noexcept { return nodes + SIZE; }
  inline constexpr volatile T *end() volatile noexcept { return nodes + SIZE; }
  inline constexpr const T *cend() const noexcept { return nodes + SIZE; }
  inline constexpr volatile T *cend() volatile noexcept { return nodes + SIZE; }

  template <IntegralLike<index_t> Index>
  static inline constexpr bool valid_index(Index index_arg) noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    return index < SIZE;
  }

  template <IntegralLike<index_t> Index>
  static inline constexpr void
  assert_index([[maybe_unused]] Index index) noexcept {
    assert(valid_index(index) &&
           "Index to UIModelNodePool must be within range 0 <= index < 0x9000");
  }

  template <IntegralLike<index_t> Index>
  inline constexpr volatile T &get(Index index) noexcept {
    assert_index(index);
    return nodes[static_cast<index_t>(index)];
  }
  template <IntegralLike<index_t> Index>
  inline constexpr const volatile T &get(Index index) const noexcept {
    assert_index(index);
    return nodes[static_cast<index_t>(index)];
  }
  template <IntegralLike<index_t> Index>
  inline constexpr volatile T &operator[](Index index) noexcept {
    return get(index);
  }
  template <IntegralLike<index_t> Index>
  inline constexpr const volatile T &operator[](Index index) const noexcept {
    return get(index);
  }

  inline bool contains(uintptr_t entry) volatile noexcept {
    const uintptr_t this_ptr = reinterpret_cast<uintptr_t>(this);
    return entry >= this_ptr &&
           entry < (this_ptr + sizeof(std::remove_pointer_t<decltype(this)>));
  }

  template <typename E> inline bool contains(E *entry) volatile noexcept {
    return contains(reinterpret_cast<uintptr_t>(entry));
  }

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"nodes\": {} }}", ::serialize(nodes));
  }

  [[nodiscard]] inline std::string serialize() const volatile noexcept {
    return const_cast<const UIModelNodePool<T> *>(this)->serialize();
  }
};

} // namespace ui
} // namespace game
