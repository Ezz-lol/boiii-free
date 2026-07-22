#pragma once

#include <cstdint>

#include "func.hpp"

namespace game {

namespace ddl {
struct DDLMember {
  const char *name;
  int32_t index;
  void *parent;
  int32_t bitSize;
  int32_t limitSize;
  int32_t offset;
  int32_t type;
  int32_t externalIndex;
  uint32_t rangeLimit;
  uint32_t serverDelta;
  uint32_t clientDelta;
  int32_t arraySize;
  int32_t enumIndex;
  int32_t permission;
};

struct DDLHash {
  int32_t hash;
  int32_t index;
};

struct DDLHashTable {
  DDLHash *list;
  int32_t count;
  int32_t max;
};

struct DDLStruct {
  const char *name;
  int32_t bitSize;
  int32_t memberCount;
  DDLMember *members;
  DDLHashTable hashTableUpper;
  DDLHashTable hashTableLower;
};

struct DDLEnum {
  const char *name;
  int32_t memberCount;
  const char **members;
  DDLHashTable hashTable;
};

struct DDLDef {
  char *name;
  uint16_t version;
  uint32_t checksum;
  uint8_t flags;
  int32_t bitSize;
  int32_t byteSize;
  DDLStruct *structList;
  int32_t structCount;
  DDLEnum *enumList;
  int32_t enumCount;
  DDLDef *next;
  int32_t headerBitSize;
  int32_t headerByteSize;
  int32_t reserveSize;
  int32_t userFlagsSize;
  bool paddingUsed;
};

struct DDLContext;
typedef fastcallPtr_t<void(DDLContext *ctx, void *data)> DDLWriteCB;

struct DDLContext {
  void *buff;
  int32_t len;
  const DDLDef *def;
  DDLWriteCB writeCB;
  void *userData;
};

} // namespace ddl
} // namespace game