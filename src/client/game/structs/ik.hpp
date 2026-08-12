#pragma once

#include <structs/array.hpp>
#include <cstdint>
namespace game {
namespace ik {

constexpr size_t IK_BUF_SIZE = 0xD87F0;
constexpr size_t IK_BUF_ALIGN = 0x10;
typedef aligned_array<uint8_t, IK_BUF_SIZE, IK_BUF_ALIGN> IkBuf;
typedef aligned_array_ref<uint8_t, IK_BUF_SIZE, IK_BUF_ALIGN> IkBufRef;

} // namespace ik
} // namespace game