#include <arch/x86_64/cpu.h>

_Noreturn void hcf(void) {
  for (;;) {
    asm ("hlt");
  }
}
