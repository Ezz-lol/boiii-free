#include <std_include.hpp>

#include "voice.hpp"

#include <game/utils.hpp>

namespace steam {
namespace voice {
namespace {
std::string selected_input_device() {
  return std::string(game::get_dvar_string("voice_input_device").value_or(""));
}

std::vector<PaDeviceIndex> input_device_indices() {
  PaHostApiIndex host_api = Pa_HostApiTypeIdToHostApiIndex(paDirectSound);
  if (host_api < 0) {
    host_api = Pa_GetDefaultHostApi();
  }
  const PaHostApiInfo *host_info = Pa_GetHostApiInfo(host_api);
  std::vector<PaDeviceIndex> indices;
  for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); ++i) {
    const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
    if (info && info->hostApi == host_api && info->maxInputChannels > 0 &&
        (!host_info || i != host_info->defaultInputDevice)) {
      indices.push_back(i);
    }
  }
  return indices;
}

PaDeviceIndex find_input_device(const std::string &name) {
  for (const PaDeviceIndex i : input_device_indices()) {
    if (name == Pa_GetDeviceInfo(i)->name) {
      return i;
    }
  }
  return Pa_GetDefaultInputDevice();
}
} // namespace

std::vector<std::string> InputDevices() {
  std::vector<std::string> devices;
  if (Pa_Initialize() != paNoError) {
    return devices;
  }
  for (const PaDeviceIndex i : input_device_indices()) {
    devices.emplace_back(Pa_GetDeviceInfo(i)->name);
  }
  Pa_Terminate();
  return devices;
}

bool Voice::InputDeviceChanged() const {
  return selected_input_device() != inputDevice;
}

void Voice::clearCapture() {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][clearCapture] entered");
  }
#endif
  if (inputStream) {
    Pa_AbortStream(inputStream);
    Pa_CloseStream(inputStream);
    inputStream = nullptr;
  }

  if (encoder) {
    opus_encoder_destroy(encoder);
    encoder = nullptr;
  }

  {
    std::scoped_lock<std::recursive_mutex> lock(inputMutex);
    std::queue<std::vector<uint8_t>> emptyQueue;
    std::swap(encodingQueue, emptyQueue);
  }

  state.recording.store(false, std::memory_order_release);
}

void Voice::clearPlayback() {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][clearPlayback] entered");
  }
#endif
  if (outputStream) {
    Pa_AbortStream(outputStream);
    Pa_CloseStream(outputStream);
    outputStream = nullptr;
  }

  {
    std::scoped_lock<std::recursive_mutex> lock(decoderMapMutex);
    for (auto &[id, decoder] : decoderMap) {
      if (decoder) {
        opus_decoder_destroy(decoder);
      }
    }
    decoderMap.clear();
  }

  {
    std::scoped_lock<std::recursive_mutex> lock(playbackQueueMutex);
    std::queue<VoicePacket> emptyQueue;
    std::swap(playbackQueue, emptyQueue);
  }

  state.playing.store(false, std::memory_order_release);
}

int32_t
Voice::inputCallback(const void *input, void *output, unsigned long frameCount,
                     [[maybe_unused]] const PaStreamCallbackTimeInfo *timeInfo,
                     [[maybe_unused]] PaStreamCallbackFlags statusFlags,
                     void *userData) {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][inputCallback] entered");
  }
#endif
  Voice *self = static_cast<Voice *>(userData);
  if (input && self && self->state.recording.load(std::memory_order_acquire)) {
    std::vector<uint8_t> encodedBuffer(ENCODING_BUFFER_SIZE);
    const int16_t *pcmInput = static_cast<const int16_t *>(input);

    int32_t len =
        opus_encode(self->encoder, pcmInput, static_cast<int32_t>(frameCount),
                    encodedBuffer.data(), encodedBuffer.size());

#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace(
          "[Steam][Voice][inputCallback] opus_encode returned result {}", len);
    }
