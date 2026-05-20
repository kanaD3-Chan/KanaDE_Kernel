#include "pmm.h"
#include <stdint.h>

uint64_t hhdm_offset = 0;

static inline void bitmap_set(uint64_t page_idx) {
  bitmap[page_idx / 8] |= (1 << (page_idx % 8));
}

static inline void bitmap_clear(uint64_t page_idx) {
  bitmap[page_idx / 8] &= ~(1 << (page_idx % 8));
}

static inline bool bitmap_test(uint64_t page_idx) {
  return (bitmap[page_idx / 8] >> (page_idx % 8)) & 1;
}

static inline void bitmap_clear_range(uint64_t base, uint64_t size) {
  uint64_t start = (base + 4095) / 4096;
  uint64_t end = (base + size) / 4096;

  for (uint64_t i = start; i < end; ++i) {
    bitmap_clear(i);
  }
}

void pmm_free(uint64_t ptr) {
  uint64_t phys_addr = ptr;

  if (phys_addr % 4096 != 0)
    return;

  uint64_t page_idx = phys_addr / 4096;
  if (page_idx >= total_pages)
    return;

  bitmap_clear(page_idx);
}

uint64_t pmm_alloc() {
  static uint64_t last_scanned_page = 0;

  for (uint64_t i = 0; i < total_pages;) {
    uint64_t check_idx = (last_scanned_page + i) % total_pages;

    if (bitmap[check_idx / 8] == 0xFF) {
      uint64_t skip = 8 - (check_idx % 8);
      if (check_idx + skip > total_pages)
        skip = total_pages - check_idx;
      i += skip;
      continue;
    }

    if (!bitmap_test(check_idx)) {
      bitmap_set(check_idx);
      last_scanned_page = (check_idx + 1) % total_pages;
      uint64_t phys_addr = check_idx * 4096;
      return phys_addr;
    }
    i++;
  }
  return 0;
}

void pmm_init() {
  if (memmap_request.response == NULL || hhdm_request.response == NULL) {
    while (1)
      __asm__ volatile("cli; hlt");
  }
  struct limine_memmap_response *memmap = memmap_request.response;
  hhdm_offset = hhdm_request.response->offset;

  uint64_t highest_addr = 0;

  for (uint64_t i = 0; i < memmap->entry_count; ++i) {
    struct limine_memmap_entry *entry = memmap->entries[i];
    uint64_t top = entry->base + entry->length;
    if (top > highest_addr)
      highest_addr = top;
  }

  total_pages = (highest_addr + 4095) / 4096;
  bitmap_size = (total_pages + 7) / 8;

  uint64_t aligned_bitmap_size = (bitmap_size + 4095) & ~4095ULL;
  uint64_t bitmap_phys_addr;

  for (uint64_t i = 0; i < memmap->entry_count; ++i) {
    struct limine_memmap_entry *entry = memmap->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE &&
        entry->length >= aligned_bitmap_size) {
      bitmap_phys_addr = entry->base;
      bitmap = PHYS_TO_VIRT(bitmap_phys_addr);

      for (uint64_t j = 0; j < bitmap_size; ++j)
        bitmap[j] = 0xFF;

      entry->base += aligned_bitmap_size;
      entry->length -= aligned_bitmap_size;
      break;
    }
  }

  if (bitmap == NULL) {
    while (1)
      __asm__ volatile("hlt");
  }

  for (uint64_t i = 0; i < memmap->entry_count; ++i) {
    struct limine_memmap_entry *entry = memmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE) {
      bitmap_clear_range(entry->base, entry->length);
    }
  }

  for (uint64_t j = 0; j < 256; j++) {
    bitmap_set(j);
  }
  fb_puts("PMM Initialized.");
}
