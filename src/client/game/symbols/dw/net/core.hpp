#pragma once

#include "game/symbols/sym_include.hpp"

namespace game {
namespace dw {
namespace net {
WEAK symbol<bdCommonAddrRef *(bdCommonAddrRef *retstruct)>
    _dwGetLocalCommonAddr{0x14144AE20, 0x14144AE00, 0x1401EAFB0};

inline bdCommonAddr *dwGetLocalCommonAddr() {
  bdCommonAddrRef ref = {.m_ptr = nullptr};
  _dwGetLocalCommonAddr(&ref);
  return ref.m_ptr;
}
} // namespace net
} // namespace dw
} // namespace game
