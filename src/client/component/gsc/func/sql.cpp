#include <std_include.hpp>

#include "http.hpp"
#include "io/fs.hpp"
#include "script_value.hpp"

#include <component/path.hpp>
#include <loader/component_loader.hpp>

#include <utils/io.hpp>

#include <sqlite3.h>

namespace gsc {
namespace func {
namespace sql {
namespace {
using script_value::task_result;

rapidjson::Value integer_value(const int64_t value,
                               script_value::json_allocator &allocator) {
  if (value >= INT32_MIN && value <= INT32_MAX) {
    return rapidjson::Value(static_cast<int32_t>(value));
  }
  return rapidjson::Value(std::to_string(value).c_str(), allocator);
}

using params_t = std::shared_ptr<const rapidjson::Document>;

params_t read_params(scriptInstance_t inst, const uint32_t first) {
  auto params = std::make_shared<rapidjson::Document>(rapidjson::kArrayType);
  auto &allocator = params->GetAllocator();
  const uint32_t count = Scr_GetNumParam(inst);
  if (count == first + 1 && Scr_GetType(inst, first) == ScrVarType::POINTER &&
      Scr_GetPointerType(inst, first) == ScrVarType::ARRAY) {
    rapidjson::Value list = script_value::read(inst, first, allocator);
    if (list.IsArray()) {
      static_cast<rapidjson::Value &>(*params).Swap(list);
    }
    return params;
  }
  for (uint32_t i = first; i < count; ++i) {
    params->PushBack(script_value::read(inst, i, allocator), allocator);
  }
  return params;
}

task_result make_result(rapidjson::Document rows, const int64_t affected,
                        const std::string &error) {
  task_result result;
  result.emplace_back(std::move(rows));
  result.emplace_back(script_value::make_value(affected));
  result.emplace_back(error.empty() ? script_value::make_null()
                                    : script_value::make_value(error));
  return result;
}

namespace sqlite {
struct database {
  sqlite3 *handle{};
  std::mutex mutex;
};

std::mutex databases_mutex;
std::unordered_map<std::string, std::unique_ptr<database>> databases;

int authorize(void *, const int action, const char *, const char *,
              const char *, const char *) {
  return action == SQLITE_ATTACH || action == SQLITE_DETACH ? SQLITE_DENY
                                                            : SQLITE_OK;
}

database *open(const std::filesystem::path &path, std::string &error) {
  std::lock_guard lock(databases_mutex);
  const std::string key = path.string();
  if (const auto found = databases.find(key); found != databases.end()) {
    return found->second.get();
  }

  utils::io::create_directory(path.parent_path());
  sqlite3 *handle = nullptr;
  if (sqlite3_open_v2(key.c_str(), &handle,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                          SQLITE_OPEN_FULLMUTEX,
                      nullptr) != SQLITE_OK) {
    error = handle ? sqlite3_errmsg(handle) : "failed to open database";
    sqlite3_close(handle);
    return nullptr;
  }
  sqlite3_busy_timeout(handle, 5000);
  sqlite3_limit(handle, SQLITE_LIMIT_ATTACHED, 0);
  sqlite3_set_authorizer(handle, authorize, nullptr);
  sqlite3_db_config(handle, SQLITE_DBCONFIG_DEFENSIVE, 1, nullptr);
  sqlite3_db_config(handle, SQLITE_DBCONFIG_ENABLE_TRIGGER, 1, nullptr);

  auto entry = std::make_unique<database>();
  entry->handle = handle;
  database *result = entry.get();
  databases.emplace(key, std::move(entry));
  return result;
}

void bind(sqlite3_stmt *statement, const int index,
          const rapidjson::Value &value) {
  if (value.IsNull()) {
    sqlite3_bind_null(statement, index);
  } else if (value.IsBool()) {
    sqlite3_bind_int(statement, index, value.GetBool() ? 1 : 0);
  } else if (value.IsInt64()) {
    sqlite3_bind_int64(statement, index, value.GetInt64());
  } else if (value.IsNumber()) {
    sqlite3_bind_double(statement, index, value.GetDouble());
  } else {
    const std::string text = script_value::to_string(value);
    sqlite3_bind_text(statement, index, text.c_str(),
                      static_cast<int>(text.size()), SQLITE_TRANSIENT);
  }
}

rapidjson::Value read_row(sqlite3_stmt *statement,
                          script_value::json_allocator &allocator) {
  rapidjson::Value row(rapidjson::kObjectType);
  const int columns = sqlite3_column_count(statement);
  for (int i = 0; i < columns; ++i) {
    rapidjson::Value value;
    switch (sqlite3_column_type(statement, i)) {
    case SQLITE_INTEGER:
      value = integer_value(sqlite3_column_int64(statement, i), allocator);
      break;
    case SQLITE_FLOAT:
      value.SetDouble(sqlite3_column_double(statement, i));
      break;
    case SQLITE_NULL:
      break;
    default:
      value.SetString(
          reinterpret_cast<const char *>(sqlite3_column_text(statement, i)),
          static_cast<uint32_t>(sqlite3_column_bytes(statement, i)), allocator);
      break;
    }
    const char *name = sqlite3_column_name(statement, i);
    row.AddMember(rapidjson::Value(name ? name : "", allocator), value,
                  allocator);
  }
  return row;
}

task_result execute(const std::filesystem::path &path, const std::string &sql,
                    const rapidjson::Value &params) {
  rapidjson::Document rows(rapidjson::kArrayType);
  std::string error;
  database *db = open(path, error);
  if (!db) {
    return make_result(std::move(rows), 0, error);
  }

  std::lock_guard lock(db->mutex);
  const char *tail = sql.c_str();
  const char *end = sql.c_str() + sql.size();
  int64_t affected = 0;
  bool bound = false;
  while (tail < end && error.empty()) {
    sqlite3_stmt *statement = nullptr;
    if (sqlite3_prepare_v2(db->handle, tail, static_cast<int>(end - tail),
                           &statement, &tail) != SQLITE_OK) {
      error = sqlite3_errmsg(db->handle);
      break;
    }
    if (!statement) {
      continue;
    }

    const int param_count = sqlite3_bind_parameter_count(statement);
    if (param_count > 0) {
      if (bound ||
          static_cast<rapidjson::SizeType>(param_count) != params.Size()) {
        error = "wrong number of parameters";
        sqlite3_finalize(statement);
        break;
      }
      for (int i = 0; i < param_count; ++i) {
        bind(statement, i + 1, params[static_cast<rapidjson::SizeType>(i)]);
      }
      bound = true;
    }

    rapidjson::Document statement_rows(rapidjson::kArrayType);
    int step;
    while ((step = sqlite3_step(statement)) == SQLITE_ROW) {
      statement_rows.PushBack(read_row(statement, rows.GetAllocator()),
                              rows.GetAllocator());
    }
    if (step != SQLITE_DONE) {
      error = sqlite3_errmsg(db->handle);
    } else if (sqlite3_column_count(statement) > 0) {
      rows.Swap(statement_rows);
    } else {
      affected += sqlite3_changes(db->handle);
    }
    sqlite3_finalize(statement);
  }

  if (error.empty() && !bound && !params.Empty()) {
    error = "wrong number of parameters";
  }
  return make_result(std::move(rows), affected, error);
}

void gscr_query(scriptInstance_t inst) {
  const char *db = Scr_GetString(inst, 0);
  const char *query = Scr_GetString(inst, 1);
  if (!db || !query || !io::fs::is_safe_path(db)) {
    script_value::start_task(inst, [] {
      return script_value::error_result("invalid database path");
    });
    return;
  }

  script_value::start_task(
      inst,
      [path = io::fs::resolve_path(db), sql = std::string(query),
       params = read_params(inst, 2)] { return execute(path, sql, *params); });
}
} // namespace sqlite

namespace mysql {
struct MYSQL;
struct MYSQL_RES;
using MYSQL_ROW = char **;

constexpr int MYSQL_OPT_CONNECT_TIMEOUT = 0;
constexpr int MYSQL_SET_CHARSET_NAME = 7;
constexpr int MYSQL_OPT_LOCAL_INFILE = 8;
constexpr int MYSQL_OPT_READ_TIMEOUT = 11;
constexpr int MYSQL_OPT_WRITE_TIMEOUT = 12;
constexpr size_t FIELD_TYPE_OFFSET = 104;

enum field_type : uint32_t {
  TYPE_TINY = 1,
  TYPE_SHORT = 2,
  TYPE_LONG = 3,
  TYPE_FLOAT = 4,
  TYPE_DOUBLE = 5,
  TYPE_LONGLONG = 8,
  TYPE_INT24 = 9,
  TYPE_YEAR = 13,
};

struct api {
  int (*server_init)(int, char **, char **);
  MYSQL *(*init)(MYSQL *);
  int (*options)(MYSQL *, int, const void *);
  MYSQL *(*real_connect)(MYSQL *, const char *, const char *, const char *,
                         const char *, unsigned int, const char *,
                         unsigned long);
  const char *(*error)(MYSQL *);
  void (*close)(MYSQL *);
  int (*ping)(MYSQL *);
  int (*real_query)(MYSQL *, const char *, unsigned long);
  MYSQL_RES *(*store_result)(MYSQL *);
  unsigned int (*field_count)(MYSQL *);
  unsigned int (*num_fields)(MYSQL_RES *);
  void *(*fetch_field_direct)(MYSQL_RES *, unsigned int);
  MYSQL_ROW (*fetch_row)(MYSQL_RES *);
  unsigned long *(*fetch_lengths)(MYSQL_RES *);
  void (*free_result)(MYSQL_RES *);
  uint64_t (*affected_rows)(MYSQL *);
  unsigned long (*real_escape_string)(MYSQL *, char *, const char *,
                                      unsigned long);
  int (*next_result)(MYSQL *);
  void (*thread_end)();
};

struct config {
  std::string host = "127.0.0.1";
  std::string user;
  std::string password;
  std::string database;
  uint32_t port = 3306;
  uint32_t version{};
};

std::mutex config_mutex;
config current_config;

template <typename T>
bool load_function(HMODULE module, T &target, const char *name) {
  target = reinterpret_cast<T>(GetProcAddress(module, name));
  return target != nullptr;
}

const api *get_api(std::string &error) {
  static std::optional<api> loaded;
  static std::string load_error;
  static std::once_flag once;
  std::call_once(once, [] {
    HMODULE module = nullptr;
    for (const char *name : {"libmariadb.dll", "libmysql.dll"}) {
      const std::filesystem::path library = path::cwd() / name;
      if (utils::io::file_exists(library)) {
        module = LoadLibraryExW(library.c_str(), nullptr,
                                LOAD_WITH_ALTERED_SEARCH_PATH);
        if (module) {
          break;
        }
      }
    }
    if (!module) {
      load_error = "libmariadb.dll or libmysql.dll was not found in the "
                   "game folder";
      return;
    }

    api functions{};
    const bool ok =
        load_function(module, functions.server_init, "mysql_server_init") &&
        load_function(module, functions.init, "mysql_init") &&
        load_function(module, functions.options, "mysql_options") &&
        load_function(module, functions.real_connect, "mysql_real_connect") &&
        load_function(module, functions.error, "mysql_error") &&
        load_function(module, functions.close, "mysql_close") &&
        load_function(module, functions.ping, "mysql_ping") &&
        load_function(module, functions.real_query, "mysql_real_query") &&
        load_function(module, functions.store_result, "mysql_store_result") &&
        load_function(module, functions.field_count, "mysql_field_count") &&
        load_function(module, functions.num_fields, "mysql_num_fields") &&
        load_function(module, functions.fetch_field_direct,
                      "mysql_fetch_field_direct") &&
        load_function(module, functions.fetch_row, "mysql_fetch_row") &&
        load_function(module, functions.fetch_lengths, "mysql_fetch_lengths") &&
        load_function(module, functions.free_result, "mysql_free_result") &&
        load_function(module, functions.affected_rows, "mysql_affected_rows") &&
        load_function(module, functions.real_escape_string,
                      "mysql_real_escape_string") &&
        load_function(module, functions.next_result, "mysql_next_result") &&
        load_function(module, functions.thread_end, "mysql_thread_end");
    if (!ok || functions.server_init(0, nullptr, nullptr) != 0) {
      load_error = "the MySQL client library could not be loaded";
      return;
    }
    loaded = functions;
  });
  error = load_error;
  return loaded ? &*loaded : nullptr;
}

struct connection {
  MYSQL *handle{};
  uint32_t version{};

