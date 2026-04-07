#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace gsc_compiler {
struct compile_error {
  std::string message;
  std::string file;
  int line;
  int column;
};

struct hash_name_pair {
  uint32_t hash;
  std::string name;
  int line;
  uint8_t params;
};

struct replacefunc_entry {
  std::string target_script;
  std::string target_func;
  std::string replace_func;
};

struct compile_result {
  bool success;
  std::vector<uint8_t> bytecode;
  std::vector<compile_error> errors;
  std::vector<hash_name_pair> hash_names;
  std::vector<replacefunc_entry> replacefuncs;
};

// Compile raw GSC source code to T7 PC bytecode.
compile_result compile(const std::string &source,
                       const std::string &source_name);
} // namespace gsc_compiler
