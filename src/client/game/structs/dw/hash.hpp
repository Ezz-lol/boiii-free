#pragma once

#include <game/structs/core.hpp>
#include <game/structs/macros.hpp>

namespace game {
namespace dw {
namespace hash {
// Intentionally empty
struct bdHashingClass {};
ASSERT_SIZE(bdHashingClass, 0x1);

class bdHash {
public:
  uint8_t __vftable[4];
};

class bdHashMD5 : hash::bdHash {};

template <typename... Args> class bdHashMap;

// Primary implementation, with template parameters matching those used by the
// engine
template <typename K, typename V, typename H> class bdHashMap<K, V, H> {
public:
  class Node {
  public:
    V m_data;
    K m_key;
    Node *m_next;
  };

  bdUInt m_size;
  bdUInt m_capacity;
  bdFloat32 m_loadFactor;
  bdUInt m_threshold;
  Node **m_map;
  H m_hashClass;
  bdUInt m_numIterators;
};

// Key and Value template variant that uses default hasher. Provided for
// convenience here, but does not exist in engine
template <typename K, typename V>
class bdHashMap<K, V> : public bdHashMap<K, V, bdHashingClass> {
public:
  using bdHashMap<K, V, bdHashingClass>::bdHashMap; // Inherit constructors
};

} // namespace hash
} // namespace dw
} // namespace game
