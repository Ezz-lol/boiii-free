// Compile with the actual src/common/utils/flags.cpp and argparse includes.
// Run each case in a fresh process because the native parser is process-global.
#include "utils/flags.hpp"
#include <iostream>
#include <string_view>
int main(int argc, char* argv[])
{
    if (utils::flags::parse_flags(argc, argv) != 0) return 2;
    const bool expected = argc > 1 && std::string_view(argv[1]) == "expect-enabled";
    const bool actual = utils::flags::has_flag("coop-debug");
    std::cout << "coop-debug=" << actual << '\n';
    return actual == expected ? 0 : 1;
}
