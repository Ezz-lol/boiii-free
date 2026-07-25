#include <std_include.hpp>
#include "gsc_emitter.hpp"
#include <algorithm>
#include <cstring>
#include <game/game.hpp>

#include "component/gsc/gsc_funcs.hpp"

namespace gsc_compiler {
namespace {
using namespace game;
using namespace game::scr;
using namespace game::scr::vm::op;

uint32_t align_value(uint32_t val, uint32_t alignment) {
  return (val + alignment - 1) & ~(alignment - 1);
}

void write_u8(std::vector<uint8_t> &buf, uint8_t v) { buf.push_back(v); }
void write_u16(std::vector<uint8_t> &buf, uint16_t v) {
  buf.push_back(v & 0xFF);
  buf.push_back((v >> 8) & 0xFF);
}
void write_u32(std::vector<uint8_t> &buf, uint32_t v) {
  const size_t s = buf.size();
  buf.resize(s + 4);
  std::memcpy(&buf[s], &v, 4);
}
void write_i16(std::vector<uint8_t> &buf, int16_t v) {
  write_u16(buf, static_cast<uint16_t>(v));
}
void write_float(std::vector<uint8_t> &buf, float v) {
  const size_t s = buf.size();
  buf.resize(s + 4);
  std::memcpy(&buf[s], &v, 4);
}

void write_at_u16(std::vector<uint8_t> &buf, size_t offset, uint16_t v) {
  buf[offset] = v & 0xFF;
  buf[offset + 1] = (v >> 8) & 0xFF;
}

void write_at_u32(std::vector<uint8_t> &buf, size_t offset, uint32_t v) {
  std::memcpy(&buf[offset], &v, 4);
}

void write_at_i16(std::vector<uint8_t> &buf, size_t offset, int16_t v) {
  write_at_u16(buf, offset, static_cast<uint16_t>(v));
}

uint64_t align_value64(uint64_t val, uint64_t alignment) {
  return (val + alignment - 1) & ~(alignment - 1);
}

// T7 PC opcode table
inline OP_TYPE map_opcode(Opcode op) {
  if (OPCODE_BYTECODE_MAP.contains(op)) {
    return OPCODE_BYTECODE_MAP.at(op)[0];
  }

  fprintf(stderr,
          "Warning: could not find valid bytecode value for opcode: "
          "0x%02X. Emitting a no-op opcode.\n",
          static_cast<uint8_t>(op));
  fflush(stderr);
  return OPCODE_BYTECODE_MAP.at(Opcode::UnknownOrInvalid)[0];
}

struct string_entry {
  std::string value;
  uint32_t offset;
  std::vector<std::pair<size_t, uint32_t>> references;
};

struct import_entry {
  uint32_t function_hash;
  uint32_t namespace_hash;
  uint8_t num_params;
  uint8_t flags;
  std::vector<std::pair<size_t, uint32_t>> references;
};

struct export_entry {
  uint32_t function_hash;
  uint32_t namespace_hash;
  uint8_t num_params;
  uint8_t flags;
  std::vector<uint8_t> bytecode;
  uint32_t bytecode_offset;

  std::vector<uint32_t> local_hashes;
  std::unordered_map<uint32_t, uint8_t> local_indices;

  uint8_t add_local(uint32_t hash) {
    auto it = local_indices.find(hash);
    if (it != local_indices.end())
      return it->second;
    uint8_t idx = static_cast<uint8_t>(local_hashes.size());
    local_hashes.push_back(hash);
    local_indices[hash] = idx;
    return idx;
  }

  uint8_t get_local_index(uint32_t hash) const {
    auto it = local_indices.find(hash);
    if (it == local_indices.end())
      return 0xFF;
    return it->second;
  }
};

struct jump_fixup {
  uint32_t offset_location;
  uint32_t jump_end;
  int32_t target_label;
};

struct loop_context {
  int32_t break_label;
  int32_t continue_label;
};

struct emitter_state {
  export_entry *current_func;
  size_t current_export_index;
  uint32_t script_namespace;
  std::string script_name;
  std::unordered_map<std::string, uint8_t> local_function_params;

  std::vector<string_entry> strings;
  std::unordered_map<std::string, size_t> string_map;

  std::vector<import_entry> imports;

  std::vector<std::string> includes;

  std::vector<export_entry> exports;

  std::vector<jump_fixup> jump_fixups;
  std::unordered_map<int, uint32_t>
      label_positions; // label_id -> bytecode offset
  int32_t next_label_id;

  std::vector<loop_context> loop_stack;

  int32_t temp_var_counter;

  std::vector<gsc::hash_name_pair> hash_names;
  std::vector<replacefunc_entry> replacefuncs;