#endif
    if (len > 0) {
      encodedBuffer.resize(static_cast<size_t>(len));
      {
        std::scoped_lock<std::recursive_mutex> lock(self->inputMutex);
        while (self->encodingQueue.size() >= MAX_QUEUED_FRAMES) {
          self->encodingQueue.pop();
        }
        self->encodingQueue.emplace(std::move(encodedBuffer));
      }
    }
  }

  return paContinue;
}

int32_t Voice::outputCallback(
    [[maybe_unused]] const void *input, void *output, unsigned long frameCount,
    [[maybe_unused]] const PaStreamCallbackTimeInfo *timeInfo,
    [[maybe_unused]] PaStreamCallbackFlags statusFlags, void *userData) {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][outputCallback] entered");
  }
#endif
  Voice *self = static_cast<Voice *>(userData);
  if (self && output) {
    memset(output, 0, frameCount * PLAYBACK_CHANNEL_COUNT * sizeof(int16_t));

    int16_t *pcmOutput = static_cast<int16_t *>(output);
    uint32_t framesRemaining = frameCount;

    while (framesRemaining > 0) {
      VoicePacket packet;
      {
        std::scoped_lock<std::recursive_mutex> lock(self->playbackQueueMutex);
        if (self->playbackQueue.empty()) {
          break;
        }
        packet = std::move(self->playbackQueue.front());
        self->playbackQueue.pop();
      }

      OpusDecoder *decoder = nullptr;
      {
        std::scoped_lock<std::recursive_mutex> lock(self->decoderMapMutex);
        decoderMap_t::iterator it = self->decoderMap.find(packet.userId);
        if (it != self->decoderMap.end()) {
          decoder = it->second;
        } else {
          int32_t error = OPUS_OK;
          decoder =
              opus_decoder_create(SAMPLE_RATE, PLAYBACK_CHANNEL_COUNT, &error);
          if (error == OPUS_OK && decoder) {
            self->decoderMap[packet.userId] = decoder;
          }
        }
      }

      std::vector<opus_int16> decodedSamples(MAX_DECODED_PLAYBACK_SIZE);
      int32_t decFrameCount =
          opus_decode(decoder, packet.encoded.data(),
                      static_cast<opus_int32>(packet.encoded.size()),
                      decodedSamples.data(), MAX_FRAME_SIZE, 0);

      if (decFrameCount < 0) {
        break;
      }

      uint32_t framesToCopy = static_cast<uint32_t>(decFrameCount);
      if (framesToCopy > framesRemaining) {
        framesToCopy = framesRemaining;
      }

      uint32_t bytesToCopy = static_cast<uint32_t>(
          framesToCopy * PLAYBACK_CHANNEL_COUNT * sizeof(opus_int16));
      std::memcpy(pcmOutput, decodedSamples.data(), bytesToCopy);

      framesRemaining -= framesToCopy;
      pcmOutput += framesToCopy * PLAYBACK_CHANNEL_COUNT;
    }
  }
  return paContinue;
}

Voice::~Voice() {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("Voice destructor entered");
  }
#endif
  clearCapture();
  clearPlayback();
  Shutdown();
}

bool Voice::Init() {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][Init] entered");
  }
#endif
  if (state.initialized.load(std::memory_order_acquire)) {
    return true;
  }

  const bool result = Pa_Initialize() == paNoError;
  if (result) {
    state.initialized.store(true, std::memory_order_release);
  } else {
#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace("Pa_Initialize call failed");
    }
#endif
  }

  return result;
}

void Voice::Shutdown() {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][Shutdown] entered");
  }
#endif
  if (state.initialized.exchange(false)) {
    Pa_Terminate();
  }
}

bool Voice::StartRecording() {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][StartRecording] entered");
  }
