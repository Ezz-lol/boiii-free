#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

#include <opus.h>
#include <portaudio.h>

#include <steam/steamclientpublic.h>

namespace steam {

namespace voice {
inline constexpr auto SAMPLE_RATE = 48000;
inline constexpr auto RECORDING_CHANNEL_COUNT = 1;
inline constexpr auto PLAYBACK_CHANNEL_COUNT = 2;

inline constexpr auto ENCODING_BUFFER_SIZE = 8192;
inline constexpr auto FRAME_SIZE = 960;
inline constexpr auto MAX_FRAME_SIZE = (FRAME_SIZE * 6);
inline constexpr auto MAX_DECODED_RECORDING_SIZE =
    (MAX_FRAME_SIZE * RECORDING_CHANNEL_COUNT);
inline constexpr auto MAX_DECODED_PLAYBACK_SIZE =
    (MAX_FRAME_SIZE * PLAYBACK_CHANNEL_COUNT);

struct VoicePacket {
  uint64_t userId{0};
  std::vector<uint8_t> encoded;
};

typedef std::unordered_map<uint64_t, OpusDecoder *> decoderMap_t;

class Voice {
private:
  struct {
    std::atomic<bool> initialized{false};
    std::atomic<bool> recording{false};
    std::atomic<bool> playing{false};
  } state;

  // Capture
  std::recursive_mutex inputMutex;
  PaStream *inputStream{nullptr};

  // Encoder
  std::queue<std::vector<uint8_t>> encodingQueue;
  OpusEncoder *encoder{nullptr};

  // Playback resources
  std::recursive_mutex playbackQueueMutex;
  std::queue<VoicePacket> playbackQueue;

  // Decoder
  std::recursive_mutex decoderMapMutex;
  decoderMap_t decoderMap;
  PaStream *outputStream{nullptr};

  // Internal teardown helpers
  void clearCapture();
  void clearPlayback();

  // Audio device stream callback handles
  static int32_t inputCallback(const void *input, void *output,
                               unsigned long frameCount,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData);

  static int32_t outputCallback(const void *input, void *output,
                                unsigned long frameCount,
                                const PaStreamCallbackTimeInfo *timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void *userData);

public:
  Voice() = default;
  ~Voice();

  // Lifecycle operations
  bool Init();
  void Shutdown();

  // Recording controls
  bool StartRecording();
  void StopRecording();

  // Playback controls
  bool StartPlayback();
  void StopPlayback();

  // Voice data retrieval & processing
  EVoiceResult GetAvailable(uint32_t *pcbCompressed);
  EVoiceResult GetVoice(bool bWantCompressed, void *pDestBuffer,
                        uint32_t cbDestBufferSize, uint32_t *nBytesWritten);
  EVoiceResult Decompress(const void *pCompressed, uint32_t cbCompressed,
                          void *pDestBuffer, uint32_t cbDestBufferSize,
                          uint32_t *nBytesWritten, uint32_t nDesiredSampleRate);

  // Audio stream ingest
  void QueueAudioPlayback(uint64_t userId, const uint8_t *data, size_t len);

  inline constexpr bool Initialized() const {
    return state.initialized.load(std::memory_order_acquire);
  }

  inline constexpr bool Recording() const {
    return state.recording.load(std::memory_order_acquire);
  }

  inline constexpr bool Playing() const {
    return state.playing.load(std::memory_order_acquire);
  }
};
} // namespace voice
} // namespace steam
