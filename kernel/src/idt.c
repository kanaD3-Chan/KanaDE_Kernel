#include "idt.h"
#include <stdint.h>

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
