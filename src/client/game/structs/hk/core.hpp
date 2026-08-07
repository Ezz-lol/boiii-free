#pragma once

#include "game/structs/quake/vec.hpp"
#include <cstdint>
namespace game {
namespace hk {
template <typename T> class hkArrayBase {
public:
  T *m_data;
  int32_t m_size;
  int32_t m_capacityAndFlags;
};

template <typename T, typename Allocator = void>
class hkArray : public hkArrayBase<T> {};

typedef uint8_t hkUint8;
typedef short hkInt16;
typedef uint16_t hkUint16;
typedef uint32_t hkUint32;
typedef int32_t hkInt32;
typedef uint64_t hkUlong;

struct hkHalf {
  typedef hkInt16 Storage;
  hkHalf::Storage m_value;
};
class hkBaseObject {
public:
  int32_t (**_vptr$hkBaseObject)(void);
};

class hkReferencedObject : public hkBaseObject {
public:
  hkUint32 m_memSizeAndRefCount;
  uint8_t _padding0C[4];
};

template <typename T, typename Storage = hkUint8> struct hkFlags {
  Storage m_storage;
};

typedef vec4_t hkVector4f;
typedef vec4<double> hkQuadDouble64;

typedef hkVector4f hkVector4;
typedef hkQuadDouble64 hkVector4d;

template <typename T> class hkRefPtr {
public:
  T *m_pntr;
};

struct hkMatrix3f {
  hkVector4f m_col0;
  hkVector4f m_col1;
  hkVector4f m_col2;
};

class hkRotationf : hkMatrix3f {};

struct hkTransformf {
  hkRotationf m_rotation;
  hkVector4f m_translation;
};

typedef hkTransformf hkTransform;

struct hkContainerHeapAllocator {};

typedef float hkFloat32;
typedef hkFloat32 hkReal;

struct hkAabb {
  hkVector4 m_min;
  hkVector4 m_max;
};

} // namespace hk
} // namespace game