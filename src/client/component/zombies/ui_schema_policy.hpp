#pragma once

#include <string_view>

namespace zombies::coop {
inline bool score_model_chunk(std::string_view name) noexcept {
  if (name.starts_with('@')) name.remove_prefix(1);
  constexpr std::string_view wanted = "ui/uieditor/clientfieldmodels.lua";
  if (name.size() != wanted.size()) return false;
  for (std::size_t i = 0; i < name.size(); ++i) {
    auto c = name[i];
    if (c == '\\') c = '/';
    if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
    if (c != wanted[i]) return false;
  }
  return true;
}
} // namespace zombies::coop
