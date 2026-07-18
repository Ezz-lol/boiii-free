#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace net {
namespace msg {

// MSG
WEAK symbol<uint16_t(msg_t *msg, void *buffer, uint32_t expectedSize)>
    MSG_ReadDataFromUnsafeBuffer16{0x0, 0x14050D2E0};
WEAK symbol<char *(msg_t *msg, char *string, uint32_t maxChars)>
    MSG_ReadStringLine{0x142157140};
WEAK symbol<char *(msg_t *msg)> MSG_ReadBigString{0x1421551C0};
WEAK symbol<uint8_t(msg_t *msg)> MSG_ReadByte{0x142155450, 0x14050D1B0};
WEAK symbol<int16_t(msg_t *msg)> MSG_ReadShort{0x142156F30};
WEAK symbol<int32_t(bool trainHuffman, const uint8_t *from,
                    int32_t fromSizeBytes, uint8_t *to, int32_t toSizeBytes)>
    MSG_WriteBitsCompress{0x14215FC60};
WEAK symbol<int32_t(const uint8_t *from, int32_t fromSizeBytes, uint8_t *to,
                    int32_t toSizeBytes)>
    MSG_ReadBitsCompress{0x142155360};
WEAK symbol<int32_t(msg_t *msg)> MSG_ReadLong{0x142156910};
WEAK symbol<qboolean(lobby::LobbyMsg *lobbyMsg, const char *expectedKey,
                     uint8_t *val)>
    MSG_ReadString{0x1422E9940};
WEAK symbol<uint64_t(msg_t *msg)> MSG_ReadInt64{0x142156680, 0x14050E3E0};
WEAK symbol<uint32_t()> MSG_GetBitfieldChecksum{0x14217DEB0};
WEAK symbol<void(const char *modeName)> MSG_InitBitfields{0x14217E0A0};
WEAK
    symbol<void(const char *pszCommand, char **pszBuffer, char *allocBufferBase,
                int32_t allocBufferLength, int32_t *allocBufferPos)>
        MSG_WriteReliableCommandToBuffer{0x1421823E0};
WEAK symbol<void(msg_t *buf, uint8_t *data, int32_t length)> MSG_Init{
    0x142154F80};
WEAK symbol<void(msg_t *buf, uint8_t *data, int32_t length)> MSG_InitReadOnly{
    0x142155010};
WEAK symbol<void(msg_t *buf, const void *data, int32_t length)> MSG_WriteData{
    0x1421577E0};
WEAK symbol<void(msg_t *msg, int8_t c)> MSG_WriteByte{0x1421577C0};
WEAK symbol<void(msg_t *msg, int16_t c)> MSG_WriteShort{0x1421726D0};
WEAK symbol<void(msg_t *msg, int32_t c)> MSG_WriteLong{0x142172680};
WEAK symbol<void(msg_t *msg)> MSG_BeginReading{0x142154D00};
WEAK symbol<void(msg_t *msg)> MSG_ClearLastReferencedEntity{0x14215D140};
WEAK symbol<void(msg_t *msg)> MSG_WriteBit0{0x14215FBC0};
WEAK symbol<void(msg_t *msg)> MSG_WriteBit1{0x14215FC00};
WEAK symbol<void(msg_t *msg, uint64_t c)> MSG_WriteInt64{0x142157F50};
WEAK symbol<void(msg_t *msg, void *data, int32_t len)> MSG_ReadData{
    0x1421554B0};
WEAK symbol<void(msg_t *sb, const char *s)> MSG_WriteString{0x142158220};
WEAK symbol<void(level::playerState_s *ps, user::usercmd_t *cmd)>
    MSG_SetDefaultUserCmd{0x142157430};

} // namespace msg
} // namespace net
} // namespace game