  void record_hash(const std::string &name, int32_t line = 0,
                   uint8_t params = 0) {
    std::string lower = name;
    std::transform(
        lower.begin(), lower.end(), lower.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    uint32_t h = gsc::gsc_hash(lower);
    hash_names.push_back({h, lower, line, params});
  }

  emitter_state()
      : current_func(nullptr), current_export_index(0), script_namespace(0),
        next_label_id(0), temp_var_counter(0) {}

  int32_t new_label() { return next_label_id++; }

  void set_label(int id) {
    label_positions[id] = static_cast<uint32_t>(current_func->bytecode.size());
  }

  void emit_op(Opcode op) { write_u16(current_func->bytecode, map_opcode(op)); }

  void emit_u8(uint8_t v) { write_u8(current_func->bytecode, v); }

  void emit_u16_aligned() {
    uint32_t pos = static_cast<uint32_t>(current_func->bytecode.size());
    uint32_t aligned = align_value(pos, 2);
    while (current_func->bytecode.size() < aligned)
      current_func->bytecode.push_back(0);
  }

  void emit_u32_aligned() {
    uint32_t pos = static_cast<uint32_t>(current_func->bytecode.size());
    uint32_t aligned = align_value(pos, 4);
    while (current_func->bytecode.size() < aligned)
      current_func->bytecode.push_back(0);
  }

  void emit_u16(uint16_t v) { write_u16(current_func->bytecode, v); }
  void emit_u32(uint32_t v) { write_u32(current_func->bytecode, v); }
  void emit_i16(int16_t v) { write_i16(current_func->bytecode, v); }
  void emit_float(float v) { write_float(current_func->bytecode, v); }

  void emit_jump(Opcode op, int32_t target_label) {
    emit_op(op);
    emit_u16_aligned();
    uint32_t offset_loc = static_cast<uint32_t>(current_func->bytecode.size());
    emit_i16(0); // placeholder
    jump_fixups.push_back({offset_loc, offset_loc + 2, target_label});
  }

  size_t add_string(const std::string &str) {
    auto it = string_map.find(str);
    if (it != string_map.end())
      return it->second;
    size_t idx = strings.size();
    strings.push_back({str, 0, {}});
    string_map[str] = idx;
    return idx;
  }

  void emit_string_ref(Opcode op, const std::string &str) {
    size_t idx = add_string(str);
    emit_op(op);
    emit_u32_aligned();
    strings[idx].references.push_back(
        {current_export_index,
         static_cast<uint32_t>(current_func->bytecode.size())});
    emit_u32(0xFFFFFFFF);
  }

  uint64_t make_import_key(uint32_t func, uint32_t ns, uint8_t params,
                           uint8_t flags) const {
    return (static_cast<uint64_t>(func) << 32) |
           (static_cast<uint64_t>(ns) << 16) |
           (static_cast<uint64_t>(params) << 8) | flags;
  }

  size_t add_import(ScrVarCanonicalName_t func_hash,
                    ScrVarCanonicalName_t ns_hash, uint8_t num_params,
                    uint8_t flags) {
    for (size_t i = 0; i < imports.size(); i++) {
      if (imports[i].function_hash == func_hash &&
          imports[i].namespace_hash == ns_hash &&
          imports[i].num_params == num_params && imports[i].flags == flags)
        return i;
    }
    imports.push_back({func_hash, ns_hash, num_params, flags, {}});
    return imports.size() - 1;
  }

  void emit_call(ScrVarCanonicalName_t func_hash, ScrVarCanonicalName_t ns_hash,
                 uint8_t num_params, bool is_method, bool is_thread,
                 bool same_namespace, bool builtin = false) {

    if (!builtin && same_namespace) {
      if (is_method) {
        if (gsc::builtin_method(func_hash)) {
          builtin = true;
        }
      } else if (gsc::builtin_function(func_hash)) {
        builtin = true;
      }
    }

    if (builtin) {
      ns_hash = gsc::GSCR_SYS_NS_HASH;
      is_thread = false;
    }

    uint8_t flags = 0;

    if (builtin) {
      flags = is_method ? IMPORT_FUNC_METHOD : IMPORT_FUNC_CALL;
    } else if (is_method) {
      flags = is_thread ? IMPORT_FUNC_METHOD_THREAD : IMPORT_FUNC_METHOD;
    } else {
      flags = is_thread ? IMPORT_FUNC_THREAD : IMPORT_FUNC_CALL;
    }

    if (builtin || same_namespace) {
      flags |= IMPORT_CALL_LOCAL;
    }

    Opcode op;
    if (builtin) {
      op = is_method ? Opcode::CallBuiltinMethod : Opcode::CallBuiltin;
    } else if (is_method) {
      op =
          is_thread ? Opcode::ScriptMethodThreadCall : Opcode::ScriptMethodCall;
    } else {
      op = is_thread ? Opcode::ScriptThreadCall : Opcode::ScriptFunctionCall;
    }

    uint32_t opcode_pos = static_cast<uint32_t>(current_func->bytecode.size());
    emit_op(op);

    size_t import_idx = add_import(func_hash, ns_hash, num_params, flags);
    imports[import_idx].references.push_back(
        {current_export_index, opcode_pos});

    emit_u8(num_params);
    // QWord align
    {
      uint32_t pad_pos = static_cast<uint32_t>(current_func->bytecode.size());
      uint32_t aligned = (pad_pos + 8) & ~7u;
      while (current_func->bytecode.size() < aligned)
        current_func->bytecode.push_back(0);
    }
    emit_u32(func_hash);
    emit_u32(0);
  }

  void emit_call_ptr(uint8_t num_params, bool is_method, bool is_thread) {
    Opcode op;
    if (is_method)
      op = is_thread ? Opcode::ScriptMethodThreadCallPointer
                     : Opcode::ScriptMethodCallPointer;
    else
      op = is_thread ? Opcode::ScriptThreadCallPointer
                     : Opcode::ScriptFunctionCallPointer;

    emit_op(op);
    emit_u16(static_cast<uint16_t>(num_params));
  }

  std::string temp_var_name() {
    return "__temp_" + std::to_string(temp_var_counter++);
  }
};

void emit_expression(emitter_state &s, const ast_ptr &node);
void emit_statement(emitter_state &s, const ast_ptr &node);
void emit_block(emitter_state &s, const ast_ptr &node);
void emit_lvalue(emitter_state &s, const ast_ptr &node, bool is_ref);

void pre_register_temps(emitter_state &s, const ast_ptr &node) {
  if (!node)
    return;

  if (node->type == node_type::n_foreach) {
    std::string array_temp = s.temp_var_name();
    s.current_func->add_local(gsc::gsc_hash(array_temp));

    std::string key_name = node->children[0]->value;
    if (key_name.empty()) {
      key_name = s.temp_var_name();
      s.current_func->add_local(gsc::gsc_hash(key_name));
    }
  } else if (node->type == node_type::n_switch) {
    std::string switch_temp = s.temp_var_name();
    s.current_func->add_local(gsc::gsc_hash(switch_temp));
  }

  for (const std::shared_ptr<ast_node> &child : node->children)
    pre_register_temps(s, child);
}

void collect_locals(const ast_ptr &node, std::vector<std::string> &locals,
                    const std::vector<std::string> &params) {
  if (!node)
    return;

  if (node->type == node_type::n_assign) {
    if (node->children.size() > 0 &&
        node->children[0]->type == node_type::n_identifier) {
      const std::string &name = node->children[0]->value;
      bool is_param =
          std::find(params.begin(), params.end(), name) != params.end();
      bool already =
          std::find(locals.begin(), locals.end(), name) != locals.end();
      if (!is_param && !already)
        locals.push_back(name);
    }
  }

  if (node->type == node_type::n_foreach) {
    const std::string &val_name = node->value;
    bool is_param =
        std::find(params.begin(), params.end(), val_name) != params.end();
    bool already =
        std::find(locals.begin(), locals.end(), val_name) != locals.end();
    if (!is_param && !already)
      locals.push_back(val_name);

    if (node->children.size() > 0 && !node->children[0]->value.empty()) {
      const std::string &key_name = node->children[0]->value;
      is_param =
          std::find(params.begin(), params.end(), key_name) != params.end();
      already =
          std::find(locals.begin(), locals.end(), key_name) != locals.end();
      if (!is_param && !already)
        locals.push_back(key_name);
    }
  }

  if (node->type == node_type::n_waittill && node->children.size() > 1) {
    const std::shared_ptr<ast_node> &args = node->children[1]; // args block
    for (size_t i = 1; i < args->children.size();
         i++) // skip first (event name)
    {
      if (args->children[i]->type == node_type::n_identifier) {
        const std::string &name = args->children[i]->value;
        bool is_param =
            std::find(params.begin(), params.end(), name) != params.end();
        bool already =
            std::find(locals.begin(), locals.end(), name) != locals.end();
        if (!is_param && !already)
          locals.push_back(name);
      }
    }
  }

  for (const std::shared_ptr<ast_node> &child : node->children)
    collect_locals(child, locals, params);
}
std::string normalize_ns(const std::string &ns) {
  std::string result = ns;
  for (char &c : result)
    if (c == '\\')
      c = '/';
  return result;
}

bool is_path_namespace(const std::string &ns) {
  return ns.find('/') != std::string::npos ||
         ns.find('\\') != std::string::npos;
}

void auto_include_path(emitter_state &s, const std::string &ns) {
  std::string normalized = ns;
  for (char &c : normalized) {
    if (c == '\\')
      c = '/';
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  for (const std::string &inc : s.includes) {
    std::string norm_inc = inc;
    for (char &c : norm_inc) {
      if (c == '\\')
        c = '/';
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    if (norm_inc == normalized)
      return;
  }
  s.includes.push_back(ns);
}

std::pair<std::string, std::string> extract_func_ref(const ast_ptr &arg) {
  if (arg->type == node_type::n_func_ref) {
    std::string ns =
        (!arg->children.empty() && !arg->children[0]->value.empty())
            ? arg->children[0]->value
            : "";
    return {ns, arg->value};
  } else if (arg->type == node_type::n_call &&
             arg->children[1]->children.empty()) {
    std::string ns =
        !arg->children[0]->value.empty() ? arg->children[0]->value : "";
    return {ns, arg->value};
  }
  return {"", ""};
}

int infer_local_function_params(const emitter_state &s,
                                const std::string &ref_ns,
                                const std::string &func_name) {
  if (func_name.empty()) {
    return -1;
  }

  std::string normalized_ref =
      normalize_ns(ref_ns.empty() ? s.script_name : ref_ns);
  std::string normalized_self = normalize_ns(s.script_name);
  std::transform(
      normalized_ref.begin(), normalized_ref.end(), normalized_ref.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  std::transform(
      normalized_self.begin(), normalized_self.end(), normalized_self.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  if (normalized_ref != normalized_self) {
    return -1;
  }

  std::string lower_name = func_name;
  std::transform(
      lower_name.begin(), lower_name.end(), lower_name.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  const auto it = s.local_function_params.find(lower_name);
  if (it == s.local_function_params.end()) {
    return -1;
  }

  return static_cast<int>(it->second);
}

bool is_builtin(const std::string &name) {
  std::string lower = name;
  std::transform(
      lower.begin(), lower.end(), lower.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return lower == "isdefined" || lower == "vectorscale" || lower == "gettime" ||
         lower == "firstarraykey" || lower == "nextarraykey" ||
         lower == "getfirstarraykey" || lower == "getnextarraykey" ||
         lower == "waitrealtime" || lower == "realwait";
}

bool try_emit_builtin(emitter_state &s, const std::string &name) {
  std::string lower = name;
  std::transform(
      lower.begin(), lower.end(), lower.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  if (lower == "isdefined") {
    s.emit_op(Opcode::IsDefined);
    return true;
  }
  if (lower == "vectorscale") {
    s.emit_op(Opcode::VectorScale);
    return true;
  }
  if (lower == "gettime") {
    s.emit_op(Opcode::GetTime);
    return true;
  }
  if (lower == "firstarraykey") {
    s.emit_op(Opcode::FirstArrayKey);
    return true;
  }
  if (lower == "nextarraykey") {
    s.emit_op(Opcode::NextArrayKey);
    return true;
  }
  if (lower == "getfirstarraykey") {
    s.emit_op(Opcode::FirstArrayKey);
    return true;
  }
  if (lower == "getnextarraykey") {
    s.emit_op(Opcode::NextArrayKey);
    return true;
  }
  /*
    TODO: `waitrealtime` and `realwait`.
    - `waitrealtime` is already handled elsewhere.

    - `realwait` is not handled at all. This seems to be a `waitrealtime`
    equivalent that is specific to CSC code, where it waits for a client-side
    amount of time, rather than a time interval synchronized with the server.
  */
  return false;
}

void emit_get_number(emitter_state &s, int64_t value) {
  if (value == 0) {
    s.emit_op(Opcode::GetZero);
  } else if (value > 0 && value <= 255) {
    s.emit_op(Opcode::GetByte);
    s.emit_u16(static_cast<uint16_t>(value));
  } else if (value < 0 && value >= -255) {
    s.emit_op(Opcode::GetNegByte);
    s.emit_u16(static_cast<uint16_t>(-value));
  } else if (value > 0 && value <= 65535) {
    s.emit_op(Opcode::GetUnsignedShort);
    s.emit_u16_aligned();
    s.emit_u16(static_cast<uint16_t>(value));
  } else if (value < 0 && value >= -65535) {
    s.emit_op(Opcode::GetNegUnsignedShort);
    s.emit_u16_aligned();
    s.emit_u16(static_cast<uint16_t>(-value));
  } else {
    s.emit_op(Opcode::GetInteger);
    s.emit_u32_aligned();
    s.emit_u32(static_cast<uint32_t>(value));
  }
}

void emit_eval_local(emitter_state &s, const std::string &name, bool is_ref,
                     bool is_waittill = false) {
  std::string lower = name;
  std::transform(
      lower.begin(), lower.end(), lower.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  uint32_t hash = gsc::gsc_hash(lower);
  uint8_t idx = s.current_func->get_local_index(hash);
  if (idx == 0xFF) {
    idx = s.current_func->add_local(hash);
  }

  if (is_waittill) {
    s.emit_op(Opcode::SafeSetWaittillVariableFieldCached);
    s.emit_u16(static_cast<uint16_t>(idx));
  } else if (is_ref) {
    s.emit_op(Opcode::EvalLocalVariableRefCached);
    s.emit_u16(static_cast<uint16_t>(idx));
  } else {
    s.emit_op(Opcode::EvalLocalVariableCached);
    s.emit_u16(static_cast<uint16_t>(idx));
  }
}

void emit_object(emitter_state &s, const ast_ptr &node) {
  if (node->type == node_type::n_self)
    s.emit_op(Opcode::GetSelfObject);
  else if (node->type == node_type::n_level)
    s.emit_op(Opcode::GetLevelObject);
  else if (node->type == node_type::n_world)
    s.emit_op(Opcode::GetWorldObject);
  else if (node->type == node_type::n_anim)
    s.emit_op(Opcode::GetAnimObject);
  else {
    emit_expression(s, node);
    s.emit_op(Opcode::CastFieldObject);
  }
}

void emit_owner(emitter_state &s, const ast_ptr &node) {
  if (node->type == node_type::n_self)
    s.emit_op(Opcode::GetSelf);
  else if (node->type == node_type::n_level)
    s.emit_op(Opcode::GetLevel);
  else if (node->type == node_type::n_world)
    s.emit_op(Opcode::GetWorld);
  else if (node->type == node_type::n_anim)
    s.emit_op(Opcode::GetAnim);
  else if (node->type == node_type::n_game)
    s.emit_op(Opcode::GetGame);
  else
    emit_expression(s, node);
}

void emit_expression(emitter_state &s, const ast_ptr &node) {
  if (!node)
    return;

  switch (node->type) {
  case node_type::n_number: {
    int64_t val = 0;
    const std::string &v = node->value;
    if (v.size() > 2 && v[0] == '0' && (v[1] == 'x' || v[1] == 'X'))
      val = std::stoll(v, nullptr, 16);
    else
      val = std::stoll(v);
    emit_get_number(s, val);
    break;
  }
  case node_type::n_float_number: {
    float fval = std::stof(node->value);
    s.emit_op(Opcode::GetFloat);
    s.emit_u32_aligned();
    s.emit_float(fval);
    break;
  }
  case node_type::n_string:
    s.emit_string_ref(Opcode::GetString, node->value);
    break;
  case node_type::n_istring:
    s.emit_string_ref(Opcode::GetIString, node->value);
    break;
  case node_type::n_hash_string: {
    uint32_t hash = gsc::gsc_hash(node->value);
    s.emit_op(Opcode::GetHash);
    s.emit_u32_aligned();
    s.emit_u32(hash);
    break;
  }
  case node_type::n_true_val:
    emit_get_number(s, 1);
    break;
  case node_type::n_false_val:
    s.emit_op(Opcode::GetZero);
    break;
  case node_type::n_undefined:
    s.emit_op(Opcode::GetUndefined);
    break;
  case node_type::n_self:
    s.emit_op(Opcode::GetSelf);
    break;
  case node_type::n_level:
    s.emit_op(Opcode::GetLevel);
    break;
  case node_type::n_game:
    s.emit_op(Opcode::GetGame);
    break;
  case node_type::n_anim:
    s.emit_op(Opcode::GetAnim);
    break;
  case node_type::n_world:
    s.emit_op(Opcode::GetWorld);
    break;
  case node_type::n_empty_array:
    s.emit_op(Opcode::GetEmptyArray);
    break;

  case node_type::n_identifier: {
    std::string lower = node->value;
    std::transform(
        lower.begin(), lower.end(), lower.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    emit_eval_local(s, lower, false);
    break;
  }

  case node_type::n_field_access: {
    // children[0] = object
    emit_object(s, node->children[0]);
    uint32_t field_hash = gsc::gsc_hash(node->value);
    s.emit_op(Opcode::EvalFieldVariable);
    s.emit_u32_aligned();
    s.emit_u32(field_hash);
    break;
  }

  case node_type::n_array_access: {
    // children[0] = array, children[1] = key
    emit_expression(s, node->children[1]); // key first
    emit_expression(s, node->children[0]); // then array
    s.emit_op(Opcode::EvalArray);
    break;
  }

  case node_type::n_size: {
    emit_expression(s, node->children[0]);
    s.emit_op(Opcode::SizeOf);
    break;
  }

  case node_type::n_vector: {
    emit_expression(s, node->children[2]); // z
    emit_expression(s, node->children[1]); // y
    emit_expression(s, node->children[0]); // x
    s.emit_op(Opcode::Vector);
    break;
  }

  case node_type::n_ternary: {
    int32_t false_label = s.new_label();
    int32_t end_label = s.new_label();
    emit_expression(s, node->children[0]); // condition
    s.emit_jump(Opcode::JumpOnFalseExpr, false_label);
    emit_expression(s, node->children[1]); // true value
    s.emit_jump(Opcode::Jump, end_label);
    s.set_label(false_label);
    emit_expression(s, node->children[2]); // false value
    s.set_label(end_label);
    break;
  }

  case node_type::n_binary_op: {
    emit_expression(s, node->children[0]); // left

    // Short-circuit for && and ||
    if (node->value == "&&") {
      int32_t skip_label = s.new_label();
      s.emit_jump(Opcode::JumpOnFalseExpr, skip_label);
      emit_expression(s, node->children[1]);
      s.set_label(skip_label);
      break;
    }
    if (node->value == "||") {
      int32_t skip_label = s.new_label();
      s.emit_jump(Opcode::JumpOnTrueExpr, skip_label);
      emit_expression(s, node->children[1]);
      s.set_label(skip_label);
      break;
    }

    emit_expression(s, node->children[1]); // right

    if (node->value == "+")
      s.emit_op(Opcode::Plus);
    else if (node->value == "-")
      s.emit_op(Opcode::Minus);
    else if (node->value == "*")
      s.emit_op(Opcode::Multiply);
    else if (node->value == "/")
      s.emit_op(Opcode::Divide);
    else if (node->value == "%")
      s.emit_op(Opcode::Modulus);
    else if (node->value == "&")
      s.emit_op(Opcode::Bit_And);
    else if (node->value == "|")
      s.emit_op(Opcode::Bit_Or);
    else if (node->value == "^")
      s.emit_op(Opcode::Bit_Xor);
    else if (node->value == "<<")
      s.emit_op(Opcode::ShiftLeft);
    else if (node->value == ">>")
      s.emit_op(Opcode::ShiftRight);
    else if (node->value == "==")
      s.emit_op(Opcode::Equal);
    else if (node->value == "!=")
      s.emit_op(Opcode::NotEqual);
    else if (node->value == "<")
      s.emit_op(Opcode::LessThan);
    else if (node->value == ">")
      s.emit_op(Opcode::GreaterThan);
    else if (node->value == "<=")
      s.emit_op(Opcode::LessThanOrEqualTo);
    else if (node->value == ">=")
      s.emit_op(Opcode::GreaterThanOrEqualTo);
    else if (node->value == "===")
      s.emit_op(Opcode::SuperEqual);
    else if (node->value == "!==")
      s.emit_op(Opcode::SuperNotEqual);
    else
      throw std::runtime_error("Unknown binary operator: " + node->value);
    break;
  }

  case node_type::n_unary_op: {
    emit_expression(s, node->children[0]);
    if (node->value == "!")
      s.emit_op(Opcode::BoolNot);
    else if (node->value == "~")
      s.emit_op(Opcode::Bit_Not);
    else if (node->value == "-") {
      emit_get_number(s, -1);
      s.emit_op(Opcode::Multiply);
    }
    break;
  }

  case node_type::n_func_ref: {
    ScrVarCanonicalName_t func_hash = gsc::gsc_hash(node->value);
    ScrVarCanonicalName_t ns_hash = s.script_namespace;
    if (!node->children.empty() && !node->children[0]->value.empty()) {
      ns_hash = gsc::gsc_hash(normalize_ns(node->children[0]->value));
      if (is_path_namespace(node->children[0]->value))
        auto_include_path(s, node->children[0]->value);
    }

    uint8_t flags = IMPORT_FUNC_GETFUNCTION;
    if (ns_hash == s.script_namespace)
      flags |= IMPORT_CALL_LOCAL;

    size_t import_idx = s.add_import(func_hash, ns_hash, 0, flags);

    uint32_t opcode_pos =
        static_cast<uint32_t>(s.current_func->bytecode.size());
    s.emit_op(Opcode::GetFunction);
    s.imports[import_idx].references.push_back(
        {s.current_export_index, opcode_pos});
    {
      uint32_t pad_pos = static_cast<uint32_t>(s.current_func->bytecode.size());
      uint32_t aligned = static_cast<uint32_t>(align_value64(pad_pos, 8));
      while (s.current_func->bytecode.size() < aligned)
        s.current_func->bytecode.push_back(0);
    }
    s.emit_u32(func_hash);
    s.emit_u32(0);
    break;
  }

  case node_type::n_call: {
    std::string func_name = node->value;
    std::string lower_name = func_name;
    std::transform(
        lower_name.begin(), lower_name.end(), lower_name.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    const std::shared_ptr<ast_node> &ns_node = node->children[0];
    const std::shared_ptr<ast_node> &args_node = node->children[1];
    uint8_t num_params = static_cast<uint8_t>(args_node->children.size());

    if (ns_node->value.empty() && is_builtin(lower_name)) {
      for (const std::shared_ptr<ast_node> &arg : args_node->children)
        emit_expression(s, arg);
      try_emit_builtin(s, lower_name);
      break;
    }

    s.emit_op(Opcode::PreScriptCall);

    for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0; i--)
      emit_expression(s, args_node->children[i]);

    ScrVarCanonicalName_t func_hash = gsc::gsc_hash(lower_name);
    bool has_explicit_ns = !ns_node->value.empty();
    ScrVarCanonicalName_t ns_hash =
        has_explicit_ns ? gsc::gsc_hash(normalize_ns(ns_node->value))
                        : s.script_namespace;

    // Auto-include for path namespaces
    if (has_explicit_ns && is_path_namespace(ns_node->value))
      auto_include_path(s, ns_node->value);

    bool is_local = !has_explicit_ns;

    s.record_hash(lower_name, node->line, num_params);
    s.emit_call(func_hash, ns_hash, num_params, false, false, is_local);
    break;
  }

  case node_type::n_method_call: {
    const std::shared_ptr<ast_node> &obj = node->children[0];
    const std::shared_ptr<ast_node> &args_node = node->children[1];
    uint8_t num_params = static_cast<uint8_t>(args_node->children.size());

    std::string func_name = node->value;
    std::string lower_name = func_name;
    std::transform(
        lower_name.begin(), lower_name.end(), lower_name.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    s.emit_op(Opcode::PreScriptCall);

    for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0; i--)
      emit_expression(s, args_node->children[i]);

    emit_expression(s, obj);

    ScrVarCanonicalName_t func_hash = gsc::gsc_hash(lower_name);
    bool has_explicit_ns =
        (node->children.size() > 2 && !node->children[2]->value.empty());
    ScrVarCanonicalName_t ns_hash =
        has_explicit_ns ? gsc::gsc_hash(normalize_ns(node->children[2]->value))
                        : s.script_namespace;

    // Auto-include for path namespaces
    if (has_explicit_ns && is_path_namespace(node->children[2]->value))
      auto_include_path(s, node->children[2]->value);

    bool is_local = !has_explicit_ns;

    s.record_hash(lower_name, node->line, num_params);
    s.emit_call(func_hash, ns_hash, num_params, true, false, is_local);
    break;
  }

  case node_type::n_call_ptr: {
    const std::shared_ptr<ast_node> &args_node = node->children[2];
    uint8_t num_params = static_cast<uint8_t>(args_node->children.size());
    bool has_caller = node->children[0]->type != node_type::n_undefined;

    s.emit_op(Opcode::PreScriptCall);

    for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0; i--)
      emit_expression(s, args_node->children[i]);

    if (has_caller)
      emit_expression(s, node->children[0]);

    emit_expression(s, node->children[1]); // func ptr
    s.emit_call_ptr(num_params, has_caller, false);
    break;
  }

  case node_type::n_thread_call: {
    const std::shared_ptr<ast_node> &inner = node->children[0];
    if (inner->type == node_type::n_call) {
      const std::shared_ptr<ast_node> &ns_node = inner->children[0];
      const std::shared_ptr<ast_node> &args_node = inner->children[1];
      uint8_t num_params = static_cast<uint8_t>(args_node->children.size());

      std::string lower_name = inner->value;
      std::transform(
          lower_name.begin(), lower_name.end(), lower_name.begin(),
          [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

      s.emit_op(Opcode::PreScriptCall);
      for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0;
           i--)
        emit_expression(s, args_node->children[i]);

      ScrVarCanonicalName_t func_hash = gsc::gsc_hash(lower_name);
      bool has_explicit_ns = !ns_node->value.empty();
      ScrVarCanonicalName_t ns_hash =
          has_explicit_ns ? gsc::gsc_hash(normalize_ns(ns_node->value))
                          : s.script_namespace;
      if (has_explicit_ns && is_path_namespace(ns_node->value))
        auto_include_path(s, ns_node->value);
      bool is_local = !has_explicit_ns;
      s.record_hash(lower_name, inner->line, num_params);
      s.emit_call(func_hash, ns_hash, num_params, false, true, is_local);
    } else if (inner->type == node_type::n_method_call) {
      const std::shared_ptr<ast_node> &obj = inner->children[0];
      const std::shared_ptr<ast_node> &args_node = inner->children[1];
      uint8_t num_params = static_cast<uint8_t>(args_node->children.size());

      std::string lower_name = inner->value;
      std::transform(
          lower_name.begin(), lower_name.end(), lower_name.begin(),
          [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

      s.emit_op(Opcode::PreScriptCall);
      for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0;
           i--)
        emit_expression(s, args_node->children[i]);

      emit_expression(s, obj);

      ScrVarCanonicalName_t func_hash = gsc::gsc_hash(lower_name);
      bool has_ns =
          (inner->children.size() > 2 && !inner->children[2]->value.empty());
      ScrVarCanonicalName_t ns_hash =
          has_ns ? gsc::gsc_hash(normalize_ns(inner->children[2]->value))
                 : s.script_namespace;
      if (has_ns && is_path_namespace(inner->children[2]->value))
        auto_include_path(s, inner->children[2]->value);
      bool is_local = !has_ns;
      s.record_hash(lower_name, inner->line, num_params);
      s.emit_call(func_hash, ns_hash, num_params, true, true, is_local);
    } else if (inner->type == node_type::n_call_ptr) {
      const std::shared_ptr<ast_node> &args_node = inner->children[2];
      uint8_t num_params = static_cast<uint8_t>(args_node->children.size());
      bool has_caller = inner->children[0]->type != node_type::n_undefined;

      s.emit_op(Opcode::PreScriptCall);
      for (int i = static_cast<int>(args_node->children.size()) - 1; i >= 0;
           i--)
        emit_expression(s, args_node->children[i]);

      if (has_caller)
        emit_expression(s, inner->children[0]);

      emit_expression(s, inner->children[1]);
      s.emit_call_ptr(num_params, has_caller, true);
    }
    break;
  }

  case node_type::n_inc_dec: {
    emit_lvalue(s, node->children[0], true);
    if (node->value == "post++" || node->value == "pre++")
      s.emit_op(Opcode::Inc);
    else
      s.emit_op(Opcode::Dec);
    break;
  }

  default:
    throw std::runtime_error("Cannot emit expression for node type " +
                             std::to_string(static_cast<int>(node->type)) +
                             " at line " + std::to_string(node->line));
  }
}

void emit_lvalue(emitter_state &s, const ast_ptr &node, bool is_ref) {
  switch (node->type) {
  case node_type::n_identifier: {
    std::string lower = node->value;
    std::transform(
        lower.begin(), lower.end(), lower.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    emit_eval_local(s, lower, is_ref);
    break;
  }
  case node_type::n_field_access: {
    emit_object(s, node->children[0]);
    uint32_t field_hash = gsc::gsc_hash(node->value);
    if (is_ref) {
      s.emit_op(Opcode::EvalFieldVariableRef);
      s.emit_u32_aligned();
      s.emit_u32(field_hash);
    } else {
      s.emit_op(Opcode::EvalFieldVariable);
      s.emit_u32_aligned();
      s.emit_u32(field_hash);
    }
    break;
  }
  case node_type::n_array_access: {
    emit_expression(s, node->children[1]);   // key
    emit_lvalue(s, node->children[0], true); // array ref
    s.emit_op(Opcode::EvalArrayRef);
    break;
  }
  default:
    emit_expression(s, node);
    break;
  }
}

void emit_statement(emitter_state &s, const ast_ptr &node) {
  if (!node)
    return;

  switch (node->type) {
  case node_type::n_block:
    emit_block(s, node);
    break;

  case node_type::n_expression_stmt: {
    std::shared_ptr<gsc_compiler::ast_node> &expr = node->children[0];

    if (expr->type == node_type::n_call) {
      std::string call_name = std::string(expr->value.c_str());
      std::transform(
          call_name.begin(), call_name.end(), call_name.begin(),
          [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

      if (expr->children[0]->value.empty() && call_name == "detour") {
        const std::vector<std::shared_ptr<ast_node>> &args =
            expr->children[1]->children;
        if (args.size() == 2) {
          auto [target_ns, target_fn] = extract_func_ref(args[0]);
          auto [replace_ns, replace_fn] = extract_func_ref(args[1]);
          if (!target_fn.empty() && !replace_fn.empty()) {
            std::string ts =
                normalize_ns(target_ns.empty() ? s.script_name : target_ns);
            std::string rs =
                normalize_ns(replace_ns.empty() ? s.script_name : replace_ns);
            std::string tfn = target_fn;
            std::transform(tfn.begin(), tfn.end(), tfn.begin(),
                           [](unsigned char c) {
                             return static_cast<char>(std::tolower(c));
                           });
            std::string rfn = replace_fn;
            std::transform(rfn.begin(), rfn.end(), rfn.begin(),
                           [](unsigned char c) {
                             return static_cast<char>(std::tolower(c));
                           });

            const std::function<std::string(std::string & script)>
                strip_script_ext = [](std::string script) {
                  for (char &c : script)
                    if (c == '\\')
                      c = '/';
                  if (script.size() >= 4 &&
                      (script.substr(script.size() - 4) == ".gsc" ||
                       script.substr(script.size() - 4) == ".csc"))
                    script = script.substr(0, script.size() - 4);
                  return script;
                };

            std::string target_script = strip_script_ext(ts);
            std::string replace_script = strip_script_ext(rs);
            const int32_t replace_params =
                infer_local_function_params(s, replace_ns, replace_fn);
            int32_t target_params =
                infer_local_function_params(s, target_ns, target_fn);
            if (target_params < 0) {
              target_params = replace_params;
            }

            s.replacefuncs.push_back({target_script, tfn, replace_script, rfn,
                                      target_params, replace_params, true});
            break;
          }
        }
      }
    }

    if (expr->type == node_type::n_endon || expr->type == node_type::n_notify ||
        expr->type == node_type::n_waittill ||
        expr->type == node_type::n_assign ||
        expr->type == node_type::n_inc_dec) {
      emit_statement(s, expr);
      break;
    }
    emit_expression(s, expr);
    if (expr->type == node_type::n_call ||
        expr->type == node_type::n_method_call ||
        expr->type == node_type::n_call_ptr ||
        expr->type == node_type::n_thread_call) {
      s.emit_op(Opcode::DecTop);
    }
    break;
  }

  case node_type::n_assign: {
    const std::shared_ptr<ast_node> &target = node->children[0];
    const std::shared_ptr<ast_node> &value = node->children[1];
    const std::string &op = node->value;

    if (op == "=") {
      emit_expression(s, value);
    } else {
      emit_expression(s, target);
      emit_expression(s, value);
      if (op == "+=")
        s.emit_op(Opcode::Plus);
      else if (op == "-=")
        s.emit_op(Opcode::Minus);
      else if (op == "*=")
        s.emit_op(Opcode::Multiply);
      else if (op == "/=")
        s.emit_op(Opcode::Divide);
      else if (op == "%=")
        s.emit_op(Opcode::Modulus);
      else if (op == "&=")
        s.emit_op(Opcode::Bit_And);
      else if (op == "|=")
        s.emit_op(Opcode::Bit_Or);
      else if (op == "^=")
        s.emit_op(Opcode::Bit_Xor);
      else if (op == "<<=")
        s.emit_op(Opcode::ShiftLeft);
      else if (op == ">>=")
        s.emit_op(Opcode::ShiftRight);
    }

    emit_lvalue(s, target, true);
    s.emit_op(Opcode::SetVariableField);
    break;
  }

  case node_type::n_inc_dec: {
    emit_lvalue(s, node->children[0], true);
    if (node->value == "post++" || node->value == "pre++")
      s.emit_op(Opcode::Inc);
    else
      s.emit_op(Opcode::Dec);
    break;
  }

  case node_type::n_return: {
    if (!node->children.empty()) {
      emit_expression(s, node->children[0]);
      s.emit_op(Opcode::Return);
    } else {
      s.emit_op(Opcode::End);
    }
    break;
  }

  case node_type::n_wait: {
    emit_expression(s, node->children[0]);
    s.emit_op(Opcode::Wait);
    break;
  }

  case node_type::n_waittillframeend:
    s.emit_op(Opcode::WaitTillFrameEnd);
    break;

  case node_type::n_waitrealtime: {
    emit_expression(s, node->children[0]);
    s.emit_op(Opcode::WaitRealTime);
    break;
  }

  case node_type::n_if: {
    int32_t else_label = s.new_label();
    int32_t end_label = s.new_label();

    emit_expression(s, node->children[0]); // condition
    s.emit_jump(Opcode::JumpOnFalse, else_label);

    emit_statement(s, node->children[1]); // if body

    if (node->children.size() > 2) {
      s.emit_jump(Opcode::Jump, end_label);
      s.set_label(else_label);
      emit_statement(s, node->children[2]); // else body
      s.set_label(end_label);
    } else {
      s.set_label(else_label);
    }
    break;
  }

  case node_type::n_while: {
    int32_t loop_start = s.new_label();
    int32_t loop_end = s.new_label();
    int32_t loop_continue = loop_start;

    s.loop_stack.push_back({loop_end, loop_continue});

    s.set_label(loop_start);
    emit_expression(s, node->children[0]); // condition
    s.emit_jump(Opcode::JumpOnFalse, loop_end);
    emit_statement(s, node->children[1]); // body
    s.emit_jump(Opcode::Jump, loop_start);
    s.set_label(loop_end);

    s.loop_stack.pop_back();
    break;
  }

  case node_type::n_do_while: {
    int32_t loop_start = s.new_label();
    int32_t loop_end = s.new_label();
    int32_t loop_continue = s.new_label();

    s.loop_stack.push_back({loop_end, loop_continue});

    s.set_label(loop_start);
    emit_statement(s, node->children[1]); // body
    s.set_label(loop_continue);
    emit_expression(s, node->children[0]); // condition
    s.emit_jump(Opcode::JumpOnTrue, loop_start);
    s.set_label(loop_end);

    s.loop_stack.pop_back();
    break;
  }

  case node_type::n_for: {
    // children: [0]=init, [1]=cond, [2]=iter, [3]=body
    int32_t loop_cond = s.new_label();
    int32_t loop_end = s.new_label();
    int32_t loop_continue = s.new_label();

    s.loop_stack.push_back({loop_end, loop_continue});

    if (node->children[0]->type != node_type::n_undefined)
      emit_statement(s, node->children[0]);

    s.set_label(loop_cond);

    if (node->children[1]->type != node_type::n_true_val) {
      emit_expression(s, node->children[1]);
      s.emit_jump(Opcode::JumpOnFalse, loop_end);
    }

    emit_statement(s, node->children[3]);

    s.set_label(loop_continue);
    if (node->children[2]->type != node_type::n_undefined)
      emit_statement(s, node->children[2]);

    s.emit_jump(Opcode::Jump, loop_cond);
    s.set_label(loop_end);

    s.loop_stack.pop_back();
    break;
  }

  case node_type::n_foreach: {
    // value = node->value, children[0] = key name, [1] = array, [2] = body
    std::string val_name = node->value;
    std::string key_name = node->children[0]->value;
    bool has_key = !key_name.empty();

    std::string array_temp = s.temp_var_name();
    if (!has_key)
      key_name = s.temp_var_name();

    s.current_func->add_local(gsc::gsc_hash(array_temp));
    s.current_func->add_local(gsc::gsc_hash(key_name));
    s.current_func->add_local(gsc::gsc_hash(val_name));

    // array_temp = <array_expr>
    emit_expression(s, node->children[1]);
    emit_eval_local(s, array_temp, true);
    s.emit_op(Opcode::SetVariableField);

    // key = firstArrayKey(array_temp)
    emit_eval_local(s, array_temp, false);
    s.emit_op(Opcode::FirstArrayKey);
    emit_eval_local(s, key_name, true);
    s.emit_op(Opcode::SetVariableField);

    int32_t loop_start = s.new_label();
    int32_t loop_end = s.new_label();
    int32_t loop_continue = s.new_label();

    s.loop_stack.push_back({loop_end, loop_continue});

    s.set_label(loop_start);

    // if (!isDefined(key)) break
    emit_eval_local(s, key_name, false);
    s.emit_op(Opcode::IsDefined);
    s.emit_jump(Opcode::JumpOnFalse, loop_end);

    // val = array_temp[key]
    emit_eval_local(s, key_name, false);
    emit_eval_local(s, array_temp, false);
    s.emit_op(Opcode::EvalArray);
    emit_eval_local(s, val_name, true);
    s.emit_op(Opcode::SetVariableField);

    emit_statement(s, node->children[2]);

    s.set_label(loop_continue);

    // key = nextArrayKey(array_temp, key)
    emit_eval_local(s, key_name, false);
    emit_eval_local(s, array_temp, false);
    s.emit_op(Opcode::NextArrayKey);
    emit_eval_local(s, key_name, true);
    s.emit_op(Opcode::SetVariableField);

    s.emit_jump(Opcode::Jump, loop_start);
    s.set_label(loop_end);

    s.loop_stack.pop_back();
    break;
  }

  case node_type::n_switch: {
    // children[0] = expr, children[1..] = cases
    std::string switch_temp = s.temp_var_name();
    s.current_func->add_local(gsc::gsc_hash(switch_temp));

    emit_expression(s, node->children[0]);
    emit_eval_local(s, switch_temp, true);
    s.emit_op(Opcode::SetVariableField);

    int32_t switch_end = s.new_label();
    s.loop_stack.push_back({switch_end, -1}); // break goes to switch_end

    // Emit cases as if-else chain
    std::vector<int> case_labels;
    int32_t default_label = -1;

    for (size_t i = 1; i < node->children.size(); i++) {
      const std::shared_ptr<ast_node> &case_node = node->children[i];
      int32_t label = s.new_label();
      case_labels.push_back(label);

      if (case_node->type == node_type::n_default_case) {
        default_label = label;
      } else {
        // Compare switch_temp == case_value
        emit_expression(s, case_node->children[0]);
        emit_eval_local(s, switch_temp, false);
        s.emit_op(Opcode::Equal);
        s.emit_jump(Opcode::JumpOnTrue, label);
      }
    }

    if (default_label >= 0)
      s.emit_jump(Opcode::Jump, default_label);
    else
      s.emit_jump(Opcode::Jump, switch_end);

    for (size_t i = 1; i < node->children.size(); i++) {
      s.set_label(case_labels[i - 1]);
      const std::shared_ptr<ast_node> &case_node = node->children[i];

      if (case_node->type == node_type::n_case) {
        if (case_node->children.size() > 1)
          emit_statement(s, case_node->children[1]); // body
      } else                                         // default
      {
        if (!case_node->children.empty())
          emit_statement(s, case_node->children[0]); // body
      }
    }

    s.set_label(switch_end);
    s.loop_stack.pop_back();
    break;
  }

  case node_type::n_break: {
    if (s.loop_stack.empty())
      throw std::runtime_error("'break' outside of loop at line " +
                               std::to_string(node->line));
    s.emit_jump(Opcode::Jump, s.loop_stack.back().break_label);
    break;
  }

  case node_type::n_continue: {
    if (s.loop_stack.empty())
      throw std::runtime_error("'continue' outside of loop at line " +
                               std::to_string(node->line));
    s.emit_jump(Opcode::Jump, s.loop_stack.back().continue_label);
    break;
  }

  case node_type::n_waittill: {
    // children[0] = object, children[1] = args (first is event name, rest are
    // vars)
    const std::shared_ptr<ast_node> &obj = node->children[0];
    const std::shared_ptr<ast_node> &args = node->children[1];

    if (args->children.empty())
      throw std::runtime_error(
          "waittill requires at least an event name at line " +
          std::to_string(node->line));

    emit_expression(s, args->children[0]); // event name
    emit_owner(s, obj); // object (uses GetLevel, not GetLevelObject)
    s.emit_op(Opcode::WaitTill);

    for (size_t i = 1; i < args->children.size(); i++) {
      if (args->children[i]->type == node_type::n_identifier) {
        emit_eval_local(s, args->children[i]->value, false, true);
      }
    }
    s.emit_op(Opcode::ClearParams);
    break;
  }

  case node_type::n_notify: {
    // children[0] = object, children[1] = args (first is event, rest are
    // params)
    const std::shared_ptr<ast_node> &obj = node->children[0];
    const std::shared_ptr<ast_node> &args = node->children[1];

    s.emit_op(Opcode::PreScriptCall);

    for (int i = static_cast<int>(args->children.size()) - 1; i >= 1; i--)
      emit_expression(s, args->children[i]);

    emit_expression(s, args->children[0]);
    emit_owner(s, obj);
    s.emit_op(Opcode::Notify);
    break;
  }

  case node_type::n_endon: {
    // children[0] = object, children[1] = args (event name)
    const std::shared_ptr<ast_node> &obj = node->children[0];
    const std::shared_ptr<ast_node> &args = node->children[1];

    if (!args->children.empty())
      emit_expression(s, args->children[0]); // event name
    emit_owner(s, obj);
    s.emit_op(Opcode::EndOn);
    break;
  }

  case node_type::n_call:
  case node_type::n_method_call:
  case node_type::n_call_ptr:
  case node_type::n_thread_call:
    emit_expression(s, node);
    s.emit_op(Opcode::DecTop);
    break;

  default:
    emit_expression(s, node);
    break;
  }
}

void emit_block(emitter_state &s, const ast_ptr &node) {
  if (!node)
    return;
  for (const std::shared_ptr<ast_node> &child : node->children)
    emit_statement(s, child);
}

void emit_function(emitter_state &s, const ast_ptr &node) {
  std::string func_name = node->value;
  std::string lower_name = func_name;
  std::transform(
      lower_name.begin(), lower_name.end(), lower_name.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  const std::shared_ptr<ast_node> &flags_node = node->children[0];
  const std::shared_ptr<ast_node> &params_node = node->children[1];
  const std::shared_ptr<ast_node> &body_node = node->children[2];

  export_entry exp{};
  exp.function_hash = gsc::gsc_hash(lower_name);
  exp.namespace_hash = s.script_namespace;
  exp.num_params = static_cast<uint8_t>(params_node->children.size());
  exp.flags = EXPORT_NONE;
  const std::string &flags_str = flags_node->value;
  if (flags_str.find("autoexec") != std::string::npos)
    exp.flags |= EXPORT_AUTOEXEC;
  if (flags_str.find("private") != std::string::npos)
    exp.flags |= EXPORT_PRIVATE;

  s.exports.push_back(std::move(exp));
  s.current_export_index = s.exports.size() - 1;
  s.current_func = &s.exports.back();

  s.jump_fixups.clear();
  s.label_positions.clear();
  s.loop_stack.clear();

  std::vector<std::string> param_names;
  for (const std::shared_ptr<ast_node> &param : params_node->children) {
    std::string pname = param->value;
    std::transform(
        pname.begin(), pname.end(), pname.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    s.current_func->add_local(gsc::gsc_hash(pname));
    param_names.push_back(pname);
  }

  std::vector<std::string> locals;
  collect_locals(body_node, locals, param_names);

  for (const std::string &local : locals) {
    std::string lower = local;
    std::transform(
        lower.begin(), lower.end(), lower.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    s.current_func->add_local(gsc::gsc_hash(lower));
  }

  int32_t saved_temp = s.temp_var_counter;
  pre_register_temps(s, body_node);
  s.temp_var_counter = saved_temp;

  if (s.current_func->local_hashes.empty()) {
    s.emit_op(Opcode::CheckClearParams);
  } else {
    s.emit_op(Opcode::SafeCreateLocalVariables);
    s.emit_u8(static_cast<uint8_t>(s.current_func->local_hashes.size()));
    for (size_t vi = 0; vi < s.current_func->local_hashes.size(); vi++) {
      s.emit_u32_aligned();
      s.emit_u32(s.current_func->local_hashes[vi]); // hash
      s.emit_u8(0); // null terminator after each hash
    }
    s.emit_u8(0); // final null byte (no CheckClearParams!)

    // Reverse variable indices (last declared = index 0)
    uint8_t N = static_cast<uint8_t>(s.current_func->local_hashes.size());
    for (auto &[hash, idx] : s.current_func->local_indices)
      idx = N - 1 - idx;
  }

  // this will allow default values in params
  for (const std::shared_ptr<ast_node> &param : params_node->children) {
    if (param->children.empty())
      continue; // no default value
    std::string pname = param->value;
    std::transform(
        pname.begin(), pname.end(), pname.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    int32_t skip_label = s.new_label();

    emit_eval_local(s, pname, false);            // push param value
    s.emit_op(Opcode::IsDefined);                // isdefined check
    s.emit_jump(Opcode::JumpOnTrue, skip_label); // skip if defined

    emit_expression(s, param->children[0]); // push default value
    emit_eval_local(s, pname, true);        // push param ref
    s.emit_op(Opcode::SetVariableField);    // assign

    s.set_label(skip_label);
  }

  emit_block(s, body_node);

  s.emit_op(Opcode::End);

  for (const jump_fixup &fixup : s.jump_fixups) {
    auto it = s.label_positions.find(fixup.target_label);
    if (it == s.label_positions.end()) {
      throw std::runtime_error("Unresolved jump label in function '" +
                               func_name + "'");
    }

    int16_t offset = static_cast<int16_t>(static_cast<int32_t>(it->second) -
                                          static_cast<int32_t>(fixup.jump_end));
    write_at_i16(s.current_func->bytecode, fixup.offset_location, offset);
  }
}

uint32_t crc32_calc(const uint8_t *data, size_t len) {
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++)
      crc = (crc >> 1) ^ (0xEDB88320 & (0u - (crc & 1)));
  }
  return ~crc;
}

// Binary assembly: Header | Strings | Includes | Code | Exports | Imports |
// AnimTrees | StringFixups
std::vector<uint8_t> assemble(emitter_state &s) {
  std::vector<uint8_t> output;

  output.resize(sizeof(t7_script_header), 0);

  // Include path strings
  std::vector<uint32_t> include_string_offsets;
  for (const std::string &inc_path : s.includes) {
    std::string normalized = inc_path;
    for (char &c : normalized) {
      if (c == '\\')
        c = '/';
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    include_string_offsets.push_back(static_cast<uint32_t>(output.size()));
    for (char c : normalized)
      output.push_back(static_cast<uint8_t>(c));
    output.push_back(0);
  }

  // Script name
  uint32_t name_offset = static_cast<uint32_t>(output.size());
  for (char c : s.script_name)
    output.push_back(static_cast<uint8_t>(c));
  output.push_back(0);

  // Code strings
  for (string_entry &str : s.strings) {
    str.offset = static_cast<uint32_t>(output.size());
    for (char c : str.value)
      output.push_back(static_cast<uint8_t>(c));
    output.push_back(0);
  }

  // Include table (reversed)
  uint32_t include_offset = static_cast<uint32_t>(output.size());
  for (int i = static_cast<int>(include_string_offsets.size()) - 1; i >= 0; i--)
    write_u32(output, include_string_offsets[i]);

  // Code section
  uint32_t bytecode_start = static_cast<uint32_t>(output.size());

  for (size_t i = 0; i < s.exports.size(); i++) {
    export_entry &exp = s.exports[i];

    // Double QWord align before each function
    {
      uint32_t pos = static_cast<uint32_t>(output.size());
      uint32_t a1 = (pos + 8) & ~7u;
      output.resize(a1, 0);
      uint32_t a2 = (a1 + 8) & ~7u;
      output.resize(a2, 0);
    }

    exp.bytecode_offset = static_cast<uint32_t>(output.size());

    output.insert(output.end(), exp.bytecode.begin(), exp.bytecode.end());
  }

  // bytecode_size covers the full code section with alignment
  uint32_t bytecode_end = static_cast<uint32_t>(output.size());
  uint32_t total_bytecode_size = bytecode_end - bytecode_start;

  // Patch string placeholders with actual offsets
  const std::function<uint32_t(const std::pair<size_t, uint32_t> &ref)>
      resolve_ref = [&](const std::pair<size_t, uint32_t> &ref) -> uint32_t {
    return s.exports[ref.first].bytecode_offset + ref.second;
  };

  for (const string_entry &str : s.strings) {
    for (const std::pair<size_t, uint32_t> &ref : str.references) {
      uint32_t abs_offset = resolve_ref(ref);
      write_at_u32(output, abs_offset, str.offset);
    }
  }

  // Export table
  uint32_t export_offset = static_cast<uint32_t>(output.size());

  for (size_t i = 0; i < s.exports.size(); i++) {
    const export_entry &exp = s.exports[i];
    uint32_t crc =
        crc32_calc(output.data() + exp.bytecode_offset, exp.bytecode.size());

    write_u32(output, crc);
    write_u32(output, exp.bytecode_offset);
    write_u32(output, exp.function_hash);
    write_u32(output, exp.namespace_hash);
    write_u8(output, exp.num_params);
    write_u8(output, exp.flags);
    write_u16(output, 0); // Unknown, always 0
  }

  // Import table
  uint32_t import_offset = static_cast<uint32_t>(output.size());
  for (const import_entry &imp : s.imports) {
    write_u32(output, imp.function_hash);
    write_u32(output, imp.namespace_hash);
    write_u16(output, static_cast<uint16_t>(imp.references.size()));
    write_u8(output, imp.num_params);
    write_u8(output, imp.flags);

    for (const std::pair<size_t, uint32_t> &ref : imp.references)
      write_u32(output, resolve_ref(ref));
  }

  // AnimTree section (empty)
  uint32_t animtree_offset = static_cast<uint32_t>(output.size());

  // String fixup table: u32 string_ptr | u32 num_refs | u32 refs[N]
  uint32_t string_fixup_offset = static_cast<uint32_t>(output.size());
  uint16_t string_count = 0;
  for (const string_entry &str : s.strings) {
    if (str.references.empty())
      continue;
    string_count++;

    write_u32(output, str.offset);
    write_u32(output, static_cast<uint32_t>(str.references.size()));

    for (size_t j = 0; j < str.references.size(); j++)
      write_u32(output, resolve_ref(str.references[j]));
  }

  uint32_t file_size = static_cast<uint32_t>(output.size());

  // Header
  t7_script_header header{};
  header.magic = gsc::T7_MAGIC;
  header.source_crc = 0x4C492053;
  header.include_offset = include_offset;
  header.animtree_offset = animtree_offset;
  header.bytecode_offset = bytecode_start;
  header.string_offset = string_fixup_offset;
  header.debug_string_offset = file_size;
  header.export_offset = export_offset;
  header.import_offset = import_offset;
  header.fixup_offset = file_size;
  header.profile_offset = file_size;
  header.bytecode_size = total_bytecode_size;
  header.name_offset = name_offset;
  header.string_count = string_count;
  header.export_count = static_cast<uint16_t>(s.exports.size());
  header.import_count = static_cast<uint16_t>(s.imports.size());
  header.fixup_count = 0;
  header.profile_count = 0;
  header.debug_string_count = 0;
  header.include_count = static_cast<uint8_t>(s.includes.size());
  header.animtree_count = 0;
  header.flags = 0;

  std::memcpy(output.data(), &header, sizeof(header));

  return output;
}
} // namespace

emitter_result emit(const ast_ptr &root, const std::string &script_name) {
  emitter_result result{};
  emitter_state state;
  state.script_name = script_name;

  {
    std::string ns_fallback = script_name;
    size_t slash = ns_fallback.find_last_of("/\\");
    if (slash != std::string::npos)
      ns_fallback = ns_fallback.substr(slash + 1);
    size_t dot = ns_fallback.find_last_of('.');
    if (dot != std::string::npos)
      ns_fallback = ns_fallback.substr(0, dot);
    std::transform(
        ns_fallback.begin(), ns_fallback.end(), ns_fallback.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    state.script_namespace = ns_fallback.empty() ? gsc::gsc_hash("ilcustom")
                                                 : gsc::gsc_hash(ns_fallback);
  }
  uint32_t default_namespace = state.script_namespace;

  try {
    // First pass: collect namespace and local function hashes
    for (std::shared_ptr<ast_node> &child : root->children) {
      if (child->type == node_type::n_namespace)
        state.script_namespace = gsc::gsc_hash(child->value);
      else if (child->type == node_type::n_function_def) {
        std::string lower = child->value;
        std::transform(
            lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        const uint8_t param_count =
            child->children.size() > 1
                ? static_cast<uint8_t>(child->children[1]->children.size())
                : 0;
        state.local_function_params[lower] = param_count;
        state.record_hash(lower, child->line, param_count);
      }
    }

    // Reset namespace for second pass
    state.script_namespace = default_namespace;

    // Second pass: process directives and emit functions
    for (std::shared_ptr<ast_node> &child : root->children) {
      if (child->type == node_type::n_namespace) {
        state.script_namespace = gsc::gsc_hash(child->value);
      } else if (child->type == node_type::n_include) {
        state.includes.push_back(child->value);
      } else if (child->type == node_type::n_function_def) {
        if (child->children.size() == 4 &&
            child->children[3]->type == node_type::n_func_ref) {
          emit_function(state, child);
          auto [target_ns, target_fn] = extract_func_ref(child->children[3]);
          if (!target_fn.empty()) {
            std::string replace_ns = state.script_name;
            std::string replace_fn = child->value;
            const int32_t param_count =
                child->children.size() > 1
                    ? static_cast<int>(child->children[1]->children.size())
                    : -1;
            std::transform(replace_fn.begin(), replace_fn.end(),
                           replace_fn.begin(), [](unsigned char c) {
                             return static_cast<char>(std::tolower(c));
                           });
            std::string ts =
                normalize_ns(target_ns.empty() ? state.script_name : target_ns);
            std::string rs = normalize_ns(replace_ns);
            std::string tfn = target_fn;
            std::transform(tfn.begin(), tfn.end(), tfn.begin(),
                           [](unsigned char c) {
                             return static_cast<char>(std::tolower(c));
                           });
            std::function<std::string(std::string script)> strip_script_ext =
                [](std::string script) -> std::string {
              for (char &c : script)
                if (c == '\\')
                  c = '/';
              if (script.size() >= 4 &&
                  (script.substr(script.size() - 4) == ".gsc" ||
                   script.substr(script.size() - 4) == ".csc"))
                script = script.substr(0, script.size() - 4);
              return script;
            };
            std::string target_script = strip_script_ext(ts);
            std::string replace_script = strip_script_ext(rs);
            state.replacefuncs.push_back({target_script, tfn, replace_script,
                                          replace_fn, param_count, param_count,
                                          true});
          }
        } else {
          emit_function(state, child);
        }
      }
    }

    result.data = assemble(state);
    for (const gsc::hash_name_pair &hn : state.hash_names)
      result.hash_names.push_back(
          {hn.hash, std::move(hn.name), hn.line, hn.params});
    result.replacefuncs = std::move(state.replacefuncs);
    result.success = true;
  } catch (const std::runtime_error &e) {
    result.success = false;
    result.error = e.what();
  }

  return result;
}
} // namespace gsc_compiler