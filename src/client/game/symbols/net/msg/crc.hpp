#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace net {
namespace msg {
namespace crc {
WEAK symbol<uint32_t()> MsgCRC_NetFieldChecksum{0x142100390, 0x140510B40};
}
} // namespace msg
} // namespace net
} // namespace game
