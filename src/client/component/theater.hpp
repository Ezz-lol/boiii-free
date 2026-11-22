#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace theater {
struct film {
  uint64_t id{};
  uint32_t size{};
  int32_t mode{};
  int32_t duration{};
  std::string map;
  std::string gametype;
  std::string author;
  std::string created;
};

// Locally recorded films, newest first.
std::vector<film> get_films();
} // namespace theater
