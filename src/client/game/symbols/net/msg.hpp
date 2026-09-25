#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace net {
namespace msg {

// MSG
WEAK symbol<uint16_t(msg_t *msg, void *buffer, uint32_t expectedSize)>
    MSG_ReadDataFromUnsafeBuffer16{0x0, 0x0, 0x14050D2E0};
WEAK symbol<char *(msg_t *msg, char *string, uint32_t maxChars)>
    MSG_ReadStringLine{0x1420FE680, 0x142157140, 0x0};
WEAK symbol<char *(msg_t *msg)> MSG_ReadBigString{0x1420FC700, 0x1421551C0,
                                                  0x0};
WEAK symbol<uint8_t(msg_t *msg)> MSG_ReadByte{0x1420FC990, 0x142155450,
                                              0x14050D1B0};
WEAK symbol<int16_t(msg_t *msg)> MSG_ReadShort{0x1420FE470, 0x142156F30, 0x0};
WEAK symbol<int32_t(bool trainHuffman, const uint8_t *from,
                    int32_t fromSizeBytes, uint8_t *to, int32_t toSizeBytes)>
    MSG_WriteBitsCompress{0x1421071A0, 0x14215FC60, 0x0};
WEAK symbol<int32_t(const uint8_t *from, int32_t fromSizeBytes, uint8_t *to,
                    int32_t toSizeBytes)>
    MSG_ReadBitsCompress{0x1420FC8A0, 0x142155360, 0x0};
WEAK symbol<int32_t(msg_t *msg)> MSG_ReadLong{0x1420FDE50, 0x142156910, 0x0};
WEAK symbol<uint64_t(msg_t *msg)> MSG_ReadInt64{0x1420FDBC0, 0x142156680,
                                                0x14050E3E0};
WEAK symbol<uint32_t()> MSG_GetBitfieldChecksum{0x142125410, 0x14217DEB0, 0x0};
WEAK symbol<void(const char *modeName)> MSG_InitBitfields{0x142125600,
                                                          0x14217E0A0, 0x0};
WEAK
    symbol<void(const char *pszCommand, char **pszBuffer, char *allocBufferBase,
                int32_t allocBufferLength, int32_t *allocBufferPos)>
        MSG_WriteReliableCommandToBuffer{0x142129940, 0x1421823E0, 0x0};
WEAK symbol<void(msg_t *buf, uint8_t *data, int32_t length)> MSG_Init{
    0x1420FC4C0, 0x142154F80, 0x0};
WEAK symbol<void(msg_t *buf, uint8_t *data, int32_t length)> MSG_InitReadOnly{
    0x1420FC550, 0x142155010, 0x0};
WEAK symbol<void(msg_t *buf, const void *data, int32_t length)> MSG_WriteData{
    0x1420FED20, 0x1421577E0, 0x0};
WEAK symbol<void(msg_t *msg, int8_t c)> MSG_WriteByte{0x1420FED00, 0x1421577C0,
                                                      0x0};
WEAK symbol<void(msg_t *msg, int16_t c)> MSG_WriteShort{0x142119C10,
                                                        0x1421726D0, 0x0};
WEAK symbol<void(msg_t *msg, int32_t c)> MSG_WriteLong{0x142119BC0, 0x142172680,
                                                       0x0};
WEAK symbol<void(msg_t *msg)> MSG_BeginReading{0x1420FC240, 0x142154D00, 0x0};
WEAK symbol<void(msg_t *msg)> MSG_ClearLastReferencedEntity{0x142104680,
                                                            0x14215D140, 0x0};
WEAK symbol<void(msg_t *msg)> MSG_WriteBit0{0x142107100, 0x14215FBC0, 0x0};
WEAK symbol<void(msg_t *msg)> MSG_WriteBit1{0x142107140, 0x14215FC00, 0x0};
WEAK symbol<void(msg_t *msg, uint64_t c)> MSG_WriteInt64{0x1420FF490,
                                                         0x142157F50, 0x0};
WEAK symbol<void(msg_t *msg, void *data, int32_t len)> MSG_ReadData{
    0x1420FC9F0, 0x1421554B0, 0x0};
WEAK symbol<void(msg_t *sb, const char *s)> MSG_WriteString{0x1420FF760,
                                                            0x142158220, 0x0};
WEAK symbol<void(level::playerState_s *ps, user::usercmd_t *cmd)>
    MSG_SetDefaultUserCmd{0x1420FE970, 0x142157430, 0x0};

} // namespace msg
} // namespace net
} // namespace game
