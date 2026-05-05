#ifndef __PMM_H
#define __PMM_H

#include "../fb_console/fb_console.h" // IWYU pragma: keep
#include "../limine/limine.h"         // IWYU pragma: keep
#include "limine.h"                   // IWYU pragma: keep
#include "memory.h"                   // IWYU pragma: keep
#include <stdbool.h>                  // IWYU pragma: keep
#include <stddef.h>                   // IWYU pragma: keep
#include <stdint.h>                   // IWYU pragma: keep

void pmm_init();
uint64_t pmm_alloc();
void pmm_free(uint64_t ptr);
#define PHYS_TO_VIRT(addr) ((void *)((uint64_t)(addr) + hhdm_offset))
#define VIRT_TO_PHYS(addr) ((uint64_t)(addr) - hhdm_offset)
#endif // !__PMM_H
