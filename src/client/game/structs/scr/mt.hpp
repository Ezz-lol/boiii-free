#pragma once

#include <cstdint>
#include <game/structs/macros.hpp>
#include <macros.hpp>

namespace game {
namespace scr {
// MT == MemoryTree
namespace mt {

typedef uint32_t ScrMTID_t;

// Verified
PACKED(struct MemoryNode {
  uint32_t fence1[1];
  ScrMTID_t prev;
  ScrMTID_t next;
  uint32_t pad3[3];
  uint32_t fence2[1];
});
ASSERT_PACKED(MemoryNode);
ASSERT_SIZE(MemoryNode, 0x1C);
ASSERT_POD(MemoryNode);

constexpr size_t GSCR_MEMORY_TREE_SERVER_NODE_POOL_LEN = 0x20000;
// Verified
struct GScrMemoryTreeServerNodePool {
  MemoryNode nodes[GSCR_MEMORY_TREE_SERVER_NODE_POOL_LEN];

  template <IntegralLike Index>
  inline constexpr MemoryNode &get(Index index) noexcept {
    return nodes[static_cast<size_t>(index)];
  }
  template <IntegralLike Index>
  inline constexpr const MemoryNode &get(Index index) const noexcept {
    return nodes[static_cast<size_t>(index)];
  }
  template <IntegralLike Index>
  inline constexpr MemoryNode &operator[](Index index) noexcept {
    return get(index);
  }
  template <IntegralLike Index>
  inline constexpr const MemoryNode &operator[](Index index) const noexcept {
    return get(index);
  }

  template <IntegralLike Index>
  static inline constexpr bool valid_index(Index index_arg) noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    return index >= 0 && index < GSCR_MEMORY_TREE_SERVER_NODE_POOL_LEN;
  }

  inline bool contains(uintptr_t entry) const noexcept {
    const uintptr_t this_ptr = reinterpret_cast<uintptr_t>(this);
    return entry >= this_ptr &&
           entry < (this_ptr + sizeof(std::remove_pointer_t<decltype(this)>));
  }
  template <typename T> inline bool contains(T *entry) const noexcept {
    return contains(reinterpret_cast<uintptr_t>(entry));
  }
};
ASSERT_PACKED(GScrMemoryTreeServerNodePool);
ASSERT_SIZE(GScrMemoryTreeServerNodePool, 0x380000);
ASSERT_POD(GScrMemoryTreeServerNodePool);

constexpr size_t GSCR_MEMORY_TREE_SERVER_HEAD_POOL_LEN = 18;
struct GScrMemoryTreeServerHeadPool {
  ScrMTID_t head[GSCR_MEMORY_TREE_SERVER_HEAD_POOL_LEN];

  template <IntegralLike Index>
  inline constexpr ScrMTID_t &get(Index index) noexcept {
    return head[static_cast<size_t>(index)];
  }
  template <IntegralLike Index>
  inline constexpr const ScrMTID_t &get(Index index) const noexcept {
    return head[static_cast<size_t>(index)];
  }
  template <IntegralLike Index>
  inline constexpr ScrMTID_t &operator[](Index index) noexcept {
    return get(index);
  }
  template <IntegralLike Index>
  inline constexpr const ScrMTID_t &operator[](Index index) const noexcept {
    return get(index);
  }

  template <IntegralLike Index>
  static inline constexpr bool valid_index(Index index_arg) noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    return index >= 0 && index < GSCR_MEMORY_TREE_SERVER_HEAD_POOL_LEN;
  }

  inline bool contains(uintptr_t entry) const noexcept {
    const uintptr_t this_ptr = reinterpret_cast<uintptr_t>(this);
    return entry >= this_ptr &&
           entry < (this_ptr + sizeof(std::remove_pointer_t<decltype(this)>));
  }
  template <typename T> inline bool contains(T *entry) const noexcept {
    return contains(reinterpret_cast<uintptr_t>(entry));
  }
};

// Verified
struct scrMemTreePub_t {
  union {
    // This is always a pointer to the
    // `GScrMemoryTreeServerNodePool gScrMemTreeServerNodes` static allocation,
    // but the engine defines this field type as `char *`, so both are provided.
    char *buf;
    GScrMemoryTreeServerNodePool *mt_buffer;
  };
};
ASSERT_SIZE(scrMemTreePub_t, 0x8);

// Verified
PACKED(struct scrMemTreeGlob_t {
  // Field is defined as `MemoryNode *nodes` in engine
  GScrMemoryTreeServerNodePool *nodePool;
  uint8_t _padding08[0x78];
  uint8_t leftBits[256];
  uint8_t numBits[256];
  uint8_t logBits[256];
  // field is defined as `ScrMTID_t *head;` in engine
  GScrMemoryTreeServerHeadPool *headPool;
  int32_t totalAlloc;
  int32_t totalAllocBuckets;
  int32_t avgAlloc;
  int32_t avgAllocBytes;
  uint8_t _padding398[0x68];
});
ASSERT_PACKED(scrMemTreeGlob_t);
ASSERT_POD(scrMemTreeGlob_t);
ASSERT_SIZE(scrMemTreeGlob_t, 0x400);

} // namespace mt
} // namespace scr
} // namespace game