#pragma once

#include <cstdint>
namespace game {
namespace seh {

enum class Language : int32_t {
  // Name: "english"
  // Prefix: "en_"
  // Abbrev: "en"
  ENGLISH_US = 0x0,
  // Name: "french"
  // Prefix: "fr_"
  // Abbrev: "fr"
  FRENCH = 0x1,
  // Name: "frenchcan"
  // Prefix: "fc_"
  // Abbrev: "fc"
  CANADIAN_FRENCH = 0x2,
  // Name: "german"
  // Prefix: "ge_"
  // Abbrev: "ge"
  GERMAN = 0x3,
  // Name: "austrian"
  // Prefix: "as_"
  // Abbrev: "ge" ; Note: this was probably a mistake by Treyarch haha
  AUSTRIAN = 0x4,
  // Name: "italian"
  // Prefix: "it_"
  // Abbrev: "it"
  ITALIAN = 0x5,
  // Name: "spanish"
  // Prefix: "es_"
  // Abbrev: "es"
  SPANISH = 0x6,
  // Name: "British" ; LOL
  // Prefix: "br_"
  // Abbrev: "br"
  ENGLISH_UK = 0x7,
  // Name: "russian"
  // Prefix: "ru_"
  // Abbrev: "ru"
  RUSSIAN = 0x8,
  // Name: "polish"
  // Prefix: "po_"
  // Abbrev: "en"
  POLISH = 0x9,
  // Name: "korean"
  // Prefix: "ko_"
  // Abbrev: "ko"
  KOREAN = 0xA,
  // Name: "japanese"
  // Prefix: "ja_"
  // Abbrev: "en"
  JAPANESE = 0xB,
  // Name: "czech"
  // Prefix: "cz_"
  // Abbrev: "en"
  CZECH = 0xC,
  // Name: "fulljapanese"
  // Prefix: "fj_"
  // Abbrev: "fj"
  FULL_JAPANESE = 0xD,
  // Name: "portuguese"
  // Prefix: "bp_"
  // Abbrev: "bp"
  PORTUGUESE = 0xE,
  // Name: "mexicanspanish"
  // Prefix: "ms_"
  // Abbrev: "ms"
  MEXICAN_SPANISH = 0xF,
  // Name: "arabic"
  // Prefix: "ar_"
  // Abbrev: "ea"
  ARABIC = 0x10,
  // Name: "traditionalchinese"
  // Prefix: "tc_"
  // Abbrev: "en"
  TRADITIONAL_CHINESE = 0x11,
  // Name: "simplifiedchinese"
  // Prefix: "sc_"
  // Abbrev: "en"
  SIMPLIFIED_CHINESE = 0x12,
  // Name: "englishgerman"
  // Prefix: "eg_"
  // Abbrev: "en"
  ENGLISH_GERMAN = 0x13,
  // Name: "englishjapanese"
  // Prefix: "ej_"
  // Abbrev: "en"
  ENGLISH_JAPANESE = 0x14,
  // Name: "hebrew"
  // Prefix: "he_"
  // Abbrev: "he"
  HEBREW = 0x15,
  // Name: "englisharabic"
  // Prefix: "ea_"
  // Abbrev: "ea"
  ENGLISH_ARABIC = 0x16,

  COUNT = 0x17
};

}
} // namespace game
