#pragma once

#include "gsc_parser.hpp"
#include "gsc_compiler.hpp"
#include "gsc.hpp"

namespace gsc_compiler {

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
  std::vector<uint8_t> gdb;
  std::string error;
  int32_t error_line;
  int32_t error_column;
  std::vector<gsc::hash_name_pair> hash_names;
  std::vector<replacefunc_entry> replacefuncs;
};

emitter_result emit(const ast_ptr &root, const std::string &script_name);
} // namespace gsc_compiler