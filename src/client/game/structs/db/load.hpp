#pragma once

#include "core.hpp"
#include "xfile.hpp"
#include <game/structs/core.hpp>
#include <game/structs/quake/core.hpp>
#include <game/structs/stream.hpp>

#include <cstdint>

namespace game {
namespace db {

typedef int32_t DBFile;

typedef void DB_Interrupt(void);

struct alignas(128) DB_LOAD_STREAM {
  uint8_t decompressedData[262144];
  uint8_t checksum[32];
  uint8_t *compressedData;
  int32_t expectedDataAvail;
  int32_t compressedDataAvail;
  int32_t alignedDataAvail;
  int32_t decompressedDataAvail;
  int32_t decompressedDataOffset;
  bool busy;
};

PACKED(struct DBDecompressCmd_t {
  int32_t which;
  qboolean isEncrypted;
  xfile::XFileCompression compression;
  uint8_t _padding0C[4];
  DB_LOAD_STREAM *stream;
  uint8_t *ioBuffer;
  int64_t numBytesDecrypted;
});
ASSERT_SIZE(DBDecompressCmd_t, 0x28);

struct BDiffState {
  bool headerRead;
  bool error;
  bool eof;
  uint32_t features;
};

struct BDiffHeader {
  xfile::XFile newXFileHeader;
  xfile::XFile baseXFileHdr;
  size_t size;
  float version;
  uint32_t flags;
  size_t maxDestWindowSize;
  size_t maxSourceWindowSize;
  size_t maxDiffWindowSize;
};

enum class DB_PATCH_STREAM_STATUS : uint32_t {
  Idle = 0x0,
  Loading = 0x1,
  LoadDone = 0x2,
  EndOfFile = 0x3,
};

typedef uint8_t Byte;
typedef Byte Bytef;
typedef uint32_t uInt;
typedef uint32_t UInt32;
typedef size_t SizeT;
typedef uint16_t UInt16;
typedef uint64_t uLong;
typedef void *voidpf;

typedef fastcallPtr_t<void(voidpf, voidpf)> free_func;
typedef fastcallPtr_t<voidpf(voidpf, uInt, uInt)> alloc_func;

struct internal_state {
  int32_t dummy;
};

struct z_stream_s {
  Bytef *next_in;
  uInt avail_in;
  uLong total_in;
  Bytef *next_out;
  uInt avail_out;
  uLong total_out;
  char *msg;
  internal_state *state;
  alloc_func zalloc;
  free_func zfree;
  voidpf opaque;
  int32_t data_type;
  uLong adler;
  uLong reserved;
};

typedef z_stream_s z_stream;

struct _CLzmaProps {
  uint32_t lc;
  uint32_t lp;
  uint32_t pb;
  UInt32 dicSize;
};

typedef _CLzmaProps CLzmaProps;

struct __declspec(align(8)) CLzmaDec {
  CLzmaProps prop;
  UInt16 *probs;
  Byte *dic;
  const Byte *buf;
  UInt32 range;
  UInt32 code;
  SizeT dicPos;
  SizeT dicBufSize;
  UInt32 processedPos;
  UInt32 checkDicSize;
  uint32_t state;
  UInt32 reps[4];
  uint32_t remainLen;
  int32_t needFlush;
  int32_t needInitState;
  UInt32 numProbs;
  uint32_t tempBufSize;
  Byte tempBuf[20];
};

struct ISzAlloc {
  fastcallPtr_t<void(void *, size_t)> Alloc;
  fastcallPtr_t<void(void *, void *)> Free;
};

struct SHA1_CTX {
  uint32_t state[5];
  uint32_t count[2];
  uint8_t buffer[64];
};

struct __declspec(align(8)) DB_PATCH_STREAM {
  bool active;
  BDiffState diffState;
  uint32_t archiveChecksum[4];
  BDiffHeader header;
  uint8_t *sourceWindow;
  size_t sourceWindowOffset;
  size_t sourceWindowSize;
  size_t sourceWindowAllocated;
  uint8_t *destWindow;
  size_t destWindowOffset;
  size_t destWindowSize;
  size_t destWindowAllocated;
  size_t destWindowReadOffset;
  uint8_t *patchWindow;
  size_t patchWindowOffset;
  size_t patchWindowSize;
  size_t patchWindowAllocated;
  size_t patchWindowOffsetLast;
  stream::stream_fileid file;
  uint8_t *decompressBuffer;
  uint8_t *decompressBufferLoad;
  size_t decompressBufferSize;
  size_t decompressBufferAllocated;
  size_t decompressBufferLoadOffset;
  size_t decompressBufferLoadSize;
  DB_PATCH_STREAM_STATUS decompressBufferLoadStatus;
  z_stream zStream;
  bool endOfStream;
  CLzmaDec lzmaStream;
  ISzAlloc lzmaAlloc;
  size_t lzmaMemUsed;
  size_t lzmaDataInBuffer;
  SHA1_CTX shaCtx;
};

// Verified
struct DB_LoadData {
  DBFile f;
  uint8_t *fileBuffer;
  size_t fileBufferSize;
  const char *filename;
  XBlock *blocks;
  int32_t flags;
  DB_Interrupt *interrupt;
  PMemStack side;
  int64_t readOffset;
  int64_t endOffset;
  uint8_t *ioBuffer;
  uint8_t *compressData;
  int32_t tail;
  int32_t head;
  DBDecompressCmd_t cmd[4];
  DB_LOAD_STREAM *stream;
  int32_t next_buffer;
  int32_t locks[4];
  bool loadPending[4];
  bool abort;
  bool suspend;
  int32_t lastError;
  xfile::XFile header;
  int32_t numBlocksDecrypted;
  int32_t startTime;
  DB_PATCH_STREAM patchStream;
};
ASSERT_SIZE(DB_LoadData, 0xA88);
} // namespace db
} // namespace game