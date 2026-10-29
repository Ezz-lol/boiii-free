#pragma once

#include "macros/builtin.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>

namespace utils {
class info_string {
public:
  info_string() = default;
  alwaysinline info_string(const std::string &buffer) { this->parse(buffer); }

  alwaysinline info_string(const char *buffer)
      : info_string(std::string{buffer}) {}

  alwaysinline info_string(const std::string_view &buffer)
      : info_string(std::string{buffer}) {}

  alwaysinline info_string(const std::basic_string_view<uint8_t> &buffer)
      : info_string(std::string_view(
            reinterpret_cast<const char *>(buffer.data()), buffer.size())) {}

  alwaysinline void set(const std::string &key, const std::string &value) {
    key_value_pairs_[key] = value;
  }

  alwaysinline void set(const std::string_view &key, const std::string &value) {
    set(std::string(key), value);
  }

  alwaysinline void set(const char *key, const std::string &value) {
    set(std::string(key), value);
  }

  alwaysinline void set(const std::string &key, const std::string_view &value) {
    set(key, std::string(value));
  }

  alwaysinline void set(const std::string_view &key,
                        const std::string_view &value) {
    set(std::string(key), std::string(value));
  }

  alwaysinline void set(const char *key, const std::string_view &value) {
    set(std::string(key), std::string(value));
  }

  alwaysinline void set(const std::string &key, const char *value) {
    set(key, std::string(value));
  }

  alwaysinline void set(const std::string_view &key, const char *value) {
    set(std::string(key), std::string(value));
  }

  alwaysinline void set(const char *key, const char *value) {
    set(std::string(key), std::string(value));
  }

  alwaysinline std::string get(const std::string &key) const {
    const auto value = key_value_pairs_.find(key);
    if (value != key_value_pairs_.end()) {
      return value->second;
    }

    return {};
  }

  alwaysinline std::string get(const char *key) const {
    return get(std::string(key));
  }

  alwaysinline std::string get(const std::string_view &key) const {
    return get(std::string(key));
  }

  std::string build() const;

private:
  std::unordered_map<std::string, std::string> key_value_pairs_{};

  void parse(std::string buffer);
};
} // namespace utils
