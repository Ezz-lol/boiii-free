#pragma once

#include <cstdint>
#include "../io.hpp"

namespace game {
namespace db {
namespace xpak {
struct XPakLoadedPackFileRef {
  uint64_t size; // offset 0

  int32_t refCount;     // offset 9
  io::stream_fileid fh; // offset 13
};
} // namespace xpak
} // namespace db
} // namespace game