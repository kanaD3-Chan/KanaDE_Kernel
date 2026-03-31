#include "fb_console.h"
#include "../limine/limine.h"
#include <stdint.h>

#define EOF -1

static void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}

typedef struct {
  uint32_t magic;
  uint32_t version;
  uint32_t headersize;
  uint32_t flags;
  uint32_t numlyph;
  uint32_t bytesperglyph;
  uint32_t height;
  uint32_t width;
} psf2_header_t;

static psf2_header_t *current_font = NULL;

volatile static uint32_t *fb_ptr = NULL;
static uint64_t fb_width = 0;
static uint64_t fb_height = 0;
static uint64_t fb_pitch = 0;

static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;

void fb_init() {
  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
    hcf();
  }

  if (fb_request.response == NULL || fb_request.response->framebuffer_count < 1)
    while (1)
      ;
  struct limine_framebuffer *fb = fb_request.response->framebuffers[0];

  fb_ptr = (uint32_t *)fb->address;
  fb_width = fb->width;
  fb_height = fb->height;
  fb_pitch = fb->pitch;

  current_font = (psf2_header_t *)font;
  if (current_font->magic != PSF2_MAGIC) {
    while (1) {
      __asm__ volatile("hlt");
    }
  }
}

static inline void put_pixel(uint32_t x, uint32_t y, uint32_t color) {
  if (x >= fb_width || y >= fb_height)
    return;
  fb_ptr[y * (fb_pitch / 4) + x] = color;
}

void fb_putchar(char c) {
  if (!current_font)
    return;
  if (c == '\n') {
    cursor_x = 0;
    cursor_y += current_font->height;
    return;
  }

  uint8_t *glyph = (uint8_t *)current_font + current_font->headersize +
                   ((uint8_t)c * current_font->bytesperglyph);

  uint32_t bytes_per_line = (current_font->width + 7) / 8;

  for (uint32_t cy = 0; cy < current_font->height; ++cy) {
    uint8_t *line_data = glyph + (cy * bytes_per_line);

    for (uint32_t cx = 0; cx < current_font->width; ++cx) {
      uint32_t byte_index = cx / 8;
      uint8_t bit_mask = 0x80 >> (cx % 8);

      if (line_data[byte_index] & bit_mask) {
        put_pixel(cursor_x + cx, cursor_y + cy, COLOR_WHITE);
      } else {
        put_pixel(cursor_x + cx, cursor_y + cy, COLOR_BLACK);
      }
    }
  }

  cursor_x += current_font->width;

  if (cursor_x + current_font->width > fb_width) {
    cursor_x = 0;
    cursor_y += current_font->height;
  }

  if (cursor_y + current_font->height > fb_height) {
    cursor_y = 0;
    cursor_x = 0;
  }
}

int fb_puts(const char *c) {
  if (c == NULL)
    return EOF;
  while (*c)
    fb_putchar(*c++);
  fb_putchar('\n');
  return 0;
}
