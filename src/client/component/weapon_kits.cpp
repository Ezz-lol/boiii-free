#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include "weapon_kits.hpp"

#include <utils/hook.hpp>

namespace weapon_kits {
namespace {
using game::ddl::StringTable;
using game::ddl::StringTableCell;

constexpr const char *ZM_STATS_TABLE = "gamedata/stats/zm/zm_statstable.csv";
constexpr const char *MP_STATS_TABLE = "gamedata/stats/mp/mp_statstable.csv";
constexpr std::string_view ZM_WEAPONS_TABLE_PREFIX = "gamedata/weapons/zm/";
constexpr std::string_view ZM_WEAPONS_TABLE_SUFFIX = "_weapons.csv";

constexpr int32_t COLUMN_INDEX = 0;
constexpr int32_t COLUMN_REF = 4;
constexpr int32_t COLUMN_IMAGE = 6;
constexpr int32_t COLUMN_DESCRIPTION = 7;
constexpr int32_t COLUMN_UNLOCK_LEVEL = 10;
constexpr int32_t COLUMN_ALLOCATION = 12;
constexpr int32_t COLUMN_LOADOUT_SLOT = 13;
constexpr int32_t COLUMN_UNLOCK_PRESTIGE = 14;

constexpr int32_t COLUMN_WEAPON_NAME = 0;
constexpr std::string_view FORCE_ATTACHMENTS_HEADER = "force_attachments";
constexpr std::string_view DUAL_WIELD_SUFFIX = "_dw";

constexpr size_t WEAPON_DEF_ITEM_INDEX = 0x2A8;
constexpr size_t WEAPON_DEF_PARENT_WEAPON_NAME = 0x2B0;
constexpr std::string_view UPGRADED_SUFFIX = "_upgraded";
constexpr size_t ATTACHMENT_SLOTS = 64;

constexpr std::string_view KIT_WEAPONS[] = {
    "smg_ak74u",      "smg_mp40",
    "smg_ppsh",       "smg_longrange",
    "ar_peacekeeper", "ar_garand",
    "ar_famas",       "ar_m16",
    "ar_m14",         "lmg_rpk",
    "shotgun_energy", "pistol_energy",
    "pistol_m1911",   "pistol_standard",
    "launcher_multi", "special_crossbow_dw"};

struct patched_table {
  std::vector<StringTableCell> values;
  std::vector<int16_t> cell_index;
};

std::mutex patch_mutex;
std::unordered_map<const StringTable *, patched_table> patched_tables;

utils::hook::detour bg_unlockables_load_items_hook;
utils::hook::detour cg_register_weapon_hook;
std::unordered_map<
    const game::weapon::WeaponVariantDef *,
    std::array<game::weapon::WeaponAttachmentPtr, ATTACHMENT_SLOTS>>
    filled_attachments;

StringTable *find_table(const char *name) {
  return game::db::xasset::DB_FindXAssetHeader(
             game::db::xasset::XAssetType::STRINGTABLE, name, false, 0)
      .stringTable;
}

const StringTableCell &cell(const StringTable *table, const int32_t row,
                            const int32_t column) {
  return table->values[row * table->columnCount + column];
}

std::string_view cell_text(const StringTable *table, const int32_t row,
                           const int32_t column) {
  const char *text = cell(table, row, column).string;
  return text ? text : "";
}

std::optional<int32_t> find_row(const StringTable *table, const int32_t column,
                                const std::string_view &value) {
  for (int32_t row = 0; row < table->rowCount; ++row) {
    if (cell_text(table, row, column) == value) {
      return row;
    }
  }
  return std::nullopt;
}

std::optional<int32_t> find_column(const StringTable *table,
                                   const std::string_view &header) {
  for (int32_t column = 0; table->rowCount && column < table->columnCount;
       ++column) {
    if (cell_text(table, 0, column) == header) {
      return column;
    }
  }
  return std::nullopt;
}

bool is_kit_weapon(const std::string_view &name) {
  return std::ranges::find(KIT_WEAPONS, name) != std::end(KIT_WEAPONS);
}

bool is_patched(const StringTable *table) {
  const auto it = patched_tables.find(table);
  return it != patched_tables.end() &&
         it->second.values.data() == table->values;
}

std::vector<StringTableCell> copy_values(const StringTable *table) {
  return {table->values, table->values + table->rowCount * table->columnCount};
}

void replace_values(StringTable *table, std::vector<StringTableCell> values) {
  if (values.size() >
      static_cast<size_t>(std::numeric_limits<int16_t>::max())) {
    return;
  }

  patched_table patched{std::move(values), {}};
  patched.cell_index.resize(patched.values.size());
  std::iota(patched.cell_index.begin(), patched.cell_index.end(), int16_t{0});
  std::ranges::stable_sort(
      patched.cell_index, [&](const int16_t left, const int16_t right) {
        return patched.values[left].hash < patched.values[right].hash;
      });

  patched_table &stored = patched_tables[table] = std::move(patched);
  table->values = stored.values.data();
  table->cellIndex = stored.cell_index.data();
  table->rowCount =
      static_cast<int32_t>(stored.values.size() / table->columnCount);
}

void patch_zombies_stats_table(StringTable *zm, const StringTable *mp) {
  if (!zm || !mp || is_patched(zm) ||
      zm->columnCount <= COLUMN_UNLOCK_PRESTIGE ||
      mp->columnCount <= COLUMN_UNLOCK_PRESTIGE) {
    return;
  }

  const std::optional<int32_t> customizable =
      find_row(zm, COLUMN_ALLOCATION, "1");
  const std::optional<int32_t> unlocked =
      find_row(zm, COLUMN_UNLOCK_LEVEL, "0");
  const std::optional<int32_t> no_prestige =
      find_row(zm, COLUMN_UNLOCK_PRESTIGE, "");
  if (!customizable || !unlocked || !no_prestige) {
    return;
  }
  const StringTableCell allocation = cell(zm, *customizable, COLUMN_ALLOCATION);
  const StringTableCell unlock_level = cell(zm, *unlocked, COLUMN_UNLOCK_LEVEL);
  const StringTableCell empty_cell =
      cell(zm, *no_prestige, COLUMN_UNLOCK_PRESTIGE);

  std::vector<StringTableCell> values = copy_values(zm);
  const auto make_available = [&](const size_t start, const int32_t source) {
    values[start + COLUMN_UNLOCK_LEVEL] = unlock_level;
    values[start + COLUMN_ALLOCATION] = allocation;
    values[start + COLUMN_LOADOUT_SLOT] = cell(mp, source, COLUMN_LOADOUT_SLOT);
    values[start + COLUMN_UNLOCK_PRESTIGE] = empty_cell;
  };

  for (const std::string_view &ref : KIT_WEAPONS) {
    const std::optional<int32_t> source = find_row(mp, COLUMN_REF, ref);
    if (!source) {
      continue;
    }
    const std::string_view index = cell_text(mp, *source, COLUMN_INDEX);
    const bool index_taken = find_row(zm, COLUMN_INDEX, index).has_value();

    if (const std::optional<int32_t> row = find_row(zm, COLUMN_REF, ref)) {
      make_available(*row * zm->columnCount, *source);
      continue;
    }

    if (index_taken) {
      continue;
    }

    const size_t start = values.size();
    values.resize(start + zm->columnCount, empty_cell);
    for (int32_t column = 0;
         column < std::min(zm->columnCount, mp->columnCount); ++column) {
      values[start + column] = cell(mp, *source, column);
    }
    if (ref.ends_with(DUAL_WIELD_SUFFIX)) {
      if (const std::optional<int32_t> single =
              find_row(mp, COLUMN_REF,
                       ref.substr(0, ref.size() - DUAL_WIELD_SUFFIX.size()))) {
        values[start + COLUMN_IMAGE] = cell(mp, *single, COLUMN_IMAGE);
        values[start + COLUMN_DESCRIPTION] =
            cell(mp, *single, COLUMN_DESCRIPTION);
      }
    }
    make_available(start, *source);
  }

  replace_values(zm, std::move(values));
}

void remove_forced_kit_attachments(StringTable *weapons) {
  if (!weapons || !weapons->name || is_patched(weapons)) {
    return;
  }
  const std::string_view name = weapons->name;
  if (!name.starts_with(ZM_WEAPONS_TABLE_PREFIX) ||
      !name.ends_with(ZM_WEAPONS_TABLE_SUFFIX)) {
    return;
  }

  const std::optional<int32_t> column =
      find_column(weapons, FORCE_ATTACHMENTS_HEADER);
  const std::optional<int32_t> empty =
      column ? find_row(weapons, *column, "") : std::nullopt;
  if (!column || !empty) {
    return;
  }

  std::vector<StringTableCell> values = copy_values(weapons);
  bool changed = false;
  for (int32_t row = 1; row < weapons->rowCount; ++row) {
    if (is_kit_weapon(cell_text(weapons, row, COLUMN_WEAPON_NAME)) &&
        !cell_text(weapons, row, *column).empty()) {
      values[row * weapons->columnCount + *column] =
          cell(weapons, *empty, *column);
      changed = true;
    }
  }
  if (changed) {
    replace_values(weapons, std::move(values));
  }
}

void bg_unlockables_load_items_stub(const game::eModes mode) {
  if (mode == game::eModes::ZOMBIES) {
    std::scoped_lock lock(patch_mutex);
    patch_zombies_stats_table(find_table(ZM_STATS_TABLE),
                              find_table(MP_STATS_TABLE));
  }
  bg_unlockables_load_items_hook.invoke(mode);
}

bool has_missing_attachment(const game::weapon::WeaponVariantDef *variant,
                            const game::weapon::Weapon weapon) {
  for (const uint64_t id :
       {weapon.attachment1, weapon.attachment2, weapon.attachment3,
        weapon.attachment4, weapon.attachment5, weapon.attachment6,
        weapon.attachment7, weapon.attachment8}) {
    if (id && (!variant->attachments || !variant->attachments[id])) {
      return true;
    }
  }
  return false;
}

game::weapon::WeaponVariantDef *find_variant(const std::string &name) {
  struct search {
    const std::string *name;
    game::weapon::WeaponVariantDef *found;
  } context{&name, nullptr};
  game::db::xasset::DB_EnumXAssets(
      game::db::xasset::XAssetType::WEAPON,
      [](const game::db::xasset::XAssetHeader header, void *data) {
        auto *context = static_cast<search *>(data);
        game::weapon::WeaponVariantDef *variant = header.weapon;
        if (!context->found && variant && variant->szInternalName &&
            *context->name == variant->szInternalName) {
          context->found = variant;
        }
      },
      &context, false);
  return context.found;
}

bool covers_attachments(const game::weapon::WeaponVariantDef *base,
                        const game::weapon::WeaponVariantDef *upgraded) {
  if (!base->attachments || !base->attachmentUniques) {
    return false;
  }
  for (size_t id = 0; upgraded->attachments && id < ATTACHMENT_SLOTS; ++id) {
    if (upgraded->attachments[id] && !base->attachments[id]) {
      return false;
    }
  }
  return true;
}

void share_base_attachments(game::weapon::WeaponVariantDef *upgraded) {
  std::string base_name = upgraded->szInternalName;
  const size_t suffix = base_name.find(UPGRADED_SUFFIX);
  if (suffix == std::string::npos) {
    return;
  }
  base_name.erase(suffix, UPGRADED_SUFFIX.size());

  const game::weapon::WeaponVariantDef *base = find_variant(base_name);
  if (!base || !covers_attachments(base, upgraded)) {
    return;
  }
  upgraded->szAttachmentUnique = base->szAttachmentUnique;
  upgraded->attachments = base->attachments;
  upgraded->attachmentUniques = base->attachmentUniques;
  upgraded->iAttachments = base->iAttachments;
  upgraded->bIgnoreAttachments = base->bIgnoreAttachments;
}

game::weapon::WeaponAttachmentPtr find_attachment(const uint64_t id) {
  struct search {
    uint64_t id;
    game::weapon::WeaponAttachmentPtr found;
  } context{id, nullptr};
  game::db::xasset::DB_EnumXAssets(
      game::db::xasset::XAssetType::WEAPON,
      [](const game::db::xasset::XAssetHeader header, void *data) {
        auto *context = static_cast<search *>(data);
        const game::weapon::WeaponVariantDef *variant = header.weapon;
        if (!context->found && variant && variant->attachments) {
          context->found = variant->attachments[context->id];
        }
      },
      &context, false);
  return context.found;
}

void fill_missing_attachments(game::weapon::WeaponVariantDef *variant,
                              const game::weapon::Weapon weapon) {
  if (!variant->attachments || !variant->attachments[0]) {
    return;
  }
  std::array<game::weapon::WeaponAttachmentPtr, ATTACHMENT_SLOTS> &table =
      filled_attachments[variant];
  if (variant->attachments != table.data()) {
    std::copy_n(variant->attachments, ATTACHMENT_SLOTS, table.begin());
  }
  for (const uint64_t id :
       {weapon.attachment1, weapon.attachment2, weapon.attachment3,
        weapon.attachment4, weapon.attachment5, weapon.attachment6,
        weapon.attachment7, weapon.attachment8}) {
    if (id && !table[id]) {
      table[id] = find_attachment(id);
      if (!table[id]) {
        table[id] = table[0];
      }
    }
  }
  variant->attachments = table.data();
}

void ensure_attachments(const game::weapon::Weapon weapon) {
  game::weapon::WeaponVariantDef *variant =
      game::bg::BG_GetWeaponVariantDef(weapon);
  if (!variant || !variant->szInternalName ||
      !has_missing_attachment(variant, weapon)) {
    return;
  }
  share_base_attachments(variant);
  if (has_missing_attachment(variant, weapon)) {
    fill_missing_attachments(variant, weapon);
  }
}

void cg_register_weapon_stub(const game::weapon::Weapon weapon, void *arg2,
                             void *arg3) {
  ensure_attachments(weapon);
  cg_register_weapon_hook.invoke(weapon, arg2, arg3);
}

void resolve_item_index(const game::weapon::Weapon weapon) {
  auto *def = reinterpret_cast<uint8_t *>(game::bg::BG_GetWeaponDef(weapon));
  if (!def) {
    return;
  }
  auto &item_index = *reinterpret_cast<int32_t *>(def + WEAPON_DEF_ITEM_INDEX);
  const char *parent =
      *reinterpret_cast<const char **>(def + WEAPON_DEF_PARENT_WEAPON_NAME);
  if (item_index > 0 || !parent || !*parent) {
    return;
  }
  const int32_t index =
      game::bg::BG_UnlockablesGetItemIndexByName(game::eModes::ZOMBIES, parent);
  if (index > 0) {
    item_index = index;
  }
}

template <game::scr::builtin::BuiltinMethod *Original>
void build_kit_method_stub(const game::scr::scriptInstance_t inst,
                           game::scr::scr_entref_t *entref) {
  if (game::com::Com_SessionMode_IsMode(game::eModes::ZOMBIES) &&
      game::scr::Scr_GetNumParam(inst) > 0) {
    resolve_item_index(game::scr::Scr_GetWeapon(inst, 0));
  }
  (*Original)(inst, entref);
}

game::scr::builtin::BuiltinMethod get_build_kit_weapon;
game::scr::builtin::BuiltinMethod get_build_kit_weapon_options;
game::scr::builtin::BuiltinMethod get_build_kit_attachment_cosmetic_variants;

void replace_build_kit_method(game::scr::builtin::BuiltinMethodDef &method,
                              game::scr::builtin::BuiltinMethod &original,
                              const game::scr::builtin::BuiltinMethod stub) {
  original = method.actionFunc;
  method.actionFunc = stub;
}
} // namespace

void on_asset_linked(const game::db::xasset::XAssetEntry *entry) {
  if (entry && entry->asset.type == game::db::xasset::XAssetType::STRINGTABLE) {
    std::scoped_lock lock(patch_mutex);
    remove_forced_kit_attachments(entry->asset.header.stringTable);
  }
}

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("weapon_kits");

public:
  void post_unpack() override {
    if (game::is_client()) {
      cg_register_weapon_hook.create(game::cg::CG_RegisterWeapon.get(),
                                     cg_register_weapon_stub);
    }
    bg_unlockables_load_items_hook.create(
        game::bg::BG_UnlockablesLoadItems.get(),
        bg_unlockables_load_items_stub);

    auto &methods = *game::scr::builtin::table::gscr::builtin_methods;
    replace_build_kit_method(methods.GetBuildKitWeapon, get_build_kit_weapon,
                             build_kit_method_stub<&get_build_kit_weapon>);
    replace_build_kit_method(
        methods.GetBuildKitWeaponOptions, get_build_kit_weapon_options,
        build_kit_method_stub<&get_build_kit_weapon_options>);
    replace_build_kit_method(
        methods.GetBuildKitAttachmentCosmeticVariantIndexes,
        get_build_kit_attachment_cosmetic_variants,
        build_kit_method_stub<&get_build_kit_attachment_cosmetic_variants>);
  }
};
} // namespace weapon_kits

REGISTER_COMPONENT(weapon_kits::component)
