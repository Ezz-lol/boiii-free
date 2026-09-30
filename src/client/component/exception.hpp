#pragma once

namespace exception {
void recover_fatal_error(const std::string &reason);
void recover_nested_error(const std::string &reason);
void restart_after_fatal_error(const std::string &message);
} // namespace exception
