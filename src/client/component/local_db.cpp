#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <utils/io.hpp>

#include "local_db.hpp"

#include <sqlite_orm/sqlite_orm.h>

namespace local_db {
namespace {
constexpr const char *db_directory = "boiii_players/user";
constexpr const char *db_file = "boiii_players/user/dw.db";

struct balance_row {
  int32_t currency;
  uint32_t value;
};

struct item_row {
  uint32_t item_id;
  uint32_t quantity;
  uint32_t expire_time;
};

auto make_storage() {
  using namespace sqlite_orm;
  return sqlite_orm::make_storage(
      db_file,
      make_table("balances",
                 make_column("currency", &balance_row::currency, primary_key()),
                 make_column("value", &balance_row::value)),
      make_table("inventory",
                 make_column("item_id", &item_row::item_id, primary_key()),
                 make_column("quantity", &item_row::quantity),
                 make_column("expire_time", &item_row::expire_time,
                             default_value(0))));
}
using storage = decltype(make_storage());

std::recursive_mutex db_mutex;
std::unique_ptr<storage> db;
// Reads are served from memory: the game asks for item quantities very often.
bool cached = false;
std::unordered_map<int32_t, uint32_t> balances;
std::unordered_map<uint32_t, item> items;

storage *open() {
  if (db) {
    return db.get();
  }
  try {
    utils::io::create_directory(db_directory);
    auto opened = std::make_unique<storage>(make_storage());
    opened->open_forever();
    opened->sync_schema(true);
    db = std::move(opened);
  } catch (const std::exception &) {
    return nullptr;
  }
  return db.get();
}

void load(bool reload = false) {
  if (cached && !reload) {
    return;
  }
  cached = true;
  balances.clear();
  items.clear();
  storage *database = open();
  if (!database) {
    return;
  }
  try {
    for (const balance_row &row : database->get_all<balance_row>()) {
      balances[row.currency] = row.value;
    }
    for (const item_row &row : database->get_all<item_row>()) {
      items[row.item_id] = {row.quantity, row.expire_time};
    }
  } catch (const std::exception &) {
  }
}
} // namespace

uint32_t balance(int32_t currency) {
  std::lock_guard _(db_mutex);
  load();
  const auto found = balances.find(currency);
  return found != balances.end() ? found->second : 0;
}

bool set_balance(int32_t currency, uint32_t value) {
  std::lock_guard _(db_mutex);
  load();
  storage *database = open();
  if (!database) {
    return false;
  }
  try {
    database->replace(balance_row{currency, value});
  } catch (const std::exception &) {
    return false;
  }
  balances[currency] = value;
  return true;
}

std::optional<item> get_item(uint32_t item_id) {
  std::lock_guard _(db_mutex);
  load();
  const auto found = items.find(item_id);
  if (found == items.end()) {
    return std::nullopt;
  }
  return found->second;
}

bool set_item(uint32_t item_id, uint32_t quantity, uint32_t expire_time) {
  std::lock_guard _(db_mutex);
  load();
  storage *database = open();
  if (!database) {
    return false;
  }
  try {
    database->replace(item_row{item_id, quantity, expire_time});
  } catch (const std::exception &) {
    return false;
  }
  items[item_id] = {quantity, expire_time};
  return true;
}

bool transaction(const std::function<bool()> &body) {
  std::lock_guard _(db_mutex);
  load();
  storage *database = open();
  if (!database) {
    return false;
  }
  bool committed = false;
  try {
    committed = database->transaction([&] { return body(); });
  } catch (const std::exception &) {
  }
  if (!committed) {
    load(true);
  }
  return committed;
}

struct component final : client_component {
  DEFINE_COMPONENT_NAME("local_db");

  void pre_destroy() override {
    std::lock_guard _(db_mutex);
    db.reset();
  }
};
} // namespace local_db

REGISTER_COMPONENT(local_db::component)
