#pragma once

#include <cstdint>
#include "../quake.hpp"
#include "core.hpp"
#include "xfile.hpp"

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

#pragma pack(push, 1)
// sizeof=0x28
struct DBDecompressCmd_t {
  int32_t which;
  qboolean isEncrypted;
  xfile::XFileCompression compression;
  uint8_t _padding0C[4];
  DB_LOAD_STREAM *stream;
  uint8_t *ioBuffer;
  int64_t numBytesDecrypted;
};
static_assert(sizeof(DBDecompressCmd_t) == 0x28,
              "DBDecompressCmd_t size must be 0x28 bytes");
#pragma pack(pop)

#ifdef STRUCTS_DB_XFILE_HPP
struct DB_LoadData {
  DBFile f;
  const char *filename;
  XBlock *blocks;
  int32_t flags;
  DB_Interrupt *interrupt;
  PMemStack side;
  int64_t readOffset;
  int64_t endOffset;
  uint8_t *ioBuffer;
  int32_t ioBufferOffset;
  uint8_t *compressData;
  int32_t tail;
  int32_t head;
  DBDecompressCmd_t cmd[4];
  DB_LOAD_STREAM *stream;
  int32_t next_buffer;
  int32_t locks[4];
  bool loadPending[4];
  bool abort;
  int32_t lastError;
  bool throttleDecompression;
  xfile::XFile header;
  int32_t numBlocksDecrypted;
  int32_t startTime;
};
#endif
} // namespace db
} // namespace game