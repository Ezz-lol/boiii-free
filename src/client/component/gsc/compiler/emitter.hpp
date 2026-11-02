#pragma once

#include "compiler.hpp"
#include "parser.hpp"
#include <component/gsc/gsc.hpp>

namespace gsc {
namespace compiler {

// Import flags (FunctionFlags | CallFlags combined in one byte)
enum class import_flags : uint8_t {
  NONE = 0x0,
  FUNC_GETFUNCTION = 0x01,
  FUNC_CALL = 0x02,
  FUNC_THREAD = 0x03,
  FUNC_METHOD = 0x04,
  FUNC_METHOD_THREAD = 0x05,
  CALL_EXTERNAL = 0x00,
  CALL_EXTERNAL_DEV = 0x10,
  CALL_LOCAL = 0x20,
  CALL_LOCAL_DEV = 0x30,
};
IMPL_ENUM_OPERATORS(import_flags);

// Export flags
enum class export_flags : uint8_t {
  NONE = 0,
  AUTOEXEC = 2,
  PRIVATE = 4,
  VARARG = 0x20,
};
IMPL_ENUM_OPERATORS(export_flags);

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

emitter_result emit(game::scr::scriptInstance_t inst, const ast_ptr &root,
                    const std::string &script_name);
} // namespace compiler
} // namespace gsc
