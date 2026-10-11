#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace cl {
namespace voice {

WEAK symbol<voiceCommunication_t> cl_voiceCommunication{
    0x14535BCB0, 0x1428B0140, 0x1453DACB0};
WEAK symbol<lobby::LobbyClientPool<int32_t>> s_clientTalkTime{0x157DF7380, 0x0,
                                                              0x0};

WEAK symbol<void(const LocalClientNum_t localClientNum)>
    CL_DispatchVoicePackets{0x1413E33B0, 0x0, 0x140199CF0};
WEAK symbol<char(LocalClientNum_t localClientNum, game::net::msg::msg_t *msg)>
    CL_HandleVoiceTypePacket{0x141359330, 0x0, 0x14018FE40};
WEAK symbol<int(int32_t bytes, char *enc_buffer)> Client_SendVoiceData{
    0x1413E37F0, 0x0, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, game::net::msg::msg_t *msg)>
    CL_VoiceConnectionTestPacket{0x1413E3630, 0x0, 0x140199F50};
WEAK
    symbol<void(const ControllerIndex_t controllerIndex, const char *eventName)>
        CL_VoiceFrame{0x1413E3660, 0x0, 0x14019A320};
WEAK symbol<void(LocalClientNum_t localClientNum, game::net::msg::msg_t *msg)>
    CL_VoicePacket{0x1413E3670, 0x0, 0x0};
WEAK symbol<void()> CL_VoiceTransmit{0x1413E3680, 0x0, 0x14019A200};
} // namespace voice
} // namespace cl
} // namespace game