#endif
  if (state.recording.load(std::memory_order_acquire)) {
    return true;
  }

  if (state.initialized.load(std::memory_order_acquire)) {
    int32_t err = OPUS_OK;
    encoder = opus_encoder_create(SAMPLE_RATE, RECORDING_CHANNEL_COUNT,
                                  OPUS_APPLICATION_VOIP, &err);

    if (encoder && err == OPUS_OK) {
      PaStreamParameters inputParams{};
      inputDevice = selected_input_device();
      inputParams.device = find_input_device(inputDevice);
      if (inputParams.device == paNoDevice) {
#ifndef NDEBUG
        if (game::voice_trace()) {
          game::trace("[Steam][Voice][StartRecording] Pa_GetDefaultInputDevice "
                      "returned device paNoDevice. Pa_GetDeviceCount returned "
                      "{} devices.",
                      Pa_GetDeviceCount());
        }
#endif
        clearCapture();
      } else {
        const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(inputParams.device);
        inputParams.channelCount = RECORDING_CHANNEL_COUNT;
        inputParams.sampleFormat = paInt16;
        inputParams.suggestedLatency =
            deviceInfo ? deviceInfo->defaultLowInputLatency : 0.0;
        inputParams.hostApiSpecificStreamInfo = nullptr;

        if (Pa_OpenStream(&inputStream, &inputParams, nullptr, SAMPLE_RATE,
                          FRAME_SIZE, paClipOff, inputCallback,
                          this) == paNoError &&
            Pa_StartStream(inputStream) == paNoError) {
          state.recording.store(true, std::memory_order_release);
          return true;
        } else {
#ifndef NDEBUG
          if (game::voice_trace()) {
            game::trace("[Steam][Voice][StartRecording] Pa_StartStream failed");
          }
#endif
          clearCapture();
        }
      }
    } else {
#ifndef NDEBUG
      if (game::voice_trace()) {
        game::trace(
            "[Steam][Voice][StartRecording] opus_encoder_create failed");
      }
#endif
      clearCapture();
    }
  }

  return false;
}

void Voice::StopRecording() {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][StopRecording] entered");
  }
#endif
  if (state.recording.exchange(false)) {
    clearCapture();
  }
}

bool Voice::StartPlayback() {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][StartPlayback] entered");
  }
#endif
  if (!state.playing.load(std::memory_order_acquire)) {
    if (!state.initialized.load(std::memory_order_acquire)) {
      return false;
    }

    PaStreamParameters params{};
    params.device = Pa_GetDefaultOutputDevice();
    if (params.device == paNoDevice) {
      clearPlayback();
      return false;
    }

    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(params.device);
    params.channelCount = PLAYBACK_CHANNEL_COUNT;
    params.sampleFormat = paInt16;
    params.suggestedLatency =
        deviceInfo ? deviceInfo->defaultLowOutputLatency : 0.0;
    params.hostApiSpecificStreamInfo = nullptr;

    if (Pa_OpenStream(&outputStream, nullptr, &params, SAMPLE_RATE, FRAME_SIZE,
                      paClipOff, outputCallback, this) != paNoError ||
        Pa_StartStream(outputStream) != paNoError) {
      clearPlayback();
      return false;
    }

    state.playing.store(true, std::memory_order_release);
  }
  return true;
}

void Voice::StopPlayback() {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][StopPlayback] entered");
  }
#endif
  if (state.playing.exchange(false)) {
    clearPlayback();
  }
}

EVoiceResult Voice::GetAvailable(uint32_t *pcbCompressed) {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][GetAvailableVoice] entered");
  }
#endif
  if (pcbCompressed) {
    *pcbCompressed = 0;
  }

  if (!state.initialized.load(std::memory_order_acquire)) {
#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace("[Steam][Voice][GetAvailable] returning "
                  "k_EVoiceResultNotInitialized");
    }
#endif
    return k_EVoiceResultNotInitialized;
  }
  if (!state.recording.load(std::memory_order_acquire)) {
#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace(
          "[Steam][Voice][GetAvailable] returning k_EVoiceResultNotRecording");
    }
#endif
    return k_EVoiceResultNotRecording;
  }
  if (!pcbCompressed) {
#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace("[Steam][Voice][GetAvailable] returning "
                  "k_EVoiceResultBufferTooSmall");
    }
#endif
    return k_EVoiceResultBufferTooSmall;
  }

  std::scoped_lock<std::recursive_mutex> lock(inputMutex);

  if (encodingQueue.empty()) {
#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace(
          "[Steam][Voice][GetAvailable] returning k_EVoiceResultNoData");
    }
