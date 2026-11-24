#include "../src/common/exception/recovery_policy.hpp"
#include <cassert>
#include <iostream>

int main() {
  // The isolated Wine crash contained a noncontinuable C++ runtime_error,
  // followed by STATUS_NONCONTINUABLE_EXCEPTION when recovery resumed it.
  assert(!exception::can_resume_exception(0xE06D7363u, 1u));
  assert(!exception::can_resume_exception(0xC0000025u, 1u));
  assert(!exception::can_resume_exception(0xC0000025u, 0u));
  assert(!exception::can_resume_exception(0xC0000005u, 1u));
  assert(!exception::can_resume_exception(0xC0000095u, 1u));
  // Existing recovery remains available for continuable native exceptions.
  assert(exception::can_resume_exception(0xC0000005u, 0u));
  assert(exception::can_resume_exception(0x80000004u, 0u));
  assert(exception::can_resume_exception(0xC0000095u, 0u));
  std::cout << "PASS: noncontinuable C++/native exceptions cannot resume\n";
}
