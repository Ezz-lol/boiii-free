#pragma once

template <typename T> inline constexpr const char *reflect_name() {
  return __PRETTY_FUNCTION__;
}

#ifndef APPENDCHAR
#define APPENDCHAR(ptr, c)                                                     \
  ptr[0] = c;                                                                  \
  ++ptr
#endif
