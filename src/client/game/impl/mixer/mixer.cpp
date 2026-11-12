#include <std_include.hpp>

#include "mixer.hpp"

namespace game {
namespace mixer {

using namespace game;
using namespace game::wave;

inline constexpr auto MIXER_WAVE_MUTE_CONTROL_ID = 0x20010002;
inline constexpr auto MIXER_WAVE_RECORD_CONTROL_ID = 0x50030001;

void Mixer_SetWaveInMuteStates_Impl(int32_t newMuteState) {
  for (WaveInLineInfo &line : s_waveInLineInfo) {
    if (line.isValid) {
      HMIXER hmx = nullptr;
      if (mixerOpen(&hmx, line.mixerID, 0, 0, 0) == MMSYSERR_NOERROR) {
        MIXERLINEA mxl{};
        mxl.cbStruct = sizeof(MIXERLINEA);
        mxl.dwLineID = line.dstWaveInLineID;

        if (mixerGetLineInfoA(reinterpret_cast<HMIXEROBJ>(hmx), &mxl,
                              MIXER_GETLINEINFOF_LINEID) == MMSYSERR_NOERROR) {
          MIXERCONTROLW mxctrl{};
          MIXERLINECONTROLSA pmxlc{};
          pmxlc.cbStruct = sizeof(MIXERLINECONTROLSA);
          pmxlc.dwLineID = mxl.dwLineID;
          pmxlc.dwControlID = MIXER_WAVE_MUTE_CONTROL_ID;
          pmxlc.cControls = 1;
          pmxlc.cbmxctrl = sizeof(MIXERCONTROLW);
          pmxlc.pamxctrl = reinterpret_cast<LPMIXERCONTROLA>(&mxctrl);

          if (mixerGetLineControlsA(reinterpret_cast<HMIXEROBJ>(hmx), &pmxlc,
                                    MIXER_GETLINECONTROLSF_ONEBYTYPE) ==
              MMSYSERR_NOERROR) {
            MIXERCONTROLDETAILS pmxcd{};
            pmxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
            pmxcd.dwControlID = mxctrl.dwControlID;
            pmxcd.cChannels = 1;
            pmxcd.cbDetails = sizeof(LONG);

            LONG detailsValue = 0;
            pmxcd.paDetails = &detailsValue;

            if (mixerGetControlDetailsA(reinterpret_cast<HMIXEROBJ>(hmx),
                                        &pmxcd, 0) == MMSYSERR_NOERROR) {
              detailsValue = newMuteState;
              mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(hmx), &pmxcd,
                                     0);
            }
          }
        }
        mixerClose(hmx);
      }
    }
  }
}

void Mixer_RestoreWaveInRecordLevels_Impl() {
  for (WaveInLineInfo &line : s_waveInLineInfo) {
    if (line.isValid) {
      HMIXER hmx = nullptr;
      if (mixerOpen(&hmx, line.mixerID, 0, 0, 0) == MMSYSERR_NOERROR) {
        MIXERLINEA mxl{};
        mxl.cbStruct = sizeof(MIXERLINEA);
        mxl.dwLineID = line.dstWaveInLineID;

        if (mixerGetLineInfoA(reinterpret_cast<HMIXEROBJ>(hmx), &mxl,
                              MIXER_GETLINEINFOF_LINEID) == MMSYSERR_NOERROR) {
          MIXERCONTROLW mxctrl{};
          MIXERLINECONTROLSA pmxlc{};
          pmxlc.cbStruct = sizeof(MIXERLINECONTROLSA);
          pmxlc.dwLineID = mxl.dwLineID;
          pmxlc.dwControlID = MIXER_WAVE_RECORD_CONTROL_ID;
          pmxlc.cControls = 1;
          pmxlc.cbmxctrl = sizeof(MIXERCONTROLW);
          pmxlc.pamxctrl = reinterpret_cast<LPMIXERCONTROLA>(&mxctrl);

          if (mixerGetLineControlsA(reinterpret_cast<HMIXEROBJ>(hmx), &pmxlc,
                                    MIXER_GETLINECONTROLSF_ONEBYTYPE) ==
              MMSYSERR_NOERROR) {
            MIXERCONTROLDETAILS pmxcd{};
            pmxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
            pmxcd.dwControlID = mxctrl.dwControlID;
            pmxcd.cChannels = 1;
            pmxcd.cbDetails = sizeof(LONG);

            LONG detailsValue = line.oldRecLevel;
            pmxcd.paDetails = &detailsValue;

            mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(hmx), &pmxcd, 0);
          }
        }
        mixerClose(hmx);
      }
    }
  }
}

