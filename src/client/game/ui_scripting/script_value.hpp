#pragma once
#include "game/game.hpp"
#include <utils/string.hpp>

#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <type_traits>

namespace ui_scripting {
class lightuserdata;
class userdata_value;
class userdata;
class table_value;
class table;
class function;
class script_value;

template <typename T> std::string get_typename();

class hks_object {
public:
  hks_object() = default;
  hks_object(const game::hks::HksObject &value);
  hks_object(const hks_object &other) noexcept;
  hks_object(hks_object &&other) noexcept;

  hks_object &operator=(const hks_object &other) noexcept;
  hks_object &operator=(hks_object &&other) noexcept;

  ~hks_object();

  const game::hks::HksObject &get() const;

private:
  void assign(const game::hks::HksObject &value);
  void release();

  game::hks::HksObject value_{game::hks::TNONE, {}};
  int ref_{};
};

using arguments = std::vector<script_value>;
using event_arguments = std::unordered_map<std::string, script_value>;

class script_value {
public:
  script_value() = default;
  script_value(const game::hks::HksObject &value);

  script_value(int value);
  script_value(unsigned int value);
  script_value(bool value);

  script_value(float value);
  script_value(double value);

  script_value(const char *value, std::size_t len);
  script_value(const char *value);
  script_value(const std::string &value);

  script_value(const lightuserdata &value);
  script_value(const userdata &value);
  script_value(const table &value);
  script_value(const function &value);

  template <template <class, class> class C, class T,
            typename TableType = table>
  script_value(const C<T, std::allocator<T>> &container);

  template <typename F> script_value(F f);

  bool operator==(const script_value &other) const;

  [[maybe_unused]] arguments operator()() const;
  [[maybe_unused]] arguments operator()(const arguments &arguments) const;

  template <class... T>
  [[maybe_unused]] arguments operator()(T... arguments) const;

  template <size_t Size> table_value operator[](const char (&key)[Size]) const;

  template <typename T = script_value>
  table_value operator[](const T &key) const;

  template <typename T> [[nodiscard]] bool is() const;

  template <typename T> T as() const;

  template <typename T> operator T() const { return this->as<T>(); }

  [[nodiscard]] const game::hks::HksObject &get_raw() const;

  hks_object value_{};

private:
  template <typename T> T get() const;
};

class variadic_args : public arguments {};

class function_argument {
public:
  function_argument(const arguments &args, const script_value &value,
                    int index);

  template <typename T> T as() const;

  template <typename T> operator T() const { return this->as<T>(); }

private:
  arguments values_{};
  script_value value_{};
  int index_{};
};

class function_arguments {
public:
  function_arguments(const arguments &values) : values_(values) {}

  function_argument operator[](const int index) const {
    if (static_cast<std::size_t>(index) >= values_.size()) {
      return {values_, {}, index};
    }
    return {values_, values_[index], index};
  }

private:
  arguments values_{};
};
} // namespace ui_scripting