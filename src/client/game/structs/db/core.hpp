#ifndef STRUCTS_DB_CORE_HPP
#define STRUCTS_DB_CORE_HPP

#include <cstdint>

namespace game {
namespace db {
typedef int DBFile;

// sizeof=0x10
struct XBlock {
  uint8_t *data;
  uint64_t size;
};
static_assert(sizeof(XBlock) == 0x10, "XBlock size must be 16 bytes");

} // namespace db
} // namespace game
#endif