int32_t Mixer_GetRecordLevel_Impl(UINT mixerID, DWORD waveInLineID) {
  HMIXER hmx = nullptr;
  if (mixerOpen(&hmx, mixerID, 0, 0, 0) == MMSYSERR_NOERROR) {
    MIXERLINEA mxl{};
    mxl.cbStruct = sizeof(MIXERLINEA);
    mxl.dwLineID = waveInLineID;

    if (mixerGetLineInfoA(reinterpret_cast<HMIXEROBJ>(hmx), &mxl,
                          MIXER_GETLINEINFOF_LINEID) == MMSYSERR_NOERROR) {
      MIXERCONTROLW mxctrl{};
      MIXERLINECONTROLSA pmxlc{};
      pmxlc.cbStruct = sizeof(MIXERLINECONTROLSA);
      pmxlc.dwLineID = mxl.dwLineID;
      pmxlc.dwControlID = MIXER_WAVE_RECORD_CONTROL_ID;
      pmxlc.cControls = 1;
      pmxlc.cbmxctrl = sizeof(MIXERCONTROLW);
      pmxlc.pamxctrl = reinterpret_cast<LPMIXERCONTROLA>(&mxctrl);

      if (mixerGetLineControlsA(reinterpret_cast<HMIXEROBJ>(hmx), &pmxlc,
                                MIXER_GETLINECONTROLSF_ONEBYTYPE) ==
          MMSYSERR_NOERROR) {
        MIXERCONTROLDETAILS pmxcd{};
        pmxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
        pmxcd.dwControlID = mxctrl.dwControlID;
        pmxcd.cChannels = 1;
        pmxcd.cbDetails = sizeof(LONG);

        LONG levelValue = 0;
        pmxcd.paDetails = &levelValue;

        if (mixerGetControlDetailsA(reinterpret_cast<HMIXEROBJ>(hmx), &pmxcd,
                                    0) == MMSYSERR_NOERROR) {
          mixerClose(hmx);
          return static_cast<int32_t>(levelValue);
        }
      }
    }
    mixerClose(hmx);
  }
  return -1;
}

int32_t Mixer_GetMuteState_Impl(UINT mixerID, UINT waveInLineID) {
  HMIXER hmx = nullptr;
  if (mixerOpen(&hmx, mixerID, 0, 0, 0) == MMSYSERR_NOERROR) {
    MIXERLINEA mixerline{};
    mixerline.cbStruct = sizeof(MIXERLINEA);
    mixerline.dwLineID = waveInLineID;

    if (mixerGetLineInfoA(reinterpret_cast<HMIXEROBJ>(hmx), &mixerline,
                          MIXER_GETLINEINFOF_LINEID) == MMSYSERR_NOERROR) {
      MIXERCONTROLW mxc{};
      MIXERLINECONTROLSA mxlc{};
      mxlc.cbStruct = sizeof(MIXERLINECONTROLSA);
      mxlc.dwLineID = mixerline.dwLineID;
      mxlc.dwControlID = MIXER_WAVE_MUTE_CONTROL_ID;
      mxlc.cControls = 1;
      mxlc.cbmxctrl = sizeof(MIXERCONTROLW);
      mxlc.pamxctrl = reinterpret_cast<LPMIXERCONTROLA>(&mxc);

      if (mixerGetLineControlsA(reinterpret_cast<HMIXEROBJ>(hmx), &mxlc,
                                MIXER_GETLINECONTROLSF_ONEBYTYPE) ==
          MMSYSERR_NOERROR) {
        MIXERCONTROLDETAILS mxcd{};
        mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
        mxcd.dwControlID = mxc.dwControlID;
        mxcd.cChannels = 1;
        mxcd.cbDetails = sizeof(LONG);

        LONG newSetting = 0;
        mxcd.paDetails = &newSetting;

        if (mixerGetControlDetailsA(reinterpret_cast<HMIXEROBJ>(hmx), &mxcd,
                                    0) == MMSYSERR_NOERROR) {
          mixerClose(hmx);
          return static_cast<int>(newSetting);
        }
      }
    }
    mixerClose(hmx);
  }
  return 0;
}

MMRESULT Mixer_InitWaveInLineInfo_Impl() {
  uint32_t mixerDevCount = mixerGetNumDevs();
  uint32_t waveInDevCount = waveInGetNumDevs();

  if (mixerDevCount < 1 || waveInDevCount < 1) {
    return MMSYSERR_NODRIVER;
  }

  s_waveInLineInfo = {};

  size_t lineIndex = 0;
  for (uint32_t mixerID = 0;
       mixerID < std::min<uint32_t>(mixerDevCount, s_waveInLineInfo.size());
       ++mixerID) {
    MIXERCAPSA mxcaps{};
    if (mixerGetDevCapsA(mixerID, &mxcaps, sizeof(MIXERCAPSA)) ==
        MMSYSERR_NOERROR) {
      HMIXER hmx = nullptr;
      if (mixerOpen(&hmx, mixerID, 0, 0, 0) == MMSYSERR_NOERROR) {
        MIXERLINEA mxl{};
        mxl.cbStruct = sizeof(MIXERLINEA);
        mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

        if (mixerGetLineInfoA(reinterpret_cast<HMIXEROBJ>(hmx), &mxl,
                              MIXER_GETLINEINFOF_COMPONENTTYPE) ==
            MMSYSERR_NOERROR) {
          WaveInLineInfo &line = s_waveInLineInfo[lineIndex];
          lineIndex++;
          line.mixerID = mixerID;
          line.dstWaveInLineID = mxl.dwLineID;
          line.oldRecLevel = Mixer_GetRecordLevel_Impl(mixerID, mxl.dwLineID);
          line.oldMuteState = Mixer_GetMuteState_Impl(mixerID, mxl.dwLineID);
          line.isValid = qtrue;
        }
        mixerClose(hmx);
      }
    }
  }

  return MMSYSERR_NOERROR;
}

