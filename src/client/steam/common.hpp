#pragma once

#include <cstdint>

namespace steam {
#ifndef STEAM_EXPORT
#ifdef _WIN32
#define STEAM_EXPORT extern "C" __declspec(dllexport)
#else
#define STEAM_EXPORT extern "C" __attribute__((visibility("default")))
#endif
#endif

struct raw_steam_id final {
  uint32_t account_id : 32;
  uint32_t account_instance : 20;
  uint32_t account_type : 4;
  int32_t universe : 8;
};

union steam_id {
  raw_steam_id raw;
  uint64_t bits;
};

#pragma pack(push, 1)
struct raw_game_id final {
  uint32_t app_id : 24;
  uint32_t type : 8;
  uint32_t mod_id : 32;
};

union game_id {
  raw_game_id raw;
  uint64_t bits;
};
#pragma pack(pop)
} // namespace steam
