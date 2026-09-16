#ifndef STRUCTS_DB_CORE_HPP
#define STRUCTS_DB_CORE_HPP

#include "../macros.hpp"
#include <cstdint>

namespace game {
namespace db {
typedef int DBFile;

struct XBlock {
  uint8_t *data;
  uint64_t size;
};
ASSERT_SIZE(XBlock, 0x10);

} // namespace db
} // namespace game
#endif