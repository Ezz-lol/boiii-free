#pragma once

#include "game/structs/lobby/core.hpp"
#include "game/structs/lobby/session.hpp"
#include "game/structs/ddl.hpp"
#include <cstdint>

namespace game {
namespace lobby {
namespace active {
PACKED(struct ClientContentState {
  ddl::DDLContext ddlCtx;
  uint32_t bufferChecksum;
  uint16_t compressedBufferSize;
  uint16_t cursorPosition;
  uint16_t totalContentReceived;
  bool isDataCompressed;
  uint8_t _padding33[1];
});

struct ClientContent {
  ClientContentState contentStates[2];
  eModes sessionMode;
  eGameModes gameMode;
  uint8_t buffer[163840];
};

PACKED(struct ClientContentTracking {
  ClientContentTransferState state;
  XUID toXUID;
  int32_t numOfAttempts;
  uint16_t cursorPosition;
  uint8_t _padding12[2];
});

PACKED(struct ClientContentFragments {
  uint16_t offset;
  uint16_t fragmentLength;
  int32_t serverSequenceNum;
  ClientContentFragmentTransferState fragmentTrasferState;
  ClientContentFragmentDataType fragmentDataType;
  XUID toXUID;
  int32_t fragmentSentTime;
  uint8_t _padding1C[4];
});

PACKED(struct ActiveClient {
  MutableClientInfo mutableClientInfo;
  FixedClientInfo fixedClientInfo;
  session::SessionInfo sessionInfo[3];
  XUID probedXuid;
  int32_t probedXuidTimestamp;
  int32_t uploadBandwidth;
  net::connectionType_e connectionType;
  ClientContent clientContent;
  LobbyClientPool<ClientContentTracking[2]> tracking;
  uint8_t _padding28894[4];
  ClientContentFragments fragments[128];
});
ASSERT_SIZE(ActiveClient, 0x29898);

struct ActiveList {
  LobbyModule module;
  uint8_t _padding04[4];
  const char *name;
  LobbyClientPool<ActiveClient *> client;
};
} // namespace active
} // namespace lobby
} // namespace game