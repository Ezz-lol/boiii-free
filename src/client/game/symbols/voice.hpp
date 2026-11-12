#pragma once

#include <game/symbols/sym_include.hpp>

// Lower level symbols for encoding, decoding, or playback, operating either on
// platform-native APIs or abstractions of them.
namespace game {
namespace voice {
WEAK symbol<void()> Voice_Playback{0x1422BB330, 0x140598D60};
WEAK symbol<bool()> Voice_SendVoiceData{0x1413E3BB0, 0x0};
WEAK symbol<void()> Voice_Shutdown{0x1422BB3B0, 0x140598DD0};
WEAK symbol<int32_t(ControllerIndex_t controllerIndex,
                    lobby::session::LobbySession *lobbySession, XUID xuid,
                    uint8_t *data, int32_t size)>
    Voice_IncomingVoiceData_Platform{0x1422BB0A0, 0x140598B20};

WEAK symbol<char()> Voice_Init{0x1422BB0FE, 0x140598B50};
WEAK symbol<bool()> Voice_IsChatRestricted{0x1422BB310, 0x0};

WEAK symbol<lobby::LobbyClientPool<dsound_sample_t *>> s_clientSamples{
    0x157DF72E0, 0x14A641380};

WEAK symbol<bool> recording{0x157DF7150, 0x14A6411F0};
WEAK symbol<bool> playing{0x157DF7151, 0x14A6411F1};
WEAK symbol<dsound_sample_t *> currentRecordingSample{0x157DF7158, 0x14A6411F8};
WEAK symbol<bool> g_recording_initialized{0x150B33DE0, 0x148203840};
WEAK symbol<bool> g_currently_recording{0x150B33DE1, 0x148203841};
WEAK symbol<dsound_sample_t *> g_current_sample{0x150B33DE8, 0x148203848};
WEAK symbol<qboolean> g_voice_initialised{0x157DF7290, 0x14A641330};
WEAK symbol<qboolean> g_voice_record_initialized{0x157DF7294, 0x14A641334};
WEAK symbol<bool> dsoundplay_initialized{0x150B328A0, 0x148202300};
WEAK symbol<LPDIRECTSOUND8> lpds{0x150B328A8, 0x148202308};
inline IDirectSound8 &get_lpds() { return **lpds; }
} // namespace voice
} // namespace game
