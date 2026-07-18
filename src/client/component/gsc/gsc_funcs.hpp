#pragma once

#include <game/game.hpp>

namespace script {
using namespace game;
using namespace game::scr;
void register_builtin(const char *name, BuiltinFunctionDef def);
void register_builtin(const char *name, BuiltinFunction func,
                      BuiltinType type = DEFAULT_BUILTIN_TYPE,
                      uint32_t min_args = MIN_BUILTIN_ARGS,
                      uint32_t max_args = MAX_BUILTIN_ARGS);
void register_builtin(const char *name, BuiltinMethodDef def);
void register_builtin(const char *name, BuiltinMethod method,
                      BuiltinType type = DEFAULT_BUILTIN_TYPE,
                      uint32_t min_args = MIN_BUILTIN_ARGS,
                      uint32_t max_args = MAX_BUILTIN_ARGS);

bool custom_builtin_function(ScrVarCanonicalName_t name);
bool custom_builtin_function(const char *name);

bool custom_builtin_method(ScrVarCanonicalName_t name);
bool custom_builtin_method(const char *name);

inline bool custom_builtin(const char *name) {
  return custom_builtin_function(name) || custom_builtin_method(name);
}

inline bool custom_builtin(ScrVarCanonicalName_t name) {
  return custom_builtin_function(name) || custom_builtin_method(name);
}

inline void push_string(scriptInstance_t inst, const char *val) {
  Scr_AddString(inst, val);
}

inline void push_conststring(scriptInstance_t inst, ScrString_t hash) {
  Scr_AddConstString(inst, hash);
}

template <typename T, typename = typename std::enable_if<
                          std::is_convertible<T, int32_t>::value>::type>
inline void push_int(scriptInstance_t inst, T val) {
  Scr_AddInt(inst, static_cast<int32_t>(val));
}

inline void push_array(scriptInstance_t inst, std::vector<std::string> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    const char *str = arr[i].c_str();
    Scr_AddString(inst, str);
    Scr_AddArray(inst);
  }
}

inline void push_array(scriptInstance_t inst, std::vector<float> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    Scr_AddFloat(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

inline void push_array(scriptInstance_t inst, std::vector<const char *> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    Scr_AddString(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

inline void push_array(scriptInstance_t inst, std::vector<bool> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    Scr_AddInt(inst, static_cast<qboolean>((arr[i])));
    Scr_AddArray(inst);
  }
}

inline void push_array(scriptInstance_t inst, std::vector<uint32_t> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    Scr_AddInt(inst, static_cast<int32_t>(arr[i]));
    Scr_AddArray(inst);
  }
}

inline void push_array(scriptInstance_t inst, std::vector<int32_t> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    Scr_AddInt(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

// Push empty array
inline void push_array(scriptInstance_t inst) { Scr_MakeArray(inst); }

} // namespace script