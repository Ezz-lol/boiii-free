#pragma once

#include <std_include.hpp>
#include <game/structs/scr/scr.hpp>

namespace gsc {
using namespace game::scr;

constexpr uint64_t T7_MAGIC = 0x1C000A0D43534780;

// Special namespace that immediately indicates to engine's linker that this is
// a builtin function
constexpr const char *GSCR_SYS_NS = "sys";
constexpr ScrVarCanonicalName_t GSCR_SYS_NS_HASH =
    game::scr::builtin::fnv1a(GSCR_SYS_NS);

inline constexpr std::string_view SCR_HASH_LITERAL_PREFIXES[] = {
    "hash", "id", "function", "var", "namespace"};

inline constexpr bool hash_literal_prefix(const std::string_view &s) {
  for (uint32_t i = 0; i < ARRAYSIZE(SCR_HASH_LITERAL_PREFIXES); i++) {
    if (s == SCR_HASH_LITERAL_PREFIXES[i]) {
      return true;
    }
  }

  return false;
}

inline constexpr std::optional<ScrVarCanonicalName_t>
try_parse_raw_hash(const std::string_view &input) {
  if (input.size() > 0) {
    std::string_view inputSubstr = input;
    if (inputSubstr[0] == '_') {
      inputSubstr = inputSubstr.substr(1);
    }
    const size_t underscoreIdx = inputSubstr.find('_');
    if (underscoreIdx != std::string::npos &&
        underscoreIdx < inputSubstr.size()) {

      const std::string_view prefix = inputSubstr.substr(0, underscoreIdx);
      if (hash_literal_prefix(prefix)) {

        const std::string_view hex_part = inputSubstr.substr(underscoreIdx + 1);
        if (hex_part.size() == 8) {

          for (char c : hex_part) {
            if (!std::isxdigit(static_cast<unsigned char>(c)))
              return std::nullopt;
          }

          ScrVarCanonicalName_t out = 0;
          auto [ptr, ec] = std::from_chars(
              hex_part.data(), hex_part.data() + hex_part.size(), out, 16);

          if (ec == std::errc{} && out != 0) {
            return out;
          }
        }
      }
    }
  }

  return std::nullopt;
}

inline constexpr game::scr::ScrVarCanonicalName_t
gsc_hash(const std::string &input) {
  return try_parse_raw_hash(input).value_or(
      game::scr::builtin::fnv1a(input.c_str()));
}
} // namespace gsc