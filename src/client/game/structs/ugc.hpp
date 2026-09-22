#pragma once

#include "core.hpp"
#include "steam.hpp"
#include "ui/core.hpp"
#include <cstdint>

namespace game {
namespace ugc {

typedef uint32_t UGCHash;
typedef str<260> UGCPath;

struct WorkshopData {
  // Field is called "name" in lua table
  str<100> title;
  name_t internalName;
  // Field is called "ugcName" in lua table
  str32_t publisherId;
  str256_t description;
  UGCPath contentPathToZoneFiles;
  UGCPath absolutePathContentDirectory;
  UGCPath absolutePathZoneFiles;
  /*
    Also used in engine to check if mod is installed
    by checking if version == 1.
    Unused logically in engine otherwise, but name of field is "ugcVersion" in
    corresponding lua table, so this must be the UGC version.
  */
  uint32_t version;
  uint8_t _padding4B4[4];
  steam::PublishedFileId_t publisherIdInteger;
  UGCHash publisherIdHash;
  ZoneType type;

  inline void clear() {
    memset(static_cast<void *>(this), 0, sizeof(WorkshopData));
  }

#ifndef NDEBUG
  inline std::string serialize() {
    return std::format(
        "WorkshopData {{ \"title\": \"{}\", \"internalName\": \"{}\", "
        "\"publisherId\": \"{}\", \"description\": \"{}\", "
        "\"contentPathToZoneFiles\": \"{}\", \"absolutePathContentDirectory\": "
        "\"{}\", \"absolutePathZoneFiles\", \"{}\", \"version\": {}, "
        "\"publisherIdInteger\": {}, \"publisherIdHash\": {}, \"type\": {} }}",
        title, internalName, publisherId, description, contentPathToZoneFiles,
        absolutePathContentDirectory, absolutePathZoneFiles, version,
        publisherIdInteger, publisherIdHash, ::game::serialize(type));
  }
#endif
};
ASSERT_CPP03_POD(WorkshopData);
ASSERT_SIZE(WorkshopData, 0x4C8);

enum class ModLoadState : uint32_t {
  IDLE = 0x0,
  LOADING = 0x1,
  COMPLETE = 0x2,
};
IMPL_ENUM_OPERATORS(ModLoadState);

struct ActiveMod : WorkshopData {
  ModLoadState loadState;
  uint8_t _padding4CC[4];
};
ASSERT_SIZE(ActiveMod, 0x4D0);

typedef std::optional<std::reference_wrapper<WorkshopData>>
    OptionalWorkshopDataRef;

typedef std::optional<std::reference_wrapper<const WorkshopData>>
    OptionalConstWorkshopDataRef;

template <const uint32_t POOL_SIZE> struct WorkshopDataPool {
  uint32_t count;
  uint8_t _padding04[4];
  WorkshopData data[POOL_SIZE];

  inline void clear() {
    memset(static_cast<void *>(this), 0, sizeof(WorkshopDataPool<POOL_SIZE>));
  }

  struct Iterator {
    using iterator_concept = std::contiguous_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = WorkshopData;
    using pointer = WorkshopData *;
    using reference = WorkshopData &;

    Iterator() = default;

    Iterator(pointer ptr) : m_ptr(ptr) {}

    reference operator*() const { return *m_ptr; }
    pointer operator->() const { return m_ptr; }

    Iterator &operator++() {
      m_ptr++;
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const Iterator &a, const Iterator &b) = default;

  private:
    pointer m_ptr = nullptr;
  };

  struct ConstIterator {
    using iterator_concept = std::contiguous_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = WorkshopData;
    using pointer = WorkshopData *;
    using reference = const WorkshopData &;

    ConstIterator() = default;

    ConstIterator(pointer ptr) : m_ptr(ptr) {}

    reference operator*() const { return *m_ptr; }
    pointer operator->() const { return m_ptr; }

    ConstIterator &operator++() {
      m_ptr++;
      return *this;
    }
    ConstIterator operator++(int) {
      ConstIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const ConstIterator &a,
                           const ConstIterator &b) = default;

  private:
    pointer m_ptr = nullptr;
  };

  struct Iterable {
    Iterator m_begin;
    Iterator m_end;

    Iterator begin() { return m_begin; }
    Iterator end() { return m_end; }
  };

  struct ConstIterable {
    ConstIterator m_begin;
    ConstIterator m_end;

    ConstIterator begin() { return m_begin; }
    ConstIterator end() { return m_end; }
  };

  Iterable iter() noexcept {
    return Iterable{Iterator(data), Iterator(&data[count])};
  }

  ConstIterable iter() const noexcept {
    return ConstIterable{ConstIterator(data), ConstIterator(&data[count])};
  }

  template <typename Predicate>
  OptionalConstWorkshopDataRef find(Predicate predicate) const noexcept {
    ConstIterable it = iter();
    const std::ranges::borrowed_iterator_t<ConstIterable &> result =
        std::ranges::find_if(it, predicate);
    return result == it.end() ? std::nullopt
                              : OptionalConstWorkshopDataRef(*result);
  }

  template <typename Predicate>
  OptionalWorkshopDataRef find(Predicate predicate) noexcept {
    Iterable it = iter();
    const std::ranges::borrowed_iterator_t<Iterable &> result =
        std::ranges::find_if(it, predicate);
    return result == it.end() ? std::nullopt : OptionalWorkshopDataRef(*result);
  }
};

template <const uint32_t POOL_SIZE>
struct ClientWorkshopDataPool : WorkshopDataPool<POOL_SIZE> {
  LocalClientPool<ui::UIModelIndex> listModels;
  uint8_t _padding[4];
};

constexpr uint32_t BUILTIN_WORKSHOP_DATA_POOL_SIZE = 128;
typedef WorkshopDataPool<BUILTIN_WORKSHOP_DATA_POOL_SIZE>
    BuiltinWorkshopDataPool;
ASSERT_SIZE(BuiltinWorkshopDataPool, 0x26408);
typedef ClientWorkshopDataPool<BUILTIN_WORKSHOP_DATA_POOL_SIZE>
    BuiltinClientWorkshopDataPool;
ASSERT_SIZE(BuiltinClientWorkshopDataPool, 0x26410);

constexpr uint32_t EXTENDED_WORKSHOP_DATA_POOL_SIZE = 8192;
typedef ClientWorkshopDataPool<EXTENDED_WORKSHOP_DATA_POOL_SIZE>
    ExtendedWorkshopDataPool;
constexpr uint32_t EXTENDED_WORKSHOP_DATA_POOL_STRUCT_SIZE =
    sizeof(ExtendedWorkshopDataPool);

typedef WorkshopData *SteamUGCRequestDetailsResult_t;

} // namespace ugc
} // namespace game
