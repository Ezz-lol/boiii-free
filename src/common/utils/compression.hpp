#pragma once

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
void write_file(
    const std::filesystem::path &output,
    const std::unordered_map<std::string, std::vector<uint8_t>> &entries);
} // namespace zip
}; // namespace utils::compression