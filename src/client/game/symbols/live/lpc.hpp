#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace lpc {
WEAK symbol<bool()> LPC_IsLPCReady{0x141F03E20};
}
} // namespace live
} // namespace game