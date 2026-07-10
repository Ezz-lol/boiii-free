#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace utils::io {
bool remove_file(const std::filesystem::path &file);
bool remove_directory(const std::filesystem::path &directory,
                      bool recursive = false);
bool move_file(const std::filesystem::path &src,
               const std::filesystem::path &target);
bool file_exists(const std::filesystem::path &file);
bool write_file(const std::filesystem::path &file, const std::string &data,
                bool append = false);
bool write_file_bytes(const std::filesystem::path &file, const uint8_t *data,
                      size_t size, const bool append = false);
bool read_file(const std::filesystem::path &file, std::string *data);
std::string read_file(const std::filesystem::path &file);
size_t file_size(const std::filesystem::path &file);
bool create_directory(const std::filesystem::path &directory);
bool directory_exists(const std::filesystem::path &directory);
bool directory_is_empty(const std::filesystem::path &directory);
void copy_folder(const std::filesystem::path &src,
                 const std::filesystem::path &target);

std::vector<std::filesystem::path>
list_files(const std::filesystem::path &directory, bool recursive = false,
           bool includeDirectories = true);

bool write_file_executable(const std::filesystem::path &file,
                           const std::string &data);
} // namespace utils::io