#pragma once

#include "game/structs/core.hpp"
#include "game/structs/ddl.hpp"
#include "game/structs/macros.hpp"

namespace game {
namespace clientcontent {
enum class ClientContentType : uint32_t {
  PAINTSHOP = 0x0,
  LOADOUT = 0x1,
  COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(ClientContentType);

enum class ClientContentTransferState : uint32_t {
  INACTIVE = 0x0,
  DIRTY = 0x1,
  SENDING = 0x2,
  SENT = 0x3,
};
IMPL_ENUM_OPERATORS(ClientContentTransferState);

enum class ClientContentFragmentTransferState : uint32_t {
  INACTIVE = 0x0,
  SENT = 0x1,
  ACKNOWLEDGED = 0x2,
};
IMPL_ENUM_OPERATORS(ClientContentFragmentTransferState);

enum class ClientContentFragmentDataType : uint32_t {
  NONE = 0x0,
  PAINTSHOP = 0x1,
  LOADOUT = 0x2,
  DONE = 0x3,
};
IMPL_ENUM_OPERATORS(ClientContentFragmentDataType);

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
  uint8_t _padding04[4];
  XUID toXUID;
  int32_t numOfAttempts;
  uint16_t cursorPosition;
  uint8_t _padding16[2];
});
ASSERT_SIZE(ClientContentTracking, 0x18);

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

} // namespace clientcontent
} // namespace game
