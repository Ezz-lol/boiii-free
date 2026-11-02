#pragma once

#include "compiler.hpp"
#include "parser.hpp"
#include <component/gsc/gsc.hpp>

namespace gsc {
namespace compiler {

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
