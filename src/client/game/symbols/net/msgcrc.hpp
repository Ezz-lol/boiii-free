#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace net {
namespace msgcrc {
WEAK symbol<uint32_t()> MsgCRC_NetFieldChecksum{0x142158E50};

} // namespace msgcrc
} // namespace net
} // namespace game

#endif