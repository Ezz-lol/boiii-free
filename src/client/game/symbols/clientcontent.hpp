#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace clientcontent {
WEAK symbol<ClientContentType(const uint32_t dataMask)>
    ClientContentType_GetContentTypeFromDatamask{0x141EDED50, 0x14049C9B0};
WEAK symbol<uint32_t(ClientContentType contentType,
                     ClientContent *clientContent)>
    ClientContent_GetChecksum{0x141EDEFD0, 0x14049CC30};
WEAK symbol<void(ClientContentType contentType, ClientContent *clientContent,
                 const uint8_t *data, uint32_t dataSize)>
    ClientContent_SetData{0x141EDF150, 0x14049CD30};
WEAK symbol<void(ClientContentType contentType, ClientContent *clientContent,
                 uint16_t newSize)>
    ClientContent_SetCompressedDataSize{0x141EDF130, 0x14049CD20};
WEAK symbol<void(ClientContentType contentType, ClientContent *clientContent,
                 uint32_t newChecksum)>
    ClientContent_SetChecksum{0x141EDF120, 0x14049CD10};
WEAK symbol<void(ClientContentType contentType, ClientContent *clientContent)>
    ClientContent_ClearData{0x141EDEE80, 0x14049CAE0};
WEAK symbol<void(ClientContentType contentType, ClientContent *clientContent,
                 bool isCompressed)>
    ClientContent_SetDataCompressed{0x141EDF1C0, 0x14049CDA0};

} // namespace clientcontent
} // namespace game
