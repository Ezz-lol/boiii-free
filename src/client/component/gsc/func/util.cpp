#include <std_include.hpp>

#include "func.hpp"

#include <atomic>

#include <game/impl/scr/scr.hpp>
#include <game/impl/scr/var.hpp>
#include <game/impl/sv/sv.hpp>
#include <game/utils.hpp>

#include <component/command.hpp>
#include <component/game_event.hpp>
#include <component/name.hpp>
#include <component/toast.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace func {
namespace util {
void gscr_conststring(scriptInstance_t inst) {
  const uint32_t argc = Scr_GetNumParam(inst);
  if (argc == 0) {
    Scr_ParamError(inst, 0, "No hash argument provided to conststring.");
  } else {
    const ScrString_t hash = static_cast<ScrString_t>(Scr_GetInt(inst, 0));
    push_conststring(inst, hash);
  }
}

void gscr_isstruct(scriptInstance_t inst) {
  const uint32_t argc = Scr_GetNumParam(inst);
  if (argc == 0) {
    Scr_ParamError(inst, 0,
                   "No argument provided to isstruct. syntax: isstruct(var)");
  } else {
    push(inst, Scr_GetType(inst, 0) == var::ScrVarType::POINTER &&
                   Scr_GetPointerType(inst, 0) == var::ScrVarType::STRUCT);
  }
}

void gscr_typename(scriptInstance_t inst) {
  const uint32_t argc = Scr_GetNumParam(inst);
  if (argc == 0) {
    Scr_ParamError(inst, 0,
                   "No argument provided to typename. syntax: typename(var)");
  } else {
    const ScrVarType var_type = Scr_GetValue(inst, 0)->type;
    const char *name = var_typename->valid_index(var_type)
                           ? var_typename->get(var_type)
                           : "INVALID";

    push(inst, name);
  }
}

void gscr_ismenucached(scriptInstance_t inst) {
  const uint32_t argc = Scr_GetNumParam(inst);
  if (argc == 0) {
    Scr_ParamError(inst, 0,
                   "No argument provided to ismenucached. syntax: "
                   "ismenucached(\"MenuName\")");
  } else {
    const var::ScrVarType_t type = Scr_GetType(inst, 0);
    if (var::ScrVar_StringLike(type)) {
      using namespace game::bg::cache;
      push(inst,
           BG_Cache_IsCachedScriptMenuIndex(static_cast<bgCacheInstance>(inst),
                                            Scr_GetString(inst, 0)));
    } else {
      Scr_ParamError(
          inst, 0,
          "Argument of type %s provided to ismenucached is not a string or "
          "localized string.",
          Scr_TypeName(type));
    }
  }
}

void gscr_vector(scriptInstance_t inst) {
  const uint32_t argc = Scr_GetNumParam(inst);
  vec3_t result = vec3_t::fill(std::numeric_limits<float>::quiet_NaN());
  if (argc > 0) {

    if (argc == 1) {
      switch (Scr_GetType(inst, 0)) {

      case ScrVarType::VECTOR: {
        Scr_GetVector(inst, 0, &result);
        break;
      }

      case ScrVarType::POINTER: {
        const std::vector<volatile var::ScrVarValue_t *> arg =
            Scr_GetArray(inst, 0);

        for (size_t i = 0; i < std::min<size_t>(arg.size(), result.size());
             ++i) {
          result[i] = ScrVar_CastFloat(arg[i]);
        }
        break;
      }
      default: {
        result.x = ScrVar_CastFloat(Scr_GetValue(inst, 0));
        break;
      }
      }
    } else {

      result.x = ScrVar_CastFloat(Scr_GetValue(inst, 0));

      if (argc > 1) {
        result.y = ScrVar_CastFloat(Scr_GetValue(inst, 1));
      }

      if (argc > 2) {
        result.z = ScrVar_CastFloat(Scr_GetValue(inst, 2));
      }
    }
  }

  push(inst, &result);
}

namespace gscr_clientindex {
inline void impl(scriptInstance_t inst, const level::gentity_t *ent) {
  if (ent != nullptr) {
    const ClientNum_t clientNum =
        static_cast<ClientNum_t>(level::entity_index(ent));
    if (valid_client_num(clientNum)) {
      push(inst, clientNum);
    }
  }

  push(inst, game::INVALID_CLIENT_INDEX);
}

void func(scriptInstance_t inst) { return impl(inst, Scr_GetEntity_Impl(0)); }
void method(scriptInstance_t inst, scr_entref_t *entref) {
  return impl(inst, level::entity(entref->u.entnum));
}
} // namespace gscr_clientindex

static std::atomic_bool exitlevel_restrict = true;
void gscr_exitlevel_togglerestrict(scriptInstance_t inst) {
  const bool current_value = exitlevel_restrict.load(std::memory_order_acquire);
  const bool new_value = static_cast<bool>(
      Scr_GetIntOptional(inst, 0, qboolean::from(!current_value)));
  exitlevel_restrict.store(new_value, std::memory_order_release);
  Scr_AddInt(inst, qboolean::from(current_value));
}

template <const BuiltinFunction &Orig>
void GScr_ExitLevel_RespectRestriction(scriptInstance_t inst) {
  if (exitlevel_restrict.load(std::memory_order_acquire)) {
    Orig(inst);
  }
}

void apply_exitlevel_hooks() {
  register_builtin(SCRIPTINSTANCE_SERVER, "exitlevel_togglerestrict",
                   gscr_exitlevel_togglerestrict, 0, 1);
  static const BuiltinFunction GScr_ExitLevel_orig =
      game::scr::builtin::table::gscr::builtin_functions->ExitLevel.actionFunc;
  game::scr::builtin::table::gscr::builtin_functions->ExitLevel.actionFunc =
      &GScr_ExitLevel_RespectRestriction<GScr_ExitLevel_orig>;
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/util");

  void post_unpack() override {
    // Misc. utils
    register_builtin("conststring", gscr_conststring, 1);
    register_builtin("isstruct", gscr_isstruct, 1);
    register_builtin("typename", gscr_typename, 1);
    register_builtin("ismenucached", gscr_ismenucached, 1);
    register_builtin("vector", gscr_vector, 0, 3);
    register_builtin(SCRIPTINSTANCE_SERVER, "clientindex",
                     gscr_clientindex::func, 1);
    register_builtin(SCRIPTINSTANCE_SERVER, "clientindex",
                     gscr_clientindex::method, 0);
    apply_exitlevel_hooks();
  }
};
} // namespace util
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::util::component)
