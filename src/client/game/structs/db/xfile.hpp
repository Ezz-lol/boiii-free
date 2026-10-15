#pragma once

#include <cstdint>

#include <structs/str.hpp>

#include <game/structs/macros.hpp>

namespace game {
namespace db {
namespace xfile {

enum class XFileBlock : int32_t {
  TEMP = 0x0,
  RUNTIME_VIRTUAL = 0x1,
  RUNTIME_PHYSICAL = 0x2,
  UNKNOWN_PREVIOUSLY_VIRTUAL = 0x3,
  VIRTUAL = 0x4,
  PHYSICAL = 0x5,
  STREAMER = 0x6,
  MEMMAPPED = 0x7,
  COUNT = 0x8,
};
enum class XFileCompression : uint32_t {
  UNCOMPRESSED = 0x0,
  ZLIB = 0x1,
  ZLIB_HC = 0x2,
  LZ4 = 0x3,
  LZ4_HC = 0x4,
  COUNT = 0x5,
};
IMPL_ENUM_OPERATORS(XFileCompression);

enum class XFileCompression8 : uint8_t {
  UNCOMPRESSED = 0x0,
  // Note: ZLIB_HC handled identically to ZLIB in engine; ZLIB_HC is actually an
  // alias to ZLIB
  ZLIB = 0x1,
  ZLIB_HC = 0x2,
  // Note: LZ4_HC handled identically to LZ4 in engine; LZ4_HC is actually an
  // alias to LZ4
  LZ4 = 0x3,
  LZ4_HC = 0x4,
  COUNT = 0x5,
};
IMPL_ENUM_OPERATORS(XFileCompression8);

enum class XFilePlatform : uint32_t {
  PC = 0x0,
  DURANGO = 0x1,
  ORBIS = 0x2,
};
IMPL_ENUM_OPERATORS(XFilePlatform);

enum class XFilePlatform8 : uint8_t {
  PC = 0x0,
  DURANGO = 0x1,
  ORBIS = 0x2,
};

IMPL_ENUM_OPERATORS(XFilePlatform8);

struct XFile {
  uint8_t magic[8];
  uint32_t version;
  bool server;
  XFileCompression8 compression;
  XFilePlatform8 platform;
  bool encrypted;
  uint64_t timestamp;
  uint32_t changelist;
  uint32_t archiveChecksum[4];
  str32_t builder;
  uint32_t metaVersion;
  char mergeFastfile[64];
  uint64_t size;
  uint64_t externalSize;
  uint64_t memMappedOffset; // correct
  uint64_t blockSize[10];
  zoneName_t fastfileName;
  uint8_t signature[256];
  uint8_t aesIV[16];
};
} // namespace xfile
} // namespace db
} // namespace game
