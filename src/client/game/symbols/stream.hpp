#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace stream {
WEAK symbol<int32_t(stream_fh *fh)> Stream_OpenInternal{0x14217BD34,
                                                        0x14051E4E0};
} // namespace stream
} // namespace game

#endif
