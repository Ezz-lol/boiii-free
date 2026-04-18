#include <std_include.hpp>
#include "symbols.hpp"
#include <cctype>

namespace game {
namespace scr {
bool I_islower(int c) { return c >= 'a' && c <= 'z'; }

bool I_isupper(int c) { return c >= 'A' && c <= 'Z'; }

unsigned int Scr_CanonHash(const char *str) {
  constexpr unsigned int FNV_OFFSET_VAL = 0x4B9ACE2F;
  constexpr unsigned int FNV_PRIME_VAL = 16777619;

  const auto *s = str;
  const int first_char = I_islower(*s)
                             ? static_cast<unsigned char>(*s)
                             : tolower(static_cast<unsigned char>(*str));

  unsigned int hash = FNV_PRIME_VAL * (first_char ^ FNV_OFFSET_VAL);
  while (*s) {
    int acc = I_islower(*++s) ? static_cast<unsigned char>(*s)
                              : std::tolower(static_cast<unsigned char>(*s));

    hash = FNV_PRIME_VAL * (acc ^ hash);
  }

  return hash;
}
} // namespace scr
} // namespace game