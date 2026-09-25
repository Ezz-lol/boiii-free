#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace net {
namespace msgcrc {
WEAK symbol<uint32_t()> MsgCRC_NetFieldChecksum{0x142100390, 0x142158E50, 0x0};

} // namespace msgcrc
} // namespace net
} // namespace game
