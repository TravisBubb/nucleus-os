#include <stddef.h>
#include <stdint.h>
#include <boot/limine/limine.h>
#include <drivers/video/fb.h>

// Scale an 8-bit color channel value to the size the framebuffer gives the
// channel and move it into place within a pixel.
static uint32_t fb_channel(uint8_t value, uint8_t mask_size, uint8_t mask_shift) {
  uint64_t max = ((uint64_t)1 << mask_size) - 1;
  return (uint32_t)((value * max / 255) << mask_shift);
}

// Build a pixel from 8-bit RGB values following the channel layout of the
// framebuffer.
static uint32_t fb_pixel(struct limine_framebuffer *fb, uint8_t red, uint8_t green, uint8_t blue) {
  return fb_channel(red, fb->red_mask_size, fb->red_mask_shift)
    | fb_channel(green, fb->green_mask_size, fb->green_mask_shift)
    | fb_channel(blue, fb->blue_mask_size, fb->blue_mask_shift);
}

void fb_pattern(struct limine_framebuffer *fb) {
  volatile uint32_t *fb_ptr = fb->address;
  for (size_t y = 0; y < fb->height; y++) {
    for (size_t x = 0; x < fb->width; x++) {
      uint8_t nX = x * 255 / fb->width;
      uint8_t nY = y * 255 / fb->height;
      fb_ptr[y * (fb->pitch / 4) + x] = fb_pixel(fb, 0, nY, nX);
    }
  }
}
