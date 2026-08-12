#pragma once

#include "structs/array.hpp"
#include <string>
#include <filesystem>
#include <unordered_map>

#define CHUNK 16384u

namespace utils::compression {
namespace zlib {
std::string compress(const std::string &data);
std::string decompress(const std::string &data);
} // namespace zlib

namespace zip {
class archive {
public:
  void add(std::string filename, std::string data);
  bool write(const std::string &filename, const std::string &comment = {});

private:
  std::unordered_map<std::string, std::string> files_;
};

std::unordered_map<std::string, std::vector<uint8_t>>
extract(const std::string &data);

void write_file(const std::filesystem::path &output,
                const std::pair<std::string, std::vector<uint8_t>> *entries,
                size_t count);

template <const size_t N>
inline void write_file(
    const std::filesystem::path &output,
    const array<std::pair<std::string, std::vector<uint8_t>>, N> &entries) {
  return write_file(output, entries, N);
}

template <const size_t N>
inline void
write_file(const std::filesystem::path &output,
           const std::array<std::pair<std::string, std::vector<uint8_t>>, N>
               &entries) {
  return write_file(output, entries, N);
}

inline void write_file(
    const std::filesystem::path &output,
    const std::vector<std::pair<std::string, std::vector<uint8_t>>> &entries) {
  return write_file(output, entries.data(), entries.size());
}
inline void write_file(
    const std::filesystem::path &output,
    const std::unordered_map<std::string, std::vector<uint8_t>> &entries) {
  std::vector<std::pair<std::string, std::vector<uint8_t>>> pairs{
      entries.begin(), entries.end()};
  return write_file(output, pairs);
}

} // namespace zip
} // namespace utils::compression
