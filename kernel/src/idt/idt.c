#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr idtr;

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
  uint64_t addr = (uint64_t)isr;
  idt[vector].isr_low = addr & 0xFFFF;
  idt[vector].kernel_cs = 0x08;
  idt[vector].ist = 0;
  idt[vector].attributes = flags;
  idt[vector].isr_middle = (addr >> 16) & 0xFFFF;
  idt[vector].isr_high = (addr >> 32) & 0xFFFFFFFF;
  idt[vector].reserved = 0;
}

void idt_init() {
  idtr.base = (uint64_t)&idt[0];
  idtr.limit = (uint16_t)sizeof(struct idt_entry) * 256 - 1;
  for (uint64_t i = 0; i < 256; ++i) {
    idt_set_descriptor(i, 0, 0);
  }
  idt_set_descriptor(0, isr_divided_by_zero, 0x8E);
  __asm__ volatile("lidt %0" ::"m"(idtr));
  __asm__ volatile("sti");
}
