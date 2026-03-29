#ifndef __FB_CONSOLE_H
#define __FB_CONSOLE_H
#include "fonts/font_psf.h"
#include "limine.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PSF2_MAGIC 0x864ab572
#define COLOR_WHITE 0x00FFFFFF
#define COLOR_BLACK 0x00000000

void fb_init();
void fb_putchar(char c);
int fb_puts(const char *c);
#endif // !__FB_CONSOLE_H