  ~connection() {
    std::string error;
    if (const api *functions = get_api(error)) {
      if (handle) {
        functions->close(handle);
      }
      functions->thread_end();
    }
  }
};

MYSQL *connect(const api &functions, std::string &error) {
  thread_local connection current;
  config settings;
  {
    std::lock_guard lock(config_mutex);
    settings = current_config;
  }
  if (settings.version == 0) {
    error = "mysql_setconfig must be called first";
    return nullptr;
  }

  if (current.handle && (current.version != settings.version ||
                         functions.ping(current.handle) != 0)) {
    functions.close(current.handle);
    current.handle = nullptr;
  }
  if (current.handle) {
    return current.handle;
  }

  MYSQL *handle = functions.init(nullptr);
  if (!handle) {
    error = "out of memory";
    return nullptr;
  }
  const unsigned int timeout = 10;
  functions.options(handle, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
  functions.options(handle, MYSQL_OPT_READ_TIMEOUT, &timeout);
  functions.options(handle, MYSQL_OPT_WRITE_TIMEOUT, &timeout);
  functions.options(handle, MYSQL_SET_CHARSET_NAME, "utf8mb4");
  const unsigned int local_infile = 0;
  functions.options(handle, MYSQL_OPT_LOCAL_INFILE, &local_infile);
  if (!functions.real_connect(
          handle, settings.host.c_str(), settings.user.c_str(),
          settings.password.c_str(),
          settings.database.empty() ? nullptr : settings.database.c_str(),
          settings.port, nullptr, 0)) {
    error = functions.error(handle);
    functions.close(handle);
    return nullptr;
  }
  current.handle = handle;
  current.version = settings.version;
  return handle;
}

std::string literal(const api &functions, MYSQL *handle,
                    const rapidjson::Value &value) {
  if (value.IsNull()) {
    return "NULL";
  }
  if (value.IsBool()) {
    return value.GetBool() ? "1" : "0";
  }
  if (value.IsNumber()) {
    return script_value::to_string(value);
  }
  const std::string text = script_value::to_string(value);
  std::string escaped(text.size() * 2 + 1, '\0');
  const unsigned long length =
      functions.real_escape_string(handle, escaped.data(), text.c_str(),
                                   static_cast<unsigned long>(text.size()));
  escaped.resize(length);
  return "'" + escaped + "'";
}

bool bind_params(const api &functions, MYSQL *handle, const std::string &sql,
                 const rapidjson::Value &params, std::string &result) {
  rapidjson::SizeType next = 0;
  char quote = 0;
  for (size_t i = 0; i < sql.size(); ++i) {
    const char c = sql[i];
    if (quote) {
      result += c;
      if (c == '\\' && i + 1 < sql.size()) {
        result += sql[++i];
      } else if (c == quote) {
        quote = 0;
      }
    } else if (c == '\'' || c == '"' || c == '`') {
      quote = c;
      result += c;
    } else if (c == '?') {
      if (next >= params.Size()) {
        return false;
      }
      result += literal(functions, handle, params[next++]);
    } else {
      result += c;
    }
  }
  return next == params.Size();
}

rapidjson::Value field_value(const char *data, const unsigned long length,
                             const uint32_t type,
                             script_value::json_allocator &allocator) {
  const std::string text(data, length);
  switch (type) {
  case TYPE_TINY:
  case TYPE_SHORT:
  case TYPE_LONG:
  case TYPE_INT24:
  case TYPE_YEAR:
  case TYPE_LONGLONG:
    return integer_value(std::strtoll(text.c_str(), nullptr, 10), allocator);
  case TYPE_FLOAT:
  case TYPE_DOUBLE:
    return rapidjson::Value(std::strtod(text.c_str(), nullptr));
  default:
    return rapidjson::Value(text.c_str(), static_cast<uint32_t>(length),
                            allocator);
  }
}

task_result execute(const std::string &sql, const rapidjson::Value &params) {
  rapidjson::Document rows(rapidjson::kArrayType);
  std::string error;
  const api *functions = get_api(error);
  if (!functions) {
    return make_result(std::move(rows), 0, error);
  }
  MYSQL *handle = connect(*functions, error);
  if (!handle) {
    return make_result(std::move(rows), 0, error);
  }

  std::string query;
  if (!bind_params(*functions, handle, sql, params, query)) {
    return make_result(std::move(rows), 0, "wrong number of parameters");
  }
  if (functions->real_query(handle, query.c_str(),
                            static_cast<unsigned long>(query.size())) != 0) {
    return make_result(std::move(rows), 0, functions->error(handle));
  }

  int64_t affected = 0;
  do {
    MYSQL_RES *result = functions->store_result(handle);
    if (result) {
      rapidjson::Document statement_rows(rapidjson::kArrayType);
      auto &allocator = rows.GetAllocator();
      const unsigned int fields = functions->num_fields(result);
      while (MYSQL_ROW row = functions->fetch_row(result)) {
        const unsigned long *lengths = functions->fetch_lengths(result);
        rapidjson::Value entry(rapidjson::kObjectType);
        for (unsigned int i = 0; i < fields; ++i) {
          const char *field = static_cast<const char *>(
              functions->fetch_field_direct(result, i));
          const char *name = *reinterpret_cast<char *const *>(field);
          const uint32_t type =
              *reinterpret_cast<const uint32_t *>(field + FIELD_TYPE_OFFSET);
          rapidjson::Value value;
          if (row[i]) {
            value = field_value(row[i], lengths[i], type, allocator);
          }
          entry.AddMember(rapidjson::Value(name ? name : "", allocator), value,
                          allocator);
        }
        statement_rows.PushBack(entry, allocator);
      }
      functions->free_result(result);
      rows.Swap(statement_rows);
    } else if (functions->field_count(handle) == 0) {
      affected += static_cast<int64_t>(functions->affected_rows(handle));
    } else {
      error = functions->error(handle);
      break;
    }
  } while (functions->next_result(handle) == 0);

  return make_result(std::move(rows), affected, error);
}

void gscr_set_config(scriptInstance_t inst) {
  rapidjson::Document document;
  const rapidjson::Value settings =
      script_value::read(inst, 0, document.GetAllocator());
  std::lock_guard lock(config_mutex);
  config updated;
  updated.version = current_config.version + 1;
  for (const auto &[name, target] :
       {std::pair{"host", &updated.host}, std::pair{"user", &updated.user},
        std::pair{"password", &updated.password},
        std::pair{"database", &updated.database}}) {
    if (const rapidjson::Value *value =
            script_value::find_member(settings, name)) {
      *target = script_value::to_string(*value);
    }
  }
  if (const rapidjson::Value *port =
          script_value::find_member(settings, "port");
      port && port->IsInt() && port->GetInt() > 0) {
    updated.port = static_cast<uint32_t>(port->GetInt());
  }
  current_config = std::move(updated);
}

void gscr_query(scriptInstance_t inst) {
  if (!http::network_allowed()) {
    script_value::start_task(inst, [] {
      return script_value::error_result(
          "network access from GSC is disabled, launch with "
          "-unsafe-gsc-network to allow it");
    });
    return;
  }
  const char *query = Scr_GetString(inst, 0);
  script_value::start_task(
      inst, [sql = std::string(query ? query : ""),
             params = read_params(inst, 1)] { return execute(sql, *params); });
}
} // namespace mysql
} // namespace

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/sql");

  void post_unpack() override {
    register_variadic_builtin(SCRIPTINSTANCE_SERVER, "sqlite_query",
                              sqlite::gscr_query, 2);
    register_builtin(SCRIPTINSTANCE_SERVER, "mysql_setconfig",
                     mysql::gscr_set_config, 1);
    register_variadic_builtin(SCRIPTINSTANCE_SERVER, "mysql_query",
                              mysql::gscr_query, 1);
  }
};
} // namespace sql
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::sql::component)
