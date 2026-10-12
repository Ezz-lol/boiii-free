#include <std_include.hpp>

#include "func.hpp"

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
namespace int64 {
// =====================================================
// Int64 builtins, string-based 64-bit arithmetic
// =====================================================

int64_t parse_int64_arg(scriptInstance_t inst, uint32_t index) {
  const char *str = Scr_GetString(inst, index);
  if (str && str[0])
    return std::strtoll(str, nullptr, 0);
  return static_cast<int64_t>(Scr_GetInt(inst, index));
}

void gscr_int64_op(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 0);
  const char *op = Scr_GetString(inst, 1);
  const int64_t b = parse_int64_arg(inst, 2);

  if (!op) {
    push_string(inst, "0");
    return;
  }

  const std::string op_str(op);
  int64_t result = 0;
  bool is_comparison = false;
  bool cmp_result = false;

  if (op_str == "+")
    result = a + b;
  else if (op_str == "-")
    result = a - b;
  else if (op_str == "*")
    result = a * b;
  else if (op_str == "/") {
    result = (b != 0) ? a / b : 0;
  } else if (op_str == "%") {
    result = (b != 0) ? a % b : 0;
  } else if (op_str == "&")
    result = a & b;
  else if (op_str == "|")
    result = a | b;
  else if (op_str == "^")
    result = a ^ b;
  else if (op_str == "~")
    result = ~a;
  else if (op_str == "<<")
    result = a << b;
  else if (op_str == ">>")
    result = a >> b;
  else if (op_str == "++")
    result = a + 1;
  else if (op_str == "--")
    result = a - 1;
  else if (op_str == ">") {
    is_comparison = true;
    cmp_result = a > b;
  } else if (op_str == ">=") {
    is_comparison = true;
    cmp_result = a >= b;
  } else if (op_str == "<") {
    is_comparison = true;
    cmp_result = a < b;
  } else if (op_str == "<=") {
    is_comparison = true;
    cmp_result = a <= b;
  } else if (op_str == "==") {
    is_comparison = true;
    cmp_result = a == b;
  } else if (op_str == "!=") {
    is_comparison = true;
    cmp_result = a != b;
  } else {
    push_string(inst, "0");
    return;
  }

  if (is_comparison)
    push(inst, cmp_result);
  else
    push_string(inst, std::to_string(result).c_str());
}

void gscr_int64_isint(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 0);
  push(inst, (val >= INT32_MIN && val <= INT32_MAX));
}

void gscr_int64_toint(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 0);
  push(inst, static_cast<int>(val));
}

void gscr_prepareweaponkitassets(scriptInstance_t inst) {
  const char *base_name = Scr_GetString(inst, 0);
  const char *upgrade_name = Scr_GetString(inst, 1);
  if (!base_name || !upgrade_name) {
    push(inst, false);
    return;
  }

  struct context {
    const char *base_name;
    const char *upgrade_name;
    game::weapon::WeaponVariantDef *base{};
    game::weapon::WeaponVariantDef *upgrade{};
  } ctx{base_name, upgrade_name};

  game::db::xasset::DB_EnumXAssets(
      game::db::xasset::XAssetType::WEAPON,
      [](game::db::xasset::XAssetHeader header, void *data) {
        context *ctx = static_cast<context *>(data);
        game::weapon::WeaponVariantDef *variant = header.weapon;
        if (!variant || !variant->szModeIndependentName)
          return;

        if (!strcmp(variant->szModeIndependentName, ctx->base_name))
          ctx->base = variant;
        else if (!strcmp(variant->szModeIndependentName, ctx->upgrade_name))
          ctx->upgrade = variant;
      },
      &ctx, false);

  if (!ctx.base || !ctx.upgrade) {
    push(inst, false);
    return;
  }

  const auto find_camo = [](const std::string &name) {
    const game::db::xasset::XAssetHeader header =
        game::db::xasset::DB_FindXAssetHeader(
            game::db::xasset::XAssetType::WEAPON_CAMO, name.c_str(), false, -1);
    return const_cast<game::weapon::WeaponCamo *>(
        reinterpret_cast<const game::weapon::WeaponCamo *>(header.named));
  };

  game::weapon::WeaponCamo *weapon_camo =
      find_camo("camo_t7_" + std::string(base_name));

  if (weapon_camo && weapon_camo->numCamoMaterials && ctx.base->weapDef &&
      ctx.upgrade->weapDef) {
    ctx.base->weapDef->weaponCamo = weapon_camo;
    ctx.upgrade->weapDef->weaponCamo = weapon_camo;
  }

  ctx.upgrade->szAttachmentUnique = ctx.base->szAttachmentUnique;
  ctx.upgrade->attachments = ctx.base->attachments;
  ctx.upgrade->attachmentUniques = ctx.base->attachmentUniques;
  ctx.upgrade->iAttachments = ctx.base->iAttachments;
  ctx.upgrade->bIgnoreAttachments = ctx.base->bIgnoreAttachments;

  push(inst, true);
}

void gscr_int64_min(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 0);
  const int64_t b = parse_int64_arg(inst, 1);
  push_string(inst, std::to_string(std::min(a, b)).c_str());
}

void gscr_int64_max(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 0);
  const int64_t b = parse_int64_arg(inst, 1);
  push_string(inst, std::to_string(std::max(a, b)).c_str());
}

void gscr_int64_abs(scriptInstance_t inst) {
  const int64_t a = parse_int64_arg(inst, 0);
  push_string(inst, std::to_string(a < 0 ? -a : a).c_str());
}

void gscr_int64_clamp(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 0);
  const int64_t lo = parse_int64_arg(inst, 1);
  const int64_t hi = parse_int64_arg(inst, 2);
  push_string(inst, std::to_string(std::clamp(val, lo, hi)).c_str());
}

void gscr_int64_tostring(scriptInstance_t inst) {
  const int64_t val = parse_int64_arg(inst, 0);
  push_string(inst, std::to_string(val).c_str());
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/int64");

  void post_unpack() override {

    // int64 arithmetic, related utils
    register_builtin("int64_op", gscr_int64_op, 3);
    register_builtin("int64_isint", gscr_int64_isint, 1);
    register_builtin("int64_toint", gscr_int64_toint, 1);
    register_builtin("int64_min", gscr_int64_min, 2);
    register_builtin("int64_max", gscr_int64_max, 2);
    register_builtin("int64_abs", gscr_int64_abs, 1);
    register_builtin("int64_clamp", gscr_int64_clamp, 3);
    register_builtin("int64_tostring", gscr_int64_tostring, 1);
  }
};
} // namespace int64
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::int64::component)
