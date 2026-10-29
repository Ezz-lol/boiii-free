#include "info_string.hpp"
#include "string.hpp"
#include <macros/macros.hpp>

namespace utils {
void info_string::parse(std::string buffer) {
  if (buffer[0] == '\\') {
    buffer = buffer.substr(1);
  }

  const std::vector<std::string> key_values = string::split(buffer, '\\');
  for (size_t i = 0; !key_values.empty() && i < (key_values.size() - 1);
       i += 2) {
    const auto &key = key_values[i];
    const auto &value = key_values[i + 1];

    if (!this->key_value_pairs_.contains(key)) {
      this->key_value_pairs_[key] = value;
    }
  }
}

std::string info_string::build() const {
  std::string info_string;
  for (const auto &[key, val] : this->key_value_pairs_) {
    info_string.append("\\");
    info_string.append(key);
    info_string.append("\\");
    info_string.append(val);
  }

  return info_string;
}
} // namespace utils
