#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> dwconsideredconnectedtime{0x15118D7A8,
                                                              0x1482DEF38};
WEAK symbol<EngineDependentDvarMut> dw_defaultdtlsassociationtimeout{
    0x151250888, 0x148340220};
WEAK symbol<EngineDependentDvarMut> dwFileFetchTryIntervalBase{0x1515C0C38,
                                                               0x1483828C0};
WEAK symbol<EngineDependentDvarMut> dwFileFetchTryIntervalMax{0x1515C0C40,
                                                              0x1483828C8};
WEAK symbol<EngineDependentDvarMut> dwFileFetchTryMaxAttempts{0x1515C0C48,
                                                              0x1483828D0};
WEAK symbol<EngineDependentDvarMut> dwNetMaxWaitMs{0x0, 0x146EA06F0};
WEAK symbol<EngineDependentDvarMut> dw_popup{0x151270AA0, 0x148360280};
WEAK symbol<EngineDependentDvarMut> dw_sendBufSize{0x0, 0x146EA06E8};
WEAK symbol<EngineDependentDvarMut> dwStreamingSendSocketBuffSize{0x1517C23A0,
                                                                  0x1483F4CC0};
WEAK symbol<EngineDependentDvarMut> nodw{0x15118D8B0, 0x1482DEE30};

} // namespace game
