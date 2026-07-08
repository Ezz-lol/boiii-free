#pragma once
#include <argparse/argparse.hpp>

namespace utils::flags {
extern argparse::ArgumentParser program;
int32_t parse_flags(int argc, char *argv[]);
bool has_flag(const char *flag);
} // namespace utils::flags