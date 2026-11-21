#pragma once

#include <stdint.h>
#include <boot/limine/limine.h>

// Limine requests the kernel makes of the bootloader. These are defined in
// requests.c with the section attributes the Limine protocol expects; the rest
// of the kernel only ever touches them through these declarations.

extern volatile uint64_t limine_base_revision[];
extern volatile struct limine_framebuffer_request framebuffer_request;
