#ifndef WEAK
#ifdef _MSC_VER
#define WEAK __declspec(selectany)
#else
#define WEAK __attribute__((weak))
#endif
#endif