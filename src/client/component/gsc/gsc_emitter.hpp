#pragma once

#include "gsc_parser.hpp"
#include "gsc_compiler.hpp"
#include "gsc.hpp"

namespace gsc_compiler {

// T7 script header (0x48 bytes, little-endian PC)
#pragma pack(push, 1)
struct t7_script_header {
  uint64_t magic;
  uint32_t source_crc;
  uint32_t include_offset;
  uint32_t animtree_offset;
  uint32_t bytecode_offset;
  uint32_t string_offset;
  uint32_t debug_string_offset;
  uint32_t export_offset;
  uint32_t import_offset;
  uint32_t fixup_offset;
  uint32_t profile_offset;
  uint32_t bytecode_size;
  uint32_t name_offset;
  uint16_t string_count;
  uint16_t export_count;
  uint16_t import_count;
  uint16_t fixup_count;
  uint16_t profile_count;
  uint16_t debug_string_count;
  uint8_t include_count;
  uint8_t animtree_count;
  uint16_t flags;
};
#pragma pack(pop)

static_assert(sizeof(t7_script_header) == 0x48, "T7 header must be 0x48 bytes");

// Import flags (FunctionFlags | CallFlags combined in one byte)
enum import_flags : uint8_t {
  IMPORT_FUNC_GETFUNCTION = 0x01,
  IMPORT_FUNC_CALL = 0x02,
  IMPORT_FUNC_THREAD = 0x03,
  IMPORT_FUNC_METHOD = 0x04,
  IMPORT_FUNC_METHOD_THREAD = 0x05,
  IMPORT_CALL_EXTERNAL = 0x00,
  IMPORT_CALL_EXTERNAL_DEV = 0x10,
  IMPORT_CALL_LOCAL = 0x20,
  IMPORT_CALL_LOCAL_DEV = 0x30,
};

// Export flags
enum export_flags : uint8_t {
  EXPORT_NONE = 0,
  EXPORT_AUTOEXEC = 2,
  EXPORT_PRIVATE = 4,
};

struct emitter_result {
  bool success;
  std::vector<uint8_t> data;
  std::string error;
  int error_line;
  int error_column;
  std::vector<gsc::hash_name_pair> hash_names;
  std::vector<replacefunc_entry> replacefuncs;
};

emitter_result emit(const ast_ptr &root, const std::string &script_name);
} // namespace gsc_compiler