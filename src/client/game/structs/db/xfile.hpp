#ifndef STRUCTS_DB_XFILE_HPP
#define STRUCTS_DB_XFILE_HPP

#include <cstdint>

namespace game {
namespace db {
namespace xfile {

enum class XFileBlock : int32_t {
  XFILE_BLOCK_TEMP = 0x0,
  XFILE_BLOCK_RUNTIME_VIRTUAL = 0x1,
  XFILE_BLOCK_RUNTIME_PHYSICAL = 0x2,
  XFILE_BLOCK_UNKNOWN_USED_TO_BE_VIRTUAL_WTF = 0x3,
  XFILE_BLOCK_VIRTUAL = 0x4,
  XFILE_BLOCK_PHYSICAL = 0x5,
  XFILE_BLOCK_STREAMER = 0x6,
  XFILE_BLOCK_MEMMAPPED = 0x7,
  MAX_XFILE_COUNT = 0x8,
};
enum class XFileCompression : uint32_t {
  XFILE_UNCOMPRESSED = 0x0,
  XFILE_ZLIB = 0x1,
  XFILE_ZLIB_HC = 0x2,
  XFILE_LZ4 = 0x3,
  XFILE_LZ4_HC = 0x4,
  XFILE_COMPRESSION_COUNT = 0x5,
};

struct XFile {
  uint8_t magic[8];
  uint32_t version;
  uint8_t server;
  uint8_t compression;
  uint8_t platform;
  uint8_t encrypted;
  uint64_t timestamp;
  uint32_t changelist;
  uint32_t archiveChecksum[4];
  char builder[32];
  uint32_t metaVersion;
  char mergeFastfile[64];
  uint64_t size;
  uint64_t externalSize;
  uint64_t memMappedOffset; // correct
  uint64_t blockSize[10];
  int8_t fastfileName[64];
  uint8_t signature[256];
  uint8_t aesIV[16];
};
} // namespace xfile
} // namespace db
} // namespace game

#endif