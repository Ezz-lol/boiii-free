#pragma once
#include <argparse/argparse.hpp>

namespace utils::flags {
extern argparse::ArgumentParser program;
int32_t parse_flags(int argc, char *argv[]);
std::optional<std::string> find_variant(const std::string_view &flag);
bool has_flag(const std::string_view &flag);
inline bool has(const std::string_view &flag) { return has_flag(flag); }
template <typename T> std::optional<T> get(const std::string_view &flag) {
  const std::optional<std::string> variant = find_variant(flag);
  return variant.has_value() ? std::optional(program.get<T>(variant.value()))
                             : std::nullopt;
}
} // namespace utils::flags