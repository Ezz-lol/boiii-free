#pragma once

#include <string>
#include <windows.h>

namespace exception {
std::string create_minidump(LPEXCEPTION_POINTERS exceptioninfo);
}