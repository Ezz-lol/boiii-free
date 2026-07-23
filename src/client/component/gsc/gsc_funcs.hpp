#pragma once

#include <game/game.hpp>

namespace gsc {
using namespace game;
using namespace game::scr;
using namespace game::scr::var;
using namespace game::scr::builtin;

namespace custom_builtins {
template <typename Def> struct CustomBuiltinMap {
  std::unordered_map<ScrVarCanonicalName_t, Def> map;
  std::unordered_map<decltype(Def::actionFunc), ScrVarCanonicalName_t> reverse;
};

extern CustomBuiltinMap<BuiltinFunctionDef> functions;
extern CustomBuiltinMap<BuiltinMethodDef> methods;
} // namespace custom_builtins

inline void register_builtin(BuiltinFunctionDef def) {
  custom_builtins::functions.map[def.canonId] = def;
  custom_builtins::functions.reverse[def.actionFunc] = def.canonId;
}

inline void register_builtin(const char *name, BuiltinFunction func,
                             uint32_t min_args, uint32_t max_args,
                             BuiltinType type) {
  ScrVarCanonicalName_t hash = game::scr::builtin::fnv1a(name);
  BuiltinFunctionDef def = {.canonId = hash,
                            .min_args = min_args,
                            .max_args = max_args,
                            ._padding0C = {0},
                            .actionFunc = func,
                            .type = type,
                            ._padding1C = {0}};

  return register_builtin(def);
}

inline void register_builtin(const char *name, BuiltinFunction func) {
  return register_builtin(name, func, MIN_BUILTIN_ARGS, MAX_BUILTIN_ARGS,
                          DEFAULT_BUILTIN_TYPE);
}
inline void register_builtin(const char *name, BuiltinFunction func,
                             uint32_t min_args, uint32_t max_args) {
  return register_builtin(name, func, min_args, max_args, DEFAULT_BUILTIN_TYPE);
}

inline void register_variadic_builtin(const char *name, BuiltinFunction func,
                                      uint32_t min_args = 1,
                                      BuiltinType type = DEFAULT_BUILTIN_TYPE) {
  return register_builtin(name, func, min_args, MAX_BUILTIN_ARGS, type);
}

// If only min_args is specified, assume this is an absolute required argument
// count (min == max)
inline void register_builtin(const char *name, BuiltinFunction func,
                             uint32_t min_args) {
  return register_builtin(name, func, min_args, min_args, DEFAULT_BUILTIN_TYPE);
}

inline void register_builtin(BuiltinMethodDef def) {
  custom_builtins::methods.map[def.canonId] = def;
  custom_builtins::methods.reverse[def.actionFunc] = def.canonId;
}

template <const IntegralLike auto AliasCount>
inline void register_builtin(const array<const char *, AliasCount> &&aliases,
                             BuiltinFunction func, uint32_t min_args,
                             uint32_t max_args, BuiltinType type) {
  for (size_t aliasIdx = 0; aliasIdx < static_cast<size_t>(AliasCount);
       ++aliasIdx) {
    ScrVarCanonicalName_t hash = game::scr::builtin::fnv1a(aliases[aliasIdx]);
    BuiltinFunctionDef def = {.canonId = hash,
                              .min_args = min_args,
                              .max_args = max_args,
                              ._padding0C = {0},
                              .actionFunc = func,
                              .type = type,
                              ._padding1C = {0}};

    register_builtin(def);
  }
}

template <const IntegralLike auto AliasCount>
inline void register_builtin(const array<const char *, AliasCount> &&aliases,
                             BuiltinFunction func) {
  return register_builtin<AliasCount>(aliases, func, MIN_BUILTIN_ARGS,
                                      MAX_BUILTIN_ARGS, DEFAULT_BUILTIN_TYPE);
}
template <const IntegralLike auto AliasCount>
inline void register_builtin(const array<const char *, AliasCount> &&aliases,
                             BuiltinFunction func, uint32_t min_args,
                             uint32_t max_args) {
  return register_builtin<AliasCount>(aliases, func, min_args, max_args,
                                      DEFAULT_BUILTIN_TYPE);
}

template <const IntegralLike auto AliasCount>
inline void
register_variadic_builtin(const array<const char *, AliasCount> &&aliases,
                          BuiltinFunction func, uint32_t min_args = 1,
                          BuiltinType type = DEFAULT_BUILTIN_TYPE) {
  return register_builtin<AliasCount>(aliases, func, min_args, MAX_BUILTIN_ARGS,
                                      type);
}

// If only min_args is specified, assume this is an absolute required argument
// count (min == max)
template <const IntegralLike auto AliasCount>
inline void register_builtin(const array<const char *, AliasCount> &&aliases,
                             BuiltinFunction func, uint32_t min_args) {
  return register_builtin<AliasCount>(func, min_args, min_args,
                                      DEFAULT_BUILTIN_TYPE);
}

template <const IntegralLike auto AliasCount>
inline void
register_builtin(const std::array<const char *, AliasCount> &&aliases,
                 BuiltinFunction func, uint32_t min_args, uint32_t max_args,
                 BuiltinType type) {
  for (size_t aliasIdx = 0; aliasIdx < static_cast<size_t>(AliasCount);
       ++aliasIdx) {
    ScrVarCanonicalName_t hash = game::scr::builtin::fnv1a(aliases[aliasIdx]);
    BuiltinFunctionDef def = {.canonId = hash,
                              .min_args = min_args,
                              .max_args = max_args,
                              ._padding0C = {0},
                              .actionFunc = func,
                              .type = type,
                              ._padding1C = {0}};

    register_builtin(def);
  }
}

template <const IntegralLike auto AliasCount>
inline void
register_builtin(const std::array<const char *, AliasCount> &&aliases,
                 BuiltinFunction func) {
  return register_builtin<AliasCount>(aliases, func, MIN_BUILTIN_ARGS,
                                      MAX_BUILTIN_ARGS, DEFAULT_BUILTIN_TYPE);
}
template <const IntegralLike auto AliasCount>
inline void
register_builtin(const std::array<const char *, AliasCount> &&aliases,
                 BuiltinFunction func, uint32_t min_args, uint32_t max_args) {
  return register_builtin<AliasCount>(aliases, func, min_args, max_args,
                                      DEFAULT_BUILTIN_TYPE);
}

template <const IntegralLike auto AliasCount>
inline void
register_variadic_builtin(const std::array<const char *, AliasCount> &&aliases,
                          BuiltinFunction func, uint32_t min_args = 1,
                          BuiltinType type = DEFAULT_BUILTIN_TYPE) {
  return register_builtin<AliasCount>(aliases, func, min_args, MAX_BUILTIN_ARGS,
                                      type);
}

// If only min_args is specified, assume this is an absolute required argument
// count (min == max)
template <const IntegralLike auto AliasCount>
inline void
register_builtin(const std::array<const char *, AliasCount> &&aliases,
                 BuiltinFunction func, uint32_t min_args) {
  return register_builtin<AliasCount>(func, min_args, min_args,
                                      DEFAULT_BUILTIN_TYPE);
}

inline void register_builtin(const std::vector<const char *> &&aliases,
                             BuiltinFunction func, uint32_t min_args,
                             uint32_t max_args, BuiltinType type) {
  for (size_t aliasIdx = 0; aliasIdx < aliases.size(); ++aliasIdx) {
    ScrVarCanonicalName_t hash = game::scr::builtin::fnv1a(aliases[aliasIdx]);
    BuiltinFunctionDef def = {.canonId = hash,
                              .min_args = min_args,
                              .max_args = max_args,
                              ._padding0C = {0},
                              .actionFunc = func,
                              .type = type,
                              ._padding1C = {0}};

    register_builtin(def);
  }
}

inline void register_builtin(const std::vector<const char *> &&aliases,
                             BuiltinFunction func) {
  return register_builtin(std::move(aliases), func, MIN_BUILTIN_ARGS,
                          MAX_BUILTIN_ARGS, DEFAULT_BUILTIN_TYPE);
}
inline void register_builtin(const std::vector<const char *> &&aliases,
                             BuiltinFunction func, uint32_t min_args,
                             uint32_t max_args) {
  return register_builtin(std::move(aliases), func, min_args, max_args,
                          DEFAULT_BUILTIN_TYPE);
}
inline void register_variadic_builtin(const std::vector<const char *> &&aliases,
                                      BuiltinFunction func,
                                      uint32_t min_args = 1,
                                      BuiltinType type = DEFAULT_BUILTIN_TYPE) {
  return register_builtin(std::move(aliases), func, min_args, MAX_BUILTIN_ARGS,
                          type);
}

// If only min_args is specified, assume this is an absolute required argument
// count (min == max)
inline void register_builtin(const std::vector<const char *> &&aliases,
                             BuiltinFunction func, uint32_t min_args) {
  return register_builtin(std::move(aliases), func, min_args, min_args,
                          DEFAULT_BUILTIN_TYPE);
}

inline void register_builtin(const char *name, BuiltinMethod method,
                             uint32_t min_args, uint32_t max_args,
                             BuiltinType type) {
  ScrVarCanonicalName_t hash = game::scr::builtin::fnv1a(name);

  BuiltinMethodDef def = {.canonId = hash,
                          .min_args = min_args,
                          .max_args = max_args,
                          ._padding0C = {0},
                          .actionFunc = method,
                          .type = type,
                          ._padding1C = {0}};
  return register_builtin(def);
}

inline void register_builtin(const char *name, BuiltinMethod method) {
  return register_builtin(name, method, MIN_BUILTIN_ARGS, MAX_BUILTIN_ARGS,
                          DEFAULT_BUILTIN_TYPE);
}
inline void register_builtin(const char *name, BuiltinMethod method,
                             uint32_t min_args, uint32_t max_args) {
  return register_builtin(name, method, min_args, max_args,
                          DEFAULT_BUILTIN_TYPE);
}

inline void register_variadic_builtin(const char *name, BuiltinMethod method,
                                      uint32_t min_args = 1,
                                      BuiltinType type = DEFAULT_BUILTIN_TYPE) {
  return register_builtin(name, method, min_args, MAX_BUILTIN_ARGS, type);
}

// If only min_args is specified, assume this is an absolute required argument
// count (min == max)
inline void register_builtin(const char *name, BuiltinMethod method,
                             uint32_t min_args) {
  return register_builtin(name, method, min_args, min_args,
                          DEFAULT_BUILTIN_TYPE);
}

template <const IntegralLike auto AliasCount>
inline void register_builtin(const array<const char *, AliasCount> &&aliases,
                             BuiltinMethod method, uint32_t min_args,
                             uint32_t max_args, BuiltinType type) {
  for (size_t aliasIdx = 0; aliasIdx < static_cast<size_t>(AliasCount);
       ++aliasIdx) {
    ScrVarCanonicalName_t hash = game::scr::builtin::fnv1a(aliases[aliasIdx]);
    BuiltinMethodDef def = {.canonId = hash,
                            .min_args = min_args,
                            .max_args = max_args,
                            ._padding0C = {0},
                            .actionFunc = method,
                            .type = type,
                            ._padding1C = {0}};

    register_builtin(def);
  }
}

template <const IntegralLike auto AliasCount>
inline void register_builtin(const array<const char *, AliasCount> &&aliases,
                             BuiltinMethod method) {
  return register_builtin<AliasCount>(aliases, method, MIN_BUILTIN_ARGS,
                                      MAX_BUILTIN_ARGS, DEFAULT_BUILTIN_TYPE);
}
template <const IntegralLike auto AliasCount>
inline void register_builtin(const array<const char *, AliasCount> &&aliases,
                             BuiltinMethod method, uint32_t min_args,
                             uint32_t max_args) {
  return register_builtin<AliasCount>(aliases, method, min_args, max_args,
                                      DEFAULT_BUILTIN_TYPE);
}

template <const IntegralLike auto AliasCount>
inline void
register_variadic_builtin(const array<const char *, AliasCount> &&aliases,
                          BuiltinMethod method, uint32_t min_args = 1,
                          BuiltinType type = DEFAULT_BUILTIN_TYPE) {
  return register_builtin<AliasCount>(aliases, method, min_args,
                                      MAX_BUILTIN_ARGS, type);
}

// If only min_args is specified, assume this is an absolute required argument
// count (min == max)
template <const IntegralLike auto AliasCount>
inline void register_builtin(const array<const char *, AliasCount> &&aliases,
                             BuiltinMethod method, uint32_t min_args) {
  return register_builtin<AliasCount>(aliases, method, min_args, min_args,
                                      DEFAULT_BUILTIN_TYPE);
}
template <const IntegralLike auto AliasCount>
inline void
register_builtin(const std::array<const char *, AliasCount> &&aliases,
                 BuiltinMethod method, uint32_t min_args, uint32_t max_args,
                 BuiltinType type) {
  for (size_t aliasIdx = 0; aliasIdx < static_cast<size_t>(AliasCount);
       ++aliasIdx) {
    ScrVarCanonicalName_t hash = game::scr::builtin::fnv1a(aliases[aliasIdx]);
    BuiltinMethodDef def = {.canonId = hash,
                            .min_args = min_args,
                            .max_args = max_args,
                            ._padding0C = {0},
                            .actionFunc = method,
                            .type = type,
                            ._padding1C = {0}};

    register_builtin(def);
  }
}

template <const IntegralLike auto AliasCount>
inline void
register_builtin(const std::array<const char *, AliasCount> &&aliases,
                 BuiltinMethod method) {
  return register_builtin<AliasCount>(aliases, method, MIN_BUILTIN_ARGS,
                                      MAX_BUILTIN_ARGS, DEFAULT_BUILTIN_TYPE);
}
template <const IntegralLike auto AliasCount>
inline void
register_builtin(const std::array<const char *, AliasCount> &&aliases,
                 BuiltinMethod method, uint32_t min_args, uint32_t max_args) {
  return register_builtin<AliasCount>(aliases, method, min_args, max_args,
                                      DEFAULT_BUILTIN_TYPE);
}

template <const IntegralLike auto AliasCount>
inline void
register_variadic_builtin(const std::array<const char *, AliasCount> &&aliases,
                          BuiltinMethod method, uint32_t min_args = 1,
                          BuiltinType type = DEFAULT_BUILTIN_TYPE) {
  return register_builtin<AliasCount>(aliases, method, min_args,
                                      MAX_BUILTIN_ARGS, type);
}

// If only min_args is specified, assume this is an absolute required argument
// count (min == max)
template <const IntegralLike auto AliasCount>
inline void
register_builtin(const std::array<const char *, AliasCount> &&aliases,
                 BuiltinMethod method, uint32_t min_args) {
  return register_builtin<AliasCount>(aliases, method, min_args, min_args,
                                      DEFAULT_BUILTIN_TYPE);
}

inline void register_builtin(const std::vector<const char *> &&aliases,
                             BuiltinMethod method, uint32_t min_args,
                             uint32_t max_args, BuiltinType type) {
  for (size_t aliasIdx = 0; aliasIdx < aliases.size(); ++aliasIdx) {
    ScrVarCanonicalName_t hash = game::scr::builtin::fnv1a(aliases[aliasIdx]);
    BuiltinMethodDef def = {.canonId = hash,
                            .min_args = min_args,
                            .max_args = max_args,
                            ._padding0C = {0},
                            .actionFunc = method,
                            .type = type,
                            ._padding1C = {0}};

    register_builtin(def);
  }
}

inline void register_builtin(const std::vector<const char *> &&aliases,
                             BuiltinMethod method) {
  return register_builtin(std::move(aliases), method, MIN_BUILTIN_ARGS,
                          MAX_BUILTIN_ARGS, DEFAULT_BUILTIN_TYPE);
}
inline void register_builtin(const std::vector<const char *> &&aliases,
                             BuiltinMethod method, uint32_t min_args,
                             uint32_t max_args) {
  return register_builtin(std::move(aliases), method, min_args, max_args,
                          DEFAULT_BUILTIN_TYPE);
}

inline void register_variadic_builtin(const std::vector<const char *> &&aliases,
                                      BuiltinMethod method,
                                      uint32_t min_args = 1,
                                      BuiltinType type = DEFAULT_BUILTIN_TYPE) {
  return register_builtin(std::move(aliases), method, min_args,
                          MAX_BUILTIN_ARGS, type);
}

// If only min_args is specified, assume this is an absolute required argument
// count (min == max)
inline void register_builtin(const std::vector<const char *> &&aliases,
                             BuiltinMethod method, uint32_t min_args) {
  return register_builtin(std::move(aliases), method, min_args, min_args,
                          DEFAULT_BUILTIN_TYPE);
}

inline bool custom_builtin_function(ScrVarCanonicalName_t name) {
  return custom_builtins::functions.map.contains(name);
}

inline bool builtin_function(ScrVarCanonicalName_t name) {
  return custom_builtin_function(name) ||
         game::scr::builtin::table::gscr::BuiltinFunctionTable::hashes.contains(
             name)

         // CScr currently unsupported
         // ||
         // game::scr::builtin::table::cscr::BuiltinFunctionTable::hashes.contains(name)
         // ||
         // game::scr::builtin::table::cscr::GfxFunctionTable::hashes.contains(name)
         // ||
         // game::scr::builtin::table::cscr::MathFunctionTable::hashes.contains(name)
         // ||
         // game::scr::builtin::table::cscr::SoundFunctionTable::hashes.contains(name)
         // ||
         // game::scr::builtin::table::cscr::UIFunctionTable::hashes.contains(name)
         // ||
         // game::scr::builtin::table::cscr::UtilFunctionTable::hashes.contains(name)

         || game::scr::builtin::table::CommonFunctionTable::hashes.contains(
                name) ||
         game::scr::builtin::table::MathFunctionTable::hashes.contains(name) ||
         game::scr::builtin::table::SentientFunctionTable::hashes.contains(
             name) ||
         game::scr::builtin::table::UtilFunctionTable::hashes.contains(name)

         || game::scr::builtin::table::bg::CommonFunctionTable::hashes.contains(
                name) ||
         game::scr::builtin::table::bg::MathFunctionTable::hashes.contains(
             name) ||
         game::scr::builtin::table::bg::WeaponFunctionTable::hashes.contains(
             name) ||
         game::scr::builtin::table::bg::UtilFunctionTable::hashes.contains(
             name);
}

inline bool custom_builtin_function(const char *name) {
  return custom_builtin_function(game::scr::builtin::fnv1a(name));
}
inline bool custom_builtin_function(const std::string_view &name) {
  return custom_builtin_function(game::scr::builtin::fnv1a(name.data()));
}

inline bool builtin_function(const char *name) {
  return builtin_function(game::scr::builtin::fnv1a(name));
}
inline bool builtin_function(const std::string_view &name) {
  return builtin_function(game::scr::builtin::fnv1a(name.data()));
}

inline bool custom_builtin_method(ScrVarCanonicalName_t name) {
  return custom_builtins::methods.map.contains(name);
}

inline bool builtin_method(ScrVarCanonicalName_t name) {
  return custom_builtin_method(name) ||
         game::scr::builtin::table::gscr::BuiltinMethodTable::hashes.contains(
             name)
         // CScr currently unsupported
         // ||
         // game::scr::builtin::table::cscr::BuiltinMethodTable::hashes.contains(name)
         // ||
         // game::scr::builtin::table::cscr::GfxMethodTable::hashes.contains(name)
         // ||
         // game::scr::builtin::table::cscr::SoundMethodTable::hashes.contains(name)
         // ||
         // game::scr::builtin::table::cscr::UtilMethodTable::hashes.contains(name)
         ||
         game::scr::builtin::table::ActorInterfaceMethodTable::hashes.contains(
             name) ||
         game::scr::builtin::table::ActorMethodTable::hashes.contains(name) ||
         game::scr::builtin::table::BotMethodTable::hashes.contains(name) ||
         game::scr::builtin::table::PlayerMethodTable::hashes.contains(name) ||
         game::scr::builtin::table::CommonMethodTable::hashes.contains(name) ||
         game::scr::builtin::table::HelicopterMethodTable::hashes.contains(
             name) ||
         game::scr::builtin::table::HudElemMethodTable::hashes.contains(name) ||
         game::scr::builtin::table::ScriptEntMethodTable::hashes.contains(
             name) ||
         game::scr::builtin::table::ScriptVehicleMethodTable::hashes.contains(
             name) ||
         game::scr::builtin::table::SentientMethodTable::hashes.contains(name);
}

inline bool custom_builtin_method(const std::string_view &name) {
  return custom_builtin_method(game::scr::builtin::fnv1a(name.data()));
}
inline bool custom_builtin_method(const char *name) {
  return custom_builtin_method(game::scr::builtin::fnv1a(name));
}

inline bool builtin_method(const std::string_view &name) {
  return builtin_method(game::scr::builtin::fnv1a(name.data()));
}
inline bool builtin_method(const char *name) {
  return builtin_method(game::scr::builtin::fnv1a(name));
}

inline bool custom_builtin(const char *name) {
  return custom_builtin_function(name) || custom_builtin_method(name);
}

inline bool custom_builtin(const std::string_view &name) {
  return custom_builtin_function(name) || custom_builtin_method(name);
}

inline bool custom_builtin(ScrVarCanonicalName_t name) {
  return custom_builtin_function(name) || custom_builtin_method(name);
}

inline bool builtin(const char *name) {
  return builtin_function(name) || builtin_method(name);
}

inline bool builtin(const std::string_view &name) {
  return builtin_function(name) || builtin_method(name);
}

inline bool builtin(ScrVarCanonicalName_t name) {
  return builtin_function(name) || builtin_method(name);
}

inline void push_conststring(scriptInstance_t inst, ScrString_t hash) {
  Scr_AddConstString(inst, hash);
}

inline void push_string(scriptInstance_t inst, const char *val) {
  Scr_AddString(inst, val);
}

inline void push_string(scriptInstance_t inst,
                        const std::filesystem::path &val) {
  const std::string str = val.string();
  Scr_AddString(inst, str.c_str());
}

inline void push_string(scriptInstance_t inst, const std::string_view &val) {
  Scr_AddString(inst, val.data());
}

inline void push_string(scriptInstance_t inst, const std::string &val) {
  Scr_AddString(inst, val.c_str());
}

inline void push(scriptInstance_t inst, const char *val) {
  return push_string(inst, val);
}

inline void push(scriptInstance_t inst, const std::filesystem::path &val) {
  return push_string(inst, std::move(val));
}

inline void push(scriptInstance_t inst, const std::string_view &val) {
  return push_string(inst, std::move(val));
}

inline void push(scriptInstance_t inst, const std::string &val) {
  return push_string(inst, std::move(val));
}

template <IntegralLike<int32_t> Prim>
inline void push_int(scriptInstance_t inst, Prim val) {
  Scr_AddInt(inst, static_cast<int32_t>(val));
}

inline void push(scriptInstance_t inst, bool val) { push_int(inst, val); }
inline void push(scriptInstance_t inst, uint32_t val) { push_int(inst, val); }
inline void push(scriptInstance_t inst, qboolean val) { push_int(inst, val); }
inline void push(scriptInstance_t inst, int32_t val) { push_int(inst, val); }
inline void push(scriptInstance_t inst, uint64_t val) { push_int(inst, val); }
inline void push(scriptInstance_t inst, int64_t val) { push_int(inst, val); }

inline void push(scriptInstance_t inst, std::vector<std::string> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    const char *str = arr[i].c_str();
    Scr_AddString(inst, str);
    Scr_AddArray(inst);
  }
}

