#pragma once

#include <game/symbols/sym_include.hpp>

// Lower level symbols for encoding, decoding, or playback, operating either on
// platform-native APIs or abstractions of them.
namespace game {
namespace voice {
WEAK symbol<void()> Voice_Playback{0x1422BB330, 0x0, 0x140598D60};
WEAK symbol<bool()> Voice_SendVoiceData{0x1413E3BB0, 0x0, 0x0};
WEAK symbol<void()> Voice_Shutdown{0x1422BB3C6, 0x0, 0x140598DD0};
WEAK symbol<int32_t(ControllerIndex_t controllerIndex,
                    lobby::session::LobbySession *lobbySession, XUID xuid,
                    uint8_t *data, int32_t size)>
    Voice_IncomingVoiceData_Platform{0x1422BB0A0, 0x0, 0x140598B20};

WEAK symbol<char()> Voice_Init{0x1422BB0FE, 0x0, 0x140598B50};
WEAK symbol<bool()> Voice_IsChatRestricted{0x1422BB310, 0x0, 0x0};
} // namespace voice
} // namespace game
