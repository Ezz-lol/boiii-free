#include <std_include.hpp>

#include "voicechat.hpp"

namespace steam {
namespace voice {
void VoiceChat::clearCapture() {
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

void VoiceChat::clearPlayback() {
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

int32_t VoiceChat::inputCallback(
    [[maybe_unused]] const void *input, void *output, unsigned long frameCount,
    [[maybe_unused]] const PaStreamCallbackTimeInfo *timeInfo,
    [[maybe_unused]] PaStreamCallbackFlags statusFlags, void *userData) {
  VoiceChat *self = static_cast<VoiceChat *>(userData);
  if (input && self && self->state.recording.load(std::memory_order_acquire)) {
    std::vector<uint8_t> encodedBuffer(ENCODING_BUFFER_SIZE);
    const int16_t *pcmInput = static_cast<const int16_t *>(input);

    int32_t bytesOrError = opus_encode(
        self->encoder, pcmInput, static_cast<int>(frameCount),
        encodedBuffer.data(), static_cast<opus_int32>(encodedBuffer.size()));

    if (bytesOrError > 0) {
      encodedBuffer.resize(static_cast<size_t>(bytesOrError));
      std::scoped_lock<std::recursive_mutex> lock(self->inputMutex);
      self->encodingQueue.push(std::move(encodedBuffer));
    }
  }

  return paContinue;
}

int32_t VoiceChat::outputCallback(
    [[maybe_unused]] const void *input, void *output, unsigned long frameCount,
    [[maybe_unused]] const PaStreamCallbackTimeInfo *timeInfo,
    [[maybe_unused]] PaStreamCallbackFlags statusFlags, void *userData) {
  VoiceChat *self = static_cast<VoiceChat *>(userData);
  if (self && output) {
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
      int32_t decodedFrameCount =
          opus_decode(decoder, packet.encoded.data(),
                      static_cast<opus_int32>(packet.encoded.size()),
                      decodedSamples.data(), MAX_FRAME_SIZE, 0);

      if (decodedFrameCount < 0) {
        break;
      }

      uint32_t framesToCopy = static_cast<uint32_t>(decodedFrameCount);
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

VoiceChat::~VoiceChat() {
  clearCapture();
  clearPlayback();
  Shutdown();
}

bool VoiceChat::Init() {
  if (state.initialized.load(std::memory_order_acquire)) {
    PaError err = Pa_Initialize();
    if (err == paNoError) {
      state.initialized.store(true, std::memory_order_release);
    }
    return true;
  }

  return false;
}

void VoiceChat::Shutdown() {
  if (state.initialized.exchange(false)) {
    Pa_Terminate();
  }
}

bool VoiceChat::StartVoiceRecording() {
  if (state.recording.load(std::memory_order_acquire)) {
    return true;
  }

  if (state.initialized.load(std::memory_order_acquire)) {
    int32_t err = OPUS_OK;
    encoder = opus_encoder_create(SAMPLE_RATE, RECORDING_CHANNEL_COUNT,
                                  OPUS_APPLICATION_VOIP, &err);

    if (encoder && err == OPUS_OK) {
      PaStreamParameters inputParams{};
      inputParams.device = Pa_GetDefaultInputDevice();
      if (inputParams.device == paNoDevice) {
        clearCapture();
      } else {
        const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(inputParams.device);
        inputParams.channelCount = RECORDING_CHANNEL_COUNT;
        inputParams.sampleFormat = paInt16;
        inputParams.suggestedLatency =
            devInfo ? devInfo->defaultLowInputLatency : 0.0;
        inputParams.hostApiSpecificStreamInfo = nullptr;

        PaError paErr =
            Pa_OpenStream(&inputStream, &inputParams, nullptr, SAMPLE_RATE,
                          FRAME_SIZE, paClipOff, inputCallback, this);

        if (paErr == paNoError) {
          paErr = Pa_StartStream(inputStream);
          if (paErr == paNoError) {
            state.recording.store(true, std::memory_order_release);
            return true;
          } else {
            clearCapture();
          }
        } else {
          clearCapture();
        }
      }
    } else {
      clearCapture();
    }
  }

  return false;
}

void VoiceChat::StopVoiceRecording() {
  if (state.recording.exchange(false)) {
    clearCapture();
  }
}

bool VoiceChat::StartVoicePlayback() {
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

    const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(params.device);
    params.channelCount = PLAYBACK_CHANNEL_COUNT;
    params.sampleFormat = paInt16;
    params.suggestedLatency = devInfo ? devInfo->defaultLowOutputLatency : 0.0;
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

void VoiceChat::StopVoicePlayback() {
  if (state.playing.exchange(false)) {
    clearPlayback();
  }
}

EVoiceResult VoiceChat::GetAvailableVoice(uint32_t *pcbCompressed) {
  if (pcbCompressed) {
    *pcbCompressed = 0;
  }

  if (!state.initialized.load(std::memory_order_acquire)) {
    return k_EVoiceResultNotInitialized;
  }
  if (!state.recording.load(std::memory_order_acquire)) {
    return k_EVoiceResultNotRecording;
  }
  if (!pcbCompressed) {
    return k_EVoiceResultBufferTooSmall;
  }

  std::scoped_lock<std::recursive_mutex> lock(inputMutex);

  if (encodingQueue.empty()) {
    return k_EVoiceResultNoData;
  }

  uint32_t availableBytes = static_cast<uint32_t>(encodingQueue.front().size());
  *pcbCompressed = availableBytes;
  return k_EVoiceResultOK;
}

EVoiceResult VoiceChat::GetVoice(bool bWantCompressed, void *pDestBuffer,
                                 uint32_t cbDestBufferSize,
                                 uint32_t *nBytesWritten) {
  if (nBytesWritten) {
    *nBytesWritten = 0;
  }

  if (!state.initialized.load(std::memory_order_acquire)) {
    return k_EVoiceResultNotInitialized;
  }
  if (!state.recording.load(std::memory_order_acquire)) {
    return k_EVoiceResultNotRecording;
  }
  if (!pDestBuffer || !nBytesWritten) {
    return k_EVoiceResultBufferTooSmall;
  }

  std::scoped_lock<std::recursive_mutex> lock(inputMutex);

  if (encodingQueue.empty()) {
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
    result = DecompressVoice(
        encodedVoice.data(), static_cast<uint32_t>(encodedVoice.size()),
        pDestBuffer, cbDestBufferSize, &bytesWritten, SAMPLE_RATE);
  }

  *nBytesWritten = bytesWritten;

  if (result == k_EVoiceResultOK) {
    encodingQueue.pop();
  }

  return result;
}

EVoiceResult
VoiceChat::DecompressVoice(const void *pCompressed, uint32_t cbCompressed,
                           void *pDestBuffer, uint32_t cbDestBufferSize,
                           uint32_t *nBytesWritten,
                           [[maybe_unused]] uint32_t nDesiredSampleRate) {
  if (nBytesWritten) {
    *nBytesWritten = 0;
  }

  if (pCompressed && cbCompressed > 0) {
    int32_t err = OPUS_OK;
    OpusDecoder *decoder =
        opus_decoder_create(SAMPLE_RATE, RECORDING_CHANNEL_COUNT, &err);
    if (decoder && err == OPUS_OK) {
      std::vector<opus_int16> pcmSamples(MAX_DECODED_RECORDING_SIZE);
      int32_t decodedFrameCount =
          opus_decode(decoder, static_cast<const uint8_t *>(pCompressed),
                      static_cast<opus_int32>(cbCompressed), pcmSamples.data(),
                      MAX_FRAME_SIZE, 0);

      opus_decoder_destroy(decoder);

      if (decodedFrameCount >= 0) {
        uint32_t bytesRequired = static_cast<uint32_t>(
            decodedFrameCount * RECORDING_CHANNEL_COUNT * sizeof(opus_int16));

        if (nBytesWritten) {
          *nBytesWritten = bytesRequired;
        }

        if (!pDestBuffer || cbDestBufferSize < bytesRequired) {
          return k_EVoiceResultBufferTooSmall;
        }

        std::memcpy(pDestBuffer, pcmSamples.data(), bytesRequired);
        return k_EVoiceResultOK;
      }
    }
    return k_EVoiceResultDataCorrupted;
  }

  return k_EVoiceResultNoData;
}

void VoiceChat::QueueAudioPlayback(uint64_t userId, const uint8_t *data,
                                   size_t len) {
  if (data && len > 0) {
    std::scoped_lock<std::recursive_mutex> lock(playbackQueueMutex);
    playbackQueue.push({userId, std::vector<uint8_t>(data, data + len)});
  }
}

} // namespace voice
} // namespace steam
