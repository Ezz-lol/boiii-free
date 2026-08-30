#pragma once
#include <game/game.hpp>
#include <utils/string.hpp>
#include "script_value.hpp"

#include "../../component/ui_scripting.hpp"

namespace ui_scripting {
using namespace game::lua::hks;
class lightuserdata {
public:
  lightuserdata(void *);
  void *ptr;
};

class userdata_value;

class userdata {
public:
  userdata(void *);

  userdata(const userdata &other);
  userdata(userdata &&other) noexcept;

  ~userdata();

  userdata &operator=(const userdata &other);
  userdata &operator=(userdata &&other) noexcept;

  script_value get(const script_value &key) const;
  void set(const script_value &key, const script_value &value) const;

  userdata_value operator[](const script_value &key) const;

  void *ptr;

private:
  void add();
  void release();

  int ref{};
};

class userdata_value : public script_value {
public:
  userdata_value(const userdata &table, const script_value &key);
  void operator=(const script_value &value);
  bool operator==(const script_value &value);

private:
  userdata userdata_;
  script_value key_;
};

class table_value;

class table {
public:
  table();
  table(HashTable *ptr_);

  table(const table &other);
  table(table &&other) noexcept;

  ~table();

  table &operator=(const table &other);
  table &operator=(table &&other) noexcept;

  [[nodiscard]] script_value get(const script_value &key) const;
  void set(const script_value &key, const script_value &value) const;

  table_value operator[](const script_value &key) const;

  HashTable *ptr;

private:
  void add();
  void release();

  int ref{};
};

class table_value : public script_value {
public:
  table_value(const table &table, const script_value &key);
  void operator=(const script_value &value);
  void operator=(const table_value &value);
  bool operator==(const script_value &value);
  bool operator==(const table_value &value);

private:
  table table_;
  script_value key_;
};

class function {
public:
  function(lua_CFunction *);
  function(cclosure *, HksObjectType);

  template <typename F> function(F f) {
    this->ptr = ui_scripting::convert_function(f);
    this->type = HksObjectType::TCFUNCTION;
  }

  function(const function &other);
  function(function &&other) noexcept;

  ~function();

  function &operator=(const function &other);
  function &operator=(function &&other) noexcept;

  arguments call(const arguments &arguments) const;

  arguments operator()(const arguments &arguments) const;

  template <class... T> arguments operator()(T... arguments) const {
    return this->call({arguments...});
  }

  arguments operator()() const;

  cclosure *ptr;
  HksObjectType type;

private:
  void add();
  void release();

  int ref{};
};

template <typename T> constexpr const char *get_typename() {

  if (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> ||
      std::is_same_v<T, const char *>) {
    return "string";
  }
  if (std::is_same_v<T, lightuserdata>) {
    return "lightuserdata";
  }
  if (std::is_same_v<T, userdata>) {
    return "userdata";
  }
  if (std::is_same_v<T, table> ||
      std::is_same_v<std::remove_pointer_t<T>, HashTable>) {
    return "table";
  }
  if (std::is_same_v<T, function>) {
    return "function";
  }
  if (std::is_same_v<T, int> || std::is_same_v<T, float> ||
      std::is_same_v<T, unsigned int>) {
    return "number";
  }
  if constexpr (std::is_same_v<T, bool>) {
    return "boolean";
  }

  return reflect_name<T>();
}

template <typename T> T script_value::as() const {
  if (!this->is<T>()) {
    const char *hks_typename =
        s_compilerTypeName->pool[static_cast<int32_t>(this->get_raw().t)];
    const std::string_view typename_ = get_typename<T>();

    throw std::runtime_error(utils::string::va("%s expected, got %s",
                                               typename_.data(), hks_typename));
  }
  return get<T>();
}

template <typename T> T function_argument::as() const {
  if constexpr (std::is_same_v<T, variadic_args>) {
    variadic_args args{};
    for (std::size_t i = this->index_; i < this->values_.size(); i++) {
      args.push_back(this->values_[i]);
    }
    return args;
  } else {
    try {
      return this->value_.as<T>();
    } catch (const std::exception &e) {
      throw std::runtime_error(utils::string::va("bad argument #%d (%s)",
                                                 this->index_ + 1, e.what()));
    }
  }
}

template <template <class, class> class C, class T, typename TableType>
script_value::script_value(const C<T, std::allocator<T>> &container) {
  TableType table_{};
  int32_t index = 1;

  for (const auto &value : container) {
    table_.set(index++, value);
  }

  HksObject obj{};
  obj.t = HksObjectType::TTABLE;
  obj.v.ptr = table_.ptr;

  this->value_ = obj;
}

template <typename F>
script_value::script_value(F f) : script_value(function(f)) {}

template <class... T> arguments script_value::operator()(T... arguments) const {
  return this->as<function>().call({arguments...});
}

template <size_t Size>
table_value script_value::operator[](const char (&key)[Size]) const {
  return {this->as<table>(), key};
}

template <typename T> table_value script_value::operator[](const T &key) const {
  return {this->as<table>(), key};
}

} // namespace ui_scripting