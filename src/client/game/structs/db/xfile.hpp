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
inline constexpr const char *serialize(const XFileBlock block) {
  switch (block) {
  case XFileBlock::TEMP:
    return "BLOCK_TEMP";
  case XFileBlock::RUNTIME_VIRTUAL:
    return "BLOCK_RUNTIME_VIRTUAL";
  case XFileBlock::RUNTIME_PHYSICAL:
    return "BLOCK_RUNTIME_PHYSICAL";
  case XFileBlock::UNKNOWN_PREVIOUSLY_VIRTUAL:
    return "BLOCK_UNKNOWN_PREVIOUSLY_VIRTUAL";
  case XFileBlock::VIRTUAL:
    return "BLOCK_VIRTUAL";
  case XFileBlock::PHYSICAL:
    return "BLOCK_PHYSICAL";
  case XFileBlock::STREAMER:
    return "BLOCK_STREAMER";
  case XFileBlock::MEMMAPPED:
    return "BLOCK_MEMMAPPED";
  default:
    return "BLOCK_INVALID";
  }
}

enum class XFileCompression : uint32_t {
  UNCOMPRESSED = 0x0,
  ZLIB = 0x1,
  ZLIB_HC = 0x2,
  LZ4 = 0x3,
  LZ4_HC = 0x4,
  COUNT = 0x5,
};
inline constexpr const char *serialize(const XFileCompression comp) {
  switch (comp) {
  case XFileCompression::UNCOMPRESSED:
    return "COMP_UNCOMPRESSED";
  case XFileCompression::ZLIB:
    return "COMP_ZLIB";
  case XFileCompression::ZLIB_HC:
    return "COMP_ZLIB_HC";
  case XFileCompression::LZ4:
    return "COMP_LZ4";
  case XFileCompression::LZ4_HC:
    return "COMP_LZ4_HC";
  default:
    return "COMP_INVALID";
  }
}
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
inline constexpr const char *serialize(const XFileCompression8 comp) {
  switch (comp) {
  case XFileCompression8::UNCOMPRESSED:
    return "COMP8_UNCOMPRESSED";
  case XFileCompression8::ZLIB:
    return "COMP8_ZLIB";
  case XFileCompression8::ZLIB_HC:
    return "COMP8_ZLIB_HC";
  case XFileCompression8::LZ4:
    return "COMP8_LZ4";
  case XFileCompression8::LZ4_HC:
    return "COMP8_LZ4_HC";
  default:
    return "COMP8_INVALID";
  }
}
IMPL_ENUM_OPERATORS(XFileCompression8);

enum class XFilePlatform : uint32_t {
  PC = 0x0,
  DURANGO = 0x1,
  ORBIS = 0x2,
};
inline constexpr const char *serialize(const XFilePlatform plat) {
  switch (plat) {
  case XFilePlatform::PC:
    return "PLAT_PC";
  case XFilePlatform::DURANGO:
    return "PLAT_DURANGO";
  case XFilePlatform::ORBIS:
    return "PLAT_ORBIS";
  default:
    return "PLAT_INVALID";
  }
}
IMPL_ENUM_OPERATORS(XFilePlatform);

enum class XFilePlatform8 : uint8_t {
  PC = 0x0,
  DURANGO = 0x1,
  ORBIS = 0x2,
};
inline constexpr const char *serialize(const XFilePlatform8 plat) {
  switch (plat) {
  case XFilePlatform8::PC:
    return "PLAT8_PC";
  case XFilePlatform8::DURANGO:
    return "PLAT8_DURANGO";
  case XFilePlatform8::ORBIS:
    return "PLAT8_ORBIS";
  default:
    return "PLAT8_INVALID";
  }
}
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"magic\": {}, \"version\": {:08X}, \"server\": {}, "
        "\"compression\": {}, "
        "\"platform\": {}, \"encrypted\": {}, \"timestamp\": {}, "
        "\"changelist\": {:08X}, "
        "\"archiveChecksum\": {}, \"builder\": {}, \"metaVersion\": "
        "{:08X}, \"mergeFastfile\": {}, "
        "\"size\": {}, \"externalSize\": {}, \"memMappedOffset\": {}, "
        "\"blockSize\": {}, "
        "\"fastfileName\": {}, \"signature\": {}, \"aesIV\": {} }}",
        ::serialize(magic), version, ::serialize(server),
        game::db::xfile::serialize(compression),
        game::db::xfile::serialize(platform), ::serialize(encrypted), timestamp,
        changelist, ::serialize(archiveChecksum), ::serialize(builder),
        metaVersion, ::serialize(mergeFastfile), size, externalSize,
        memMappedOffset, ::serialize(blockSize), ::serialize(fastfileName),
        ::serialize(signature), ::serialize(aesIV));
  }
};
} // namespace xfile
} // namespace db
} // namespace game
