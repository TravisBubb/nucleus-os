#pragma once

#include <boot/limine/limine.h>

// Fill the given framebuffer with an example gradient pattern. Assumes a 32-bpp
// RGB framebuffer; the caller is responsible for checking that.
void fb_pattern(struct limine_framebuffer *fb);
