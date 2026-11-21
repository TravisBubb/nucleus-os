#pragma once

#include <stddef.h>

// Freestanding implementations of the handful of libc functions GCC and Clang
// reserve the right to emit calls to even when they are not called directly.
// DO NOT remove or rename these, or code generation will break at link time.

void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
