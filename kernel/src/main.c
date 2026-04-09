#include "main.h"

static inline void enable_fpu_and_sse() {
  uint64_t cr0, cr4;

  __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 &= ~(1 << 2);
  cr0 &= ~(1 << 3);
  cr0 |= (1 << 1);
  cr0 |= (1 << 5);
  __asm__ volatile("mov %0, %%cr0" ::"r"(cr0));

  __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
  cr4 |= (1 << 9);
  cr4 |= (1 << 10);
  __asm__ volatile("mov %0, %%cr4" ::"r"(cr4));

  __asm__ volatile("finit");
}

// Halt and catch fire function.
noreturn static void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}

void init() {
  enable_fpu_and_sse();
  gdt_init();
  idt_init();
  fb_init();
  pmm_init();
}

/*  The following will be our kernel's entry point.
    If renaming kmain() to something else, make sure
    to change the linker script accordingly. */
noreturn void kmain(void) {
  init();
  fb_puts("Hello World!");
  hcf();
}
