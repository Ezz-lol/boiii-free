#pragma once

#include <std_include.hpp>
#include <game/structs/scr/scr.hpp>
#include <frozen/unordered_set.h>
#include <frozen/string.h>
#include <str.hpp>

namespace gsc {
using namespace game::scr;

struct hash_name_pair {
  uint32_t hash;
  std::string name;
  int line;
  uint8_t params;
};

inline constexpr frozen::string SCR_HASH_LITERAL_PREFIX_ARRAY[] = {
    "hash", "var",    "variable", "id",        "function", "fn",
    "func", "method", "meth",     "namespace", "ns"};

inline constexpr frozen::unordered_set<frozen::string,
                                       std::size(SCR_HASH_LITERAL_PREFIX_ARRAY)>
    SCR_HASH_LITERAL_PREFIXES =
        frozen::make_unordered_set(SCR_HASH_LITERAL_PREFIX_ARRAY);

inline constexpr bool hash_literal_prefix(const std::string_view &s) {
  return SCR_HASH_LITERAL_PREFIXES.contains(s);
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
        std::string_view hex_part = inputSubstr.substr(underscoreIdx + 1);
        constexpr uint32_t EXPECTED_HEX_DIGITS =
            sizeof(uint32_t) * 2 /* digits per byte */;
        while (hex_part.size() > EXPECTED_HEX_DIGITS && hex_part[0] == '0') {
          hex_part = hex_part.substr(1);
        }
        if (hex_part.size() > 0 && hex_part.size() <= EXPECTED_HEX_DIGITS) {
          char padded_hex_part_buf[EXPECTED_HEX_DIGITS + 1 /* NULL */] = {
              '0', '0', '0', '0', '0', '0', '0', '0', '\0'};
          const size_t num_padding_bytes =
              EXPECTED_HEX_DIGITS - hex_part.size();
          strscpy(&padded_hex_part_buf[num_padding_bytes], hex_part.data(),
                  hex_part.size() + 1 /* NULL */);
          const std::string_view padded_hex_part = padded_hex_part_buf;

          for (char c : padded_hex_part) {
            if (!std::isxdigit(static_cast<unsigned char>(c)))
              return std::nullopt;
          }

          ScrVarCanonicalName_t out = 0;
          auto [ptr, ec] = std::from_chars(
              padded_hex_part.data(),
              padded_hex_part.data() + padded_hex_part.size(), out, 16);

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