#pragma once

#include <cstdint>

#include <game/structs/macros.hpp>

namespace game {
namespace db {
typedef int DBFile;

struct XBlock {
  uint8_t *data;
  uint64_t size;

  IMPL_SLICE_METHODS(data, size, Data);
};
ASSERT_SIZE(XBlock, 0x10);

} // namespace db
} // namespace game
