#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <boot/limine/limine.h>
#include <boot/limine/requests.h>
#include <arch/x86_64/cpu.h>
#include <drivers/video/fb.h>

// The following will be our kernel's entry point. If renaming kmain() to
// something else, make sure to change the linker script accordingly.
void kmain(void) {
  // Ensure the bootloader actually understands our base revision
  if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
    hcf();
  }

  // Ensure we got a framebuffer
  if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
    hcf();
  }

  // Print the pattern to every framebuffer
  for (uint64_t i = 0; i < framebuffer_request.response->framebuffer_count; i++) {
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[i];

    // Ensure the framebuffer has 32-bit RGB pixels, the only kind we handle
    if (framebuffer->memory_model != LIMINE_FRAMEBUFFER_RGB || framebuffer->bpp != 32) {
      hcf();
    }

    fb_pattern(framebuffer);
  }

  // We're done, just hang
  hcf();
}
