#include "pmm.h"

static uint8_t *bitmap = NULL;
static uint64_t total_pages = 0;
static uint64_t bitmap_size = 0;
static uint64_t hhdm_offset = 0;

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
  uint64_t start = base / 4096;
  uint64_t page_count = size / 4096;

  for (uint64_t i = 0; i < page_count; ++i) {
    uint64_t page_idx = start + i;
    bitmap_clear(page_idx);
  }
}

void pmm_free(void *ptr) {
  uint64_t phys_addr = (uint64_t)ptr;

  if (phys_addr % 4096 != 0)
    return;

  uint64_t page_idx = phys_addr / 4096;
  bitmap_clear(page_idx);
}

void *pmm_alloc() {
  for (uint64_t i = 0; i < total_pages; ++i) {
    if (bitmap[i / 8] == 0xFF) {
      i += 7;
      continue;
    }

    if (!bitmap_test(i)) {
      bitmap_set(i);
      uint64_t phys_addr = i * 4096;
      return (void *)phys_addr;
    }
  }
  return NULL;
}

void pmm_init() {
  struct limine_memmap_response *memmap = memmap_request.response;
  hhdm_offset = hhdm_request.response->offset;

  uint64_t highest_addr = 0;

  for (uint64_t i = 0; i < memmap->entry_count; ++i) {
    struct limine_memmap_entry *entry = memmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE) {
      highest_addr = entry->base + entry->length;
    }
  }

  total_pages = highest_addr / 4096;
  bitmap_size = total_pages / 8;

  uint64_t bitmap_phys_addr;
  for (uint64_t i = 0; i < memmap->entry_count; ++i) {
    struct limine_memmap_entry *entry = memmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
      bitmap_phys_addr = entry->base;
      bitmap = PHYS_TO_VIRT(bitmap_phys_addr);
      for (uint64_t j = 0; j < bitmap_size; ++j)
        bitmap[j] = 0xFF;

      entry->base += bitmap_size;
      entry->length -= bitmap_size;
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
  for (uint64_t j = 0; j < (bitmap_size + 4095); ++j) {
    bitmap_set((bitmap_phys_addr / 4096) + j);
  }
  for (uint64_t j = 0; j < 256; j++) {
    bitmap_set(j);
  }
  fb_puts("PMM Initialized.");
}
