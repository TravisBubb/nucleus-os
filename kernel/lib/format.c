#include <stdint.h>
#include <stdbool.h>
#include <lib/format.h>

// Length modifier parsed from the conversion specifier. Determines the width
// of the integer argument pulled from the va_list.
enum length { LEN_INT, LEN_LONG, LEN_LLONG, LEN_SIZE };

// A bounded output cursor. `pos` keeps counting past `size` so the caller can
// learn the length that *would* have been produced, but bytes are only stored
// while they fit (leaving room for the trailing null written at the end).
struct sink {
  char *buf;
  size_t size;
  size_t pos;
};

static void emit(struct sink *s, char c) {
  if (s->pos + 1 < s->size) {
    s->buf[s->pos] = c;
  }
  s->pos++;
}

static void emit_str(struct sink *s, const char *str) {
  if (str == NULL) {
    str = "(null)";
  }
  while (*str) {
    emit(s, *str++);
  }
}

// Format an unsigned value in the given base (10 or 16) into `s`.
static void emit_uint(struct sink *s, uint64_t value, unsigned base) {
  static const char digits[] = "0123456789abcdef";
  char tmp[20]; // enough for 64-bit in base 10
  size_t i = 0;

  do {
    tmp[i++] = digits[value % base];
    value /= base;
  } while (value > 0);

  while (i > 0) {
    emit(s, tmp[--i]);
  }
}

static void emit_int(struct sink *s, int64_t value) {
  uint64_t mag;
  if (value < 0) {
    emit(s, '-');
    // Negate via unsigned to avoid UB at INT64_MIN.
    mag = (uint64_t)(-(value + 1)) + 1;
  } else {
    mag = (uint64_t)value;
  }
  emit_uint(s, mag, 10);
}

int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
  struct sink s = { buf, size, 0 };

  for (; *fmt; fmt++) {
    if (*fmt != '%') {
      emit(&s, *fmt);
      continue;
    }

    fmt++; // consume '%'

    // Parse optional length modifier.
    enum length len = LEN_INT;
    if (*fmt == 'z') {
      len = LEN_SIZE;
      fmt++;
    } else if (*fmt == 'l') {
      len = LEN_LONG;
      fmt++;
      if (*fmt == 'l') {
        len = LEN_LLONG;
        fmt++;
      }
    }

    switch (*fmt) {
      case 's':
        emit_str(&s, va_arg(args, const char *));
        break;
      case 'c':
        // char is promoted to int through '...'.
        emit(&s, (char)va_arg(args, int));
        break;
      // Integer conversions read va_arg inline: va_list is an array type on
      // the SysV ABI, so it cannot be forwarded to a helper and reused here.
      case 'd':
      case 'i': {
        int64_t v;
        switch (len) {
          case LEN_LLONG: v = va_arg(args, long long); break;
          case LEN_LONG:  v = va_arg(args, long); break;
          case LEN_SIZE:  v = va_arg(args, long); break; // size_t-width signed
          default:        v = va_arg(args, int); break;
        }
        emit_int(&s, v);
        break;
      }
      case 'u':
      case 'x': {
        uint64_t v;
        switch (len) {
          case LEN_LLONG: v = va_arg(args, unsigned long long); break;
          case LEN_LONG:  v = va_arg(args, unsigned long); break;
          case LEN_SIZE:  v = va_arg(args, size_t); break;
          default:        v = va_arg(args, unsigned int); break;
        }
        emit_uint(&s, v, *fmt == 'x' ? 16 : 10);
        break;
      }
      case 'p':
        emit_str(&s, "0x");
        emit_uint(&s, (uint64_t)(uintptr_t)va_arg(args, void *), 16);
        break;
      case '%':
        emit(&s, '%');
        break;
      case '\0':
        // Trailing '%' at end of string: emit it and stop.
        emit(&s, '%');
        fmt--; // so the loop's fmt++ lands on the null and terminates
        break;
      default:
        // Unknown specifier: print it verbatim so bugs are visible.
        emit(&s, '%');
        emit(&s, *fmt);
        break;
    }
  }

  // Null-terminate within bounds.
  if (s.size > 0) {
    size_t term = s.pos < s.size ? s.pos : s.size - 1;
    s.buf[term] = '\0';
  }

  return (int)s.pos;
}

int ksnprintf(char *buf, size_t size, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int n = kvsnprintf(buf, size, fmt, args);
  va_end(args);
  return n;
}
