#ifndef __LIMINE_H
#define __LIMINE_H

#include <stdint.h>

extern volatile uint64_t limine_base_revision[];

extern volatile struct limine_framebuffer_request fb_request;

extern volatile uint64_t limine_requests_start_marker;

extern volatile uint64_t limine_requests_end_marker;

extern volatile struct limine_memmap_request memmap_request;

extern volatile struct limine_hhdm_request hhdm_request;
#endif // !__LIMINE_H