#endif
    return k_EVoiceResultNoData;
  }

  uint32_t availableBytes = static_cast<uint32_t>(encodingQueue.front().size());
  *pcbCompressed = availableBytes;
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][GetAvailable] returning k_EVoiceResultOK");
  }
#endif
  return k_EVoiceResultOK;
}

EVoiceResult Voice::GetVoice(bool bWantCompressed, void *pDestBuffer,
                             uint32_t cbDestBufferSize,
                             uint32_t *nBytesWritten) {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace(
        "[Steam][Voice][GetVoice] called with bWantCompressed: {}, "
        "pDestBuffer: {:p}, cbDestBufferSize: {}, nBytesWritten: {:X}@{:p}",
        bWantCompressed ? "true" : "false", pDestBuffer, cbDestBufferSize,
        nBytesWritten ? *nBytesWritten : 0, static_cast<void *>(nBytesWritten));
  }
#endif
  if (nBytesWritten) {
    *nBytesWritten = 0;
  }

  if (!state.initialized.load(std::memory_order_acquire)) {
#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace(
          "[Steam][Voice][GetVoice] returning k_EVoiceResultNotInitialized");
    }
#endif
    return k_EVoiceResultNotInitialized;
  }
  if (!state.recording.load(std::memory_order_acquire)) {
#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace(
          "[Steam][Voice][GetVoice] returning k_EVoiceResultNotRecording");
    }
#endif
    return k_EVoiceResultNotRecording;
  }
  if (!pDestBuffer || !nBytesWritten) {
#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace(
          "[Steam][Voice][GetVoice] returning k_EVoiceResultBufferTooSmall");
    }
#endif
    return k_EVoiceResultBufferTooSmall;
  }

  std::scoped_lock<std::recursive_mutex> lock(inputMutex);

  if (encodingQueue.empty()) {
#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace("[Steam][Voice][GetVoice] returning k_EVoiceResultNoData");
    }
#endif
    return k_EVoiceResultNoData;
  }

  const std::vector<uint8_t> &encodedVoice = encodingQueue.front();
  EVoiceResult result = k_EVoiceResultOK;
  uint32_t bytesWritten = 0;

  if (bWantCompressed) {
    if (cbDestBufferSize < encodedVoice.size()) {
      result = k_EVoiceResultBufferTooSmall;
    } else {
      std::memcpy(pDestBuffer, encodedVoice.data(), encodedVoice.size());
      bytesWritten = static_cast<uint32_t>(encodedVoice.size());
    }
  } else {
    result = Decompress(encodedVoice.data(),
                        static_cast<uint32_t>(encodedVoice.size()), pDestBuffer,
                        cbDestBufferSize, &bytesWritten, 0, SAMPLE_RATE);
  }

  *nBytesWritten = bytesWritten;

  if (result == k_EVoiceResultOK) {
    encodingQueue.pop();
  }

#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][GetVoice] returning with result: {}. Arg "
                "values: bWantCompressed: {}, pDestBuffer: {:p}, "
                "cbDestBufferSize: {}, nBytesWritten: {:X}@{:p}",
                static_cast<int32_t>(result),
                bWantCompressed ? "true" : "false", pDestBuffer,
                cbDestBufferSize, nBytesWritten ? *nBytesWritten : 0,
                static_cast<void *>(nBytesWritten));
  }
#endif
  return result;
}

EVoiceResult Voice::Decompress(const void *pCompressed, uint32_t cbCompressed,
                               void *pDestBuffer, uint32_t cbDestBufferSize,
                               uint32_t *nBytesWritten, uint64_t stream,
                               uint32_t nDesiredSampleRate) {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace(
        "[Steam][Voice][DecompressVoice] called with pCompressed: {:p}, "
        "cbCompressed: {}, pDestBuffer: {:p}, cbDestBufferSize: {}, "
        "nBytesWritten: {}@{:p}, nDesiredSampleRate: {}",
        pCompressed, cbCompressed, pDestBuffer, cbDestBufferSize,
        nBytesWritten ? *nBytesWritten : 0, static_cast<void *>(nBytesWritten));
  }
