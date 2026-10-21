#pragma once

#ifndef UNROLL_NEXT_LOOP
#if defined(__clang__)
#define UNROLL_NEXT_LOOP _Pragma("clang loop unroll(full)")
#elif defined(__GNUC__)
#define UNROLL_NEXT_LOOP _Pragma("GCC unroll 65535")
#else
#define UNROLL_NEXT_LOOP
#endif
#endif