inline void push(scriptInstance_t inst, std::vector<std::string_view> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    const char *str = arr[i].data();
    Scr_AddString(inst, str);
    Scr_AddArray(inst);
  }
}

inline void push(scriptInstance_t inst,
                 std::vector<std::filesystem::path> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    const std::string str = arr[i].string();
    const char *cstr = str.c_str();
    Scr_AddString(inst, cstr);
    Scr_AddArray(inst);
  }
}

inline void push(scriptInstance_t inst, std::vector<float> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    Scr_AddFloat(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

inline void push(scriptInstance_t inst, std::vector<const char *> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    Scr_AddString(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

inline void push(scriptInstance_t inst, std::vector<bool> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    Scr_AddInt(inst, static_cast<qboolean>((arr[i])));
    Scr_AddArray(inst);
  }
}

inline void push(scriptInstance_t inst, std::vector<uint32_t> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    Scr_AddInt(inst, static_cast<int32_t>(arr[i]));
    Scr_AddArray(inst);
  }
}

inline void push(scriptInstance_t inst, std::vector<int32_t> &&arr) {
  Scr_MakeArray(inst);
  for (size_t i = 0; i < arr.size(); ++i) {
    Scr_AddInt(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

template <IntegralLike auto ArraySize>
inline void push(scriptInstance_t inst,
                 const array<std::string, ArraySize> &&arr) {
  Scr_MakeArray(inst);
  for (uint64_t i = 0; i < static_cast<uint64_t>(ArraySize); ++i) {
    const char *str = arr[i].c_str();
    Scr_AddString(inst, str);
    Scr_AddArray(inst);
  }
}

template <IntegralLike auto ArraySize>
inline void push(scriptInstance_t inst,
                 const array<std::string_view, ArraySize> &&arr) {
  Scr_MakeArray(inst);
  for (uint64_t i = 0; i < static_cast<uint64_t>(ArraySize); ++i) {
    const char *str = arr[i].data();
    Scr_AddString(inst, str);
    Scr_AddArray(inst);
  }
}

template <IntegralLike auto ArraySize>
inline void push(scriptInstance_t inst,
                 const array<std::filesystem::path, ArraySize> &&arr) {
  Scr_MakeArray(inst);
  for (uint64_t i = 0; i < static_cast<uint64_t>(ArraySize); ++i) {
    const std::string str = arr[i].string();
    const char *cstr = str.c_str();
    Scr_AddString(inst, cstr);
    Scr_AddArray(inst);
  }
}

template <IntegralLike auto ArraySize>
inline void push(scriptInstance_t inst, const array<float, ArraySize> &&arr) {
  Scr_MakeArray(inst);
  for (uint64_t i = 0; i < static_cast<uint64_t>(ArraySize); ++i) {
    Scr_AddFloat(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

template <IntegralLike auto ArraySize>
inline void push(scriptInstance_t inst,
                 const array<const char *, ArraySize> &&arr) {
  Scr_MakeArray(inst);
  for (uint64_t i = 0; i < static_cast<uint64_t>(ArraySize); ++i) {
    Scr_AddString(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

template <IntegralLike auto ArraySize>
inline void push(scriptInstance_t inst, const array<bool, ArraySize> &&arr) {
  Scr_MakeArray(inst);
  for (uint64_t i = 0; i < static_cast<uint64_t>(ArraySize); ++i) {
    Scr_AddInt(inst, static_cast<qboolean>((arr[i])));
    Scr_AddArray(inst);
  }
}

template <IntegralLike auto ArraySize>
inline void push(scriptInstance_t inst,
                 const array<uint32_t, ArraySize> &&arr) {
  Scr_MakeArray(inst);
  for (uint64_t i = 0; i < static_cast<uint64_t>(ArraySize); ++i) {
    Scr_AddInt(inst, static_cast<int32_t>(arr[i]));
    Scr_AddArray(inst);
  }
}

template <IntegralLike auto ArraySize>
inline void push(scriptInstance_t inst, const array<int32_t, ArraySize> &&arr) {
  Scr_MakeArray(inst);
  for (uint64_t i = 0; i < static_cast<uint64_t>(ArraySize); ++i) {
    Scr_AddInt(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

// Push empty array
inline void push(scriptInstance_t inst) { Scr_MakeArray(inst); }

template <SizedIntegralLike<int32_t> Prim,
          // Ensure specific overrides remain respected
          typename std::enable_if<
              !std::disjunction<
                  std::is_same<Prim, int32_t>, std::is_same<Prim, uint32_t>,
                  std::is_same<Prim, bool>, std::is_same<Prim, qboolean>,
                  std::is_same<Prim, int64_t>, std::is_same<Prim, uint64_t>,
                  std::is_array<Prim>>::value,
              int>::type = 0>
inline void push(scriptInstance_t inst, Prim val) {
  return push_int(inst, val);
}

template <SizedIntegralLike<int32_t> Prim,
          // Ensure specific overrides remain respected
          typename std::enable_if<
              !std::disjunction<
                  std::is_same<Prim, int32_t>, std::is_same<Prim, uint32_t>,
                  std::is_same<Prim, bool>, std::is_same<Prim, qboolean>,
                  std::is_same<Prim, int64_t>, std::is_same<Prim, uint64_t>,
                  std::is_array<Prim>>::value,
              int>::type = 0,
          IntegralLike auto ArraySize>
inline void push(scriptInstance_t inst, const array<Prim, ArraySize> &&arr) {
  Scr_MakeArray(inst);
  for (uint64_t i = 0; i < static_cast<uint64_t>(ArraySize); ++i) {
    push_int(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

template <SizedIntegralLike<int32_t> Prim,
          // Ensure specific overrides remain respected
          typename std::enable_if<
              !std::disjunction<
                  std::is_same<Prim, int32_t>, std::is_same<Prim, uint32_t>,
                  std::is_same<Prim, bool>, std::is_same<Prim, qboolean>,
                  std::is_same<Prim, int64_t>, std::is_same<Prim, uint64_t>,
                  std::is_array<Prim>>::value,
              int>::type = 0>
inline void push(scriptInstance_t inst, const std::vector<Prim> &&arr) {
  Scr_MakeArray(inst);
  for (uint64_t i = 0; i < arr.size(); ++i) {
    push_int(inst, arr[i]);
    Scr_AddArray(inst);
  }
}

} // namespace gsc