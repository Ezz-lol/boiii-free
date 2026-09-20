#pragma once

#include <cstdint>

#include <game/structs/macros.hpp>
#include <structs/func.hpp>

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
ASSERT_SIZE(DDLDef, 0x60);

struct DDLState {
  bool isValid;
  int32_t offset;
  int32_t arrayIndex;
  DDLMember *member;
  const DDLDef *ddlDef;
};
ASSERT_SIZE(DDLState, 0x20);

struct DDLContext;
typedef fastcallPtr_t<void(DDLContext *ctx, void *data)> DDLWriteCB;

struct DDLContext {
  void *buff;
  int32_t len;
  const DDLDef *def;
  DDLWriteCB writeCB;
  void *userData;
};

PACKED(struct StringTableCell {
  const char *string;
  int32_t hash;
  uint8_t _padding0C[4];
});

struct StringTable {
  const char *name;
  int32_t columnCount;
  int32_t rowCount;
  StringTableCell *values;
  int16_t *cellIndex;
};
ASSERT_SIZE(StringTable, 0x20);

} // namespace ddl
} // namespace game