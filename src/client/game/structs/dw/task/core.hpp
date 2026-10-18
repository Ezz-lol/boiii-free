#pragma once

#include <game/structs/dw/core.hpp>
#include <game/structs/dw/hash.hpp>

namespace game {
namespace dw {
namespace task {

class bdTaskByteBuffer : public bdByteBuffer {
public:
  bdUByte8 m_serviceId;
  bdUByte8 m_taskId;
  bdUByte8 *m_taskData;
  bdUInt m_taskDataSize;
  bdUInt m_validHeaderSize;
  bdUInt m_paddingSize;
};

typedef bdReference<bdTaskByteBuffer> bdTaskByteBufferRef;

} // namespace task
} // namespace dw
} // namespace game
