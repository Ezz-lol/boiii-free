#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace dw {
WEAK symbol<bdRemoteTaskRef(bdRemoteTaskRef *taskRef,
                            ControllerIndex_t controllerIndex,
                            RecordEventType type, const char *fmt, ...)>
    dwRecordEventFormat{0x14143F5D0, 0x14143F5B0, 0x0};
WEAK symbol<taskCompleteResults(const game::net::netadr_t *netadr)>
    dwGetConnectionTaskStatus{0x14143CC10, 0x14143CBF0, 0x0};
WEAK symbol<bool(game::net::netadr_t *netadr, const uint8_t *commonAddr,
                 const game::net::bdSecurityID *secId)>
    dwCommonAddrToNetadr{0x14143C380, 0x0, 0x0};
WEAK symbol<int32_t(const game::net::netadr_t *netadr)> dwGetConnectionStatus{
    0x14143CAD0, 0x0, 0x0};

} // namespace dw
} // namespace game
