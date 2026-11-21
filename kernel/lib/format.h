#pragma once

#include <stddef.h>
#include <stdarg.h>

// Minimal snprintf-style formatter for kernel logging. Writes at most
// `size` bytes into `buf` (always null-terminated when size > 0) and returns
// the number of characters that would have been written had the buffer been
// large enough, i.e. excluding the null terminator (C99 snprintf semantics).
//
// Supported conversions:
//   %s  string            %c  char
//   %d %i  signed int      %u  unsigned int
//   %x  unsigned hex       %p  pointer (0x...)
//   %%  literal percent
// Length modifiers: l, ll (long / long long) and z (size_t), e.g. %lx, %zu.
int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int ksnprintf(char *buf, size_t size, const char *fmt, ...);
