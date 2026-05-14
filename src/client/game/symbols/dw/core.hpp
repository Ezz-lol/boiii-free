#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace dw {
WEAK symbol<bdRemoteTaskRef(bdRemoteTaskRef *taskRef,
                            ControllerIndex_t controllerIndex,
                            RecordEventType type, const char *fmt, ...)>
    dwRecordEventFormat{0x14143F5B0};
WEAK symbol<taskCompleteResults(const net::netadr_t *netadr)>
    dwGetConnectionTaskStatus{0x14143CBF0};

} // namespace dw
} // namespace game

#endif