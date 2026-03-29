#include "fb_console.h"

// Set the base revision to 6, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests"))) static volatile uint64_t
    limine_base_revision[] = LIMINE_BASE_REVISION(6);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used,
               section(".limine_requests_start"))) static volatile uint64_t
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

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

__attribute__((
    used,
    section(".requests"))) static volatile struct limine_framebuffer_request
    fb_request = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0};

static uint32_t *fb_ptr = NULL;
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

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
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