#endif
  if (nBytesWritten) {
    *nBytesWritten = 0;
  }

  if (pCompressed && cbCompressed) {
    const uint32_t outputRate =
        nDesiredSampleRate ? nDesiredSampleRate : SAMPLE_RATE;
    static constexpr std::array<uint32_t, 5> decodeRates{8000, 12000, 16000,
                                                         24000, SAMPLE_RATE};
    const uint32_t decodeRate =
        *std::ranges::find_if(decodeRates, [outputRate](const uint32_t rate) {
          return rate >= outputRate || rate == SAMPLE_RATE;
        });
    std::scoped_lock<std::recursive_mutex> lock(decoderMapMutex);
    int32_t err = OPUS_OK;
    OpusDecoder *&decoder = decoderMap[stream];
    if (!decoder) {
      decoder = opus_decoder_create(static_cast<opus_int32>(decodeRate),
                                    RECORDING_CHANNEL_COUNT, &err);
    }
    if (decoder && err == OPUS_OK) {
      std::vector<opus_int16> pcmSamples(MAX_DECODED_RECORDING_SIZE);
      int32_t decFrameCount =
          opus_decode(decoder, static_cast<const uint8_t *>(pCompressed),
                      static_cast<opus_int32>(cbCompressed), pcmSamples.data(),
                      MAX_FRAME_SIZE, 0);

      if (decFrameCount >= 0) {
        const uint32_t outputCount = static_cast<uint32_t>(
            static_cast<uint64_t>(decFrameCount) * outputRate / decodeRate);
        const uint32_t bytesRequired = static_cast<uint32_t>(
            outputCount * RECORDING_CHANNEL_COUNT * sizeof(opus_int16));

        if (nBytesWritten) {
          *nBytesWritten = bytesRequired;
        }

        if (!pDestBuffer || cbDestBufferSize < bytesRequired) {
#ifndef NDEBUG
          if (game::voice_trace()) {
            game::trace("[Steam][Voice][DecompressVoice] returning "
                        "k_EVoiceResultBufferTooSmall");
          }
#endif
          return k_EVoiceResultBufferTooSmall;
        }

        opus_int16 *output = static_cast<opus_int16 *>(pDestBuffer);
        for (uint32_t i = 0; i < outputCount; ++i) {
          const double position =
              static_cast<double>(i) * decodeRate / outputRate;
          const uint32_t index = static_cast<uint32_t>(position);
          const int32_t current = pcmSamples[index];
          const int32_t next = index + 1 < static_cast<uint32_t>(decFrameCount)
                                   ? pcmSamples[index + 1]
                                   : current;
          output[i] = static_cast<opus_int16>(current + (next - current) *
                                                            (position - index));
        }
#ifndef NDEBUG
        if (game::voice_trace()) {
          game::trace("[Steam][Voice][DecompressVoice] returning "
                      "k_EVoiceResultOK");
        }
#endif
        return k_EVoiceResultOK;
      }
    }
#ifndef NDEBUG
    if (game::voice_trace()) {
      game::trace("[Steam][Voice][DecompressVoice] returning "
                  "k_EVoiceResultDataCorrupted");
    }
#endif
    return k_EVoiceResultDataCorrupted;
  }

#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][DecompressVoice] returning "
                "k_EVoiceResultNoData");
  }
#endif
  return k_EVoiceResultNoData;
}

void Voice::QueueAudioPlayback(uint64_t userId, const uint8_t *data,
                               size_t len) {
#ifndef NDEBUG
  if (game::voice_trace()) {
    game::trace("[Steam][Voice][QueueAudioPlayback] entered");
  }
#endif
  if (data && len > 0) {
    std::scoped_lock<std::recursive_mutex> lock(playbackQueueMutex);
    playbackQueue.push({userId, std::vector<uint8_t>(data, data + len)});
  }
}

} // namespace
  // voice
} // namespace
  // steam
