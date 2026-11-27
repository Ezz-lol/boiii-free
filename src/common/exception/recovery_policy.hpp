#pragma once

#include <cstdint>

namespace exception {
constexpr bool can_resume_exception(std::uint32_t code,
                                    std::uint32_t flags) noexcept {
  // Windows forbids resuming an exception carrying EXCEPTION_NONCONTINUABLE.
  // STATUS_NONCONTINUABLE_EXCEPTION is itself fatal even if its flags are bad.
  return (flags & 1u) == 0 && code != 0xC0000025u;
}
} // namespace exception
