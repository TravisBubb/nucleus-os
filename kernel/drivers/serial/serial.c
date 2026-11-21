#include <stdint.h>
#include <stdarg.h>
#include <drivers/serial/serial.h>
#include <arch/x86_64/io.h>
#include <lib/format.h>

#define COM1 0x3F8

#define SERIAL_DATA (COM1 + 0)
#define SERIAL_IER (COM1 + 1)
#define SERIAL_FIFO (COM1 + 2)
#define SERIAL_LCR (COM1 + 3)
#define SERIAL_MCR (COM1 + 4)
#define SERIAL_LSR (COM1 + 5)

void serial_init(void) {
  // Disable interrupts
  outb(SERIAL_IER, 0x00);

  // Enable divisor latch
  outb(SERIAL_LCR, 0x80);

  // Set divisor to 1 (115200 baud)
  outb(SERIAL_DATA, 0x01);
  outb(SERIAL_IER, 0x00);

  // 8 bits, no parity, one stop bit
  outb(SERIAL_LCR, 0x03);

  // Enable FIFO, clear buffers
  outb(SERIAL_FIFO, 0xC7);

  // Enable DTR, RTS, OUT2
  outb(SERIAL_MCR, 0x0B);
}

void serial_putc(char c) {
  // Wait until transmitter is ready
  while ((inb(SERIAL_LSR) & 0x20) == 0) { }

  outb(SERIAL_DATA, c);
}

void serial_write(const char *str) {
  while (*str) {
    if (*str == '\n') {
      serial_putc('\r');
    }

    serial_putc(*str++);
  }
}

void serial_printf(const char *fmt, ...) {
  char buf[256];

  va_list args;
  va_start(args, fmt);
  kvsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  serial_write(buf);
}