#include "interrupts/interrupts.h"
#include "fb_console/fb_console.h"

__attribute__((interrupt)) void
isr_divided_by_zero(struct interrupt_frame *frame __attribute__((unused))) {
  fb_puts("Divided by zero!");
  while (1)
    __asm__ volatile("hlt");
}
