#pragma once

#include <windows.h>
#include <string>

namespace exception {
std::string create_minidump(LPEXCEPTION_POINTERS exceptioninfo);
}