void Mixer_RestoreWaveInMuteStates_Impl() {
  for (WaveInLineInfo &line : s_waveInLineInfo) {
    if (line.isValid) {
      HMIXER hmx = nullptr;
      if (mixerOpen(&hmx, line.mixerID, 0, 0, 0) == MMSYSERR_NOERROR) {
        MIXERLINEA mxl{};
        mxl.cbStruct = sizeof(MIXERLINEA);
        mxl.dwLineID = line.dstWaveInLineID;

        if (mixerGetLineInfoA(reinterpret_cast<HMIXEROBJ>(hmx), &mxl,
                              MIXER_GETLINEINFOF_LINEID) == MMSYSERR_NOERROR) {
          MIXERCONTROLW mxctrl{};
          MIXERLINECONTROLSA pmxlc{};
          pmxlc.cbStruct = sizeof(MIXERLINECONTROLSA);
          pmxlc.dwLineID = mxl.dwLineID;
          pmxlc.dwControlID = MIXER_WAVE_MUTE_CONTROL_ID;
          pmxlc.cControls = 1;
          pmxlc.cbmxctrl = sizeof(MIXERCONTROLW);
          pmxlc.pamxctrl = reinterpret_cast<LPMIXERCONTROLA>(&mxctrl);

          if (mixerGetLineControlsA(reinterpret_cast<HMIXEROBJ>(hmx), &pmxlc,
                                    MIXER_GETLINECONTROLSF_ONEBYTYPE) ==
              MMSYSERR_NOERROR) {
            MIXERCONTROLDETAILS pmxcd{};
            pmxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
            pmxcd.dwControlID = mxctrl.dwControlID;
            pmxcd.cChannels = 1;
            pmxcd.cbDetails = sizeof(LONG);

            LONG detailsValue = line.oldMuteState;
            pmxcd.paDetails = &detailsValue;

            mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(hmx), &pmxcd, 0);
          }
        }
        mixerClose(hmx);
      }
    }
  }
}

void Mixer_SetWaveInRecordLevels_Impl(int32_t newLevel) {
  for (WaveInLineInfo &line : s_waveInLineInfo) {
    if (line.isValid) {
      HMIXER hmx = nullptr;
      if (mixerOpen(&hmx, line.mixerID, 0, 0, 0) == MMSYSERR_NOERROR) {
        MIXERLINEA mxl{};
        mxl.cbStruct = sizeof(MIXERLINEA);
        mxl.dwLineID = line.dstWaveInLineID;

        if (mixerGetLineInfoA(reinterpret_cast<HMIXEROBJ>(hmx), &mxl,
                              MIXER_GETLINEINFOF_LINEID) == MMSYSERR_NOERROR) {
          MIXERCONTROLW mxctrl{};
          MIXERLINECONTROLSA pmxlc{};
          pmxlc.cbStruct = sizeof(MIXERLINECONTROLSA);
          pmxlc.dwLineID = mxl.dwLineID;
          pmxlc.dwControlID = MIXER_WAVE_RECORD_CONTROL_ID;
          pmxlc.cControls = 1;
          pmxlc.cbmxctrl = sizeof(MIXERCONTROLW);
          pmxlc.pamxctrl = reinterpret_cast<LPMIXERCONTROLA>(&mxctrl);

          if (mixerGetLineControlsA(reinterpret_cast<HMIXEROBJ>(hmx), &pmxlc,
                                    MIXER_GETLINECONTROLSF_ONEBYTYPE) ==
              MMSYSERR_NOERROR) {
            MIXERCONTROLDETAILS pmxcd{};
            pmxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
            pmxcd.dwControlID = mxctrl.dwControlID;
            pmxcd.cChannels = 1;
            pmxcd.cbDetails = sizeof(LONG);

            LONG detailsValue = 0;
            pmxcd.paDetails = &detailsValue;

            if (mixerGetControlDetailsA(reinterpret_cast<HMIXEROBJ>(hmx),
                                        &pmxcd, 0) == MMSYSERR_NOERROR) {
              detailsValue = newLevel;
              mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(hmx), &pmxcd,
                                     0);
            }
          }
        }
        mixerClose(hmx);
      }
    }
  }
}

} // namespace mixer
} // namespace game
