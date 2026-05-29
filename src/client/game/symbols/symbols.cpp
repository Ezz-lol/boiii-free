#include <std_include.hpp>
#include "symbols.hpp"
#include "../structs/structs.hpp"
#include <cctype>

namespace game {
namespace scr {
bool I_islower(int c) { return c >= 'a' && c <= 'z'; }

bool I_isupper(int c) { return c >= 'A' && c <= 'Z'; }

game::CanonHash_t Scr_CanonHash(const char *str) {
  constexpr game::CanonHash_t FNV_OFFSET_VAL = 0x4B9ACE2F;
  constexpr game::CanonHash_t FNV_PRIME_VAL = 16777619;

  const char *s = str;
  const int32_t first_char = I_islower(*s)
                                 ? static_cast<unsigned char>(*s)
                                 : tolower(static_cast<unsigned char>(*str));

  game::CanonHash_t hash = FNV_PRIME_VAL * (first_char ^ FNV_OFFSET_VAL);
  while (*s) {
    int32_t acc = I_islower(*++s)
                      ? static_cast<unsigned char>(*s)
                      : std::tolower(static_cast<unsigned char>(*s));

    hash = FNV_PRIME_VAL * (acc ^ hash);
  }

  return hash;
}
} // namespace scr
} // namespace game