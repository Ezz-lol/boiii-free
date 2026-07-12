#pragma once

#define DVAR_SYMBOL(name, cl_ofs, sv_ofs)                                      \
  WEAK symbol<const game::dvar_t *> __sym_##name{cl_ofs, sv_ofs};              \
  inline game::EngineDependent<const game::dvar_t_cl *, const game::dvar_t *>  \
  name() {                                                                     \
    if (game::is_server()) {                                                   \
      return *__sym_##name;                                                    \
    }                                                                          \
    return reinterpret_cast<const game::dvar_t_cl *>(*__sym_##name);           \
  }