#include <std_include.hpp>

#include <loader/component_loader.hpp>

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
constexpr int32_t COLUMN_UNLOCK_LEVEL = 10;
constexpr int32_t COLUMN_ALLOCATION = 12;
constexpr int32_t COLUMN_LOADOUT_SLOT = 13;
constexpr int32_t COLUMN_UNLOCK_PRESTIGE = 14;

constexpr int32_t COLUMN_WEAPON_NAME = 0;
constexpr std::string_view FORCE_ATTACHMENTS_HEADER = "force_attachments";

constexpr std::string_view KIT_WEAPONS[] = {
    "smg_ak74u",      "smg_mp40",        "smg_ppsh",       "smg_longrange",
    "ar_peacekeeper", "ar_garand",       "ar_famas",       "ar_m16",
    "ar_m14",         "lmg_rpk",         "shotgun_energy", "pistol_energy",
    "pistol_m1911",   "pistol_standard", "launcher_multi"};

struct patched_table {
  std::vector<StringTableCell> values;
  std::vector<int16_t> cell_index;
};

std::mutex patch_mutex;
std::unordered_map<const StringTable *, patched_table> patched_tables;

utils::hook::detour bg_unlockables_load_items_hook;
utils::hook::detour db_link_xasset_entry_hook;

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
  const StringTableCell unlock_prestige =
      cell(zm, *no_prestige, COLUMN_UNLOCK_PRESTIGE);

  std::vector<StringTableCell> values = copy_values(zm);
  const auto make_available = [&](const size_t start, const int32_t source) {
    values[start + COLUMN_UNLOCK_LEVEL] = unlock_level;
    values[start + COLUMN_ALLOCATION] = allocation;
    values[start + COLUMN_LOADOUT_SLOT] = cell(mp, source, COLUMN_LOADOUT_SLOT);
    values[start + COLUMN_UNLOCK_PRESTIGE] = unlock_prestige;
  };

  for (const std::string_view &ref : KIT_WEAPONS) {
    const std::optional<int32_t> source = find_row(mp, COLUMN_REF, ref);
    if (!source) {
      continue;
    }
    const std::string_view index = cell_text(mp, *source, COLUMN_INDEX);
    const bool index_taken = find_row(zm, COLUMN_INDEX, index).has_value();

    if (const std::optional<int32_t> row = find_row(zm, COLUMN_REF, ref)) {
      const size_t start = *row * zm->columnCount;
      if (cell_text(zm, *row, COLUMN_INDEX) != index) {
        if (index_taken) {
          continue;
        }
        values[start + COLUMN_INDEX] = cell(mp, *source, COLUMN_INDEX);
      }
      make_available(start, *source);
      continue;
    }

    if (index_taken) {
      continue;
    }

    const size_t start = values.size();
    values.resize(start + zm->columnCount);
    for (int32_t column = 0;
         column < std::min(zm->columnCount, mp->columnCount); ++column) {
      values[start + column] = cell(mp, *source, column);
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

game::db::xasset::XAssetEntry *
db_link_xasset_entry_stub(game::db::xasset::XAssetEntry *entry,
                          const game::qboolean allow_override) {
  game::db::xasset::XAssetEntry *linked =
      db_link_xasset_entry_hook.invoke<game::db::xasset::XAssetEntry *>(
          entry, allow_override);
  if (linked &&
      linked->asset.type == game::db::xasset::XAssetType::STRINGTABLE) {
    std::scoped_lock lock(patch_mutex);
    remove_forced_kit_attachments(linked->asset.header.stringTable);
  }
  return linked;
}
} // namespace

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("weapon_kits");

public:
  void post_unpack() override {
    bg_unlockables_load_items_hook.create(
        game::bg::BG_UnlockablesLoadItems.get(),
        bg_unlockables_load_items_stub);
    db_link_xasset_entry_hook.create(game::db::load::DB_LinkXAssetEntry.get(),
                                     db_link_xasset_entry_stub);
  }
};
} // namespace weapon_kits

REGISTER_COMPONENT(weapon_kits::component)
