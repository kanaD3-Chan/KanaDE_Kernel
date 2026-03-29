#ifndef __IDT_H
#define __IDT_H

#include <stdint.h>

struct idt_entry {
  uint16_t isr_low;
  uint16_t kernel_cs;
  uint8_t ist;
  uint8_t attributes;
  uint16_t isr_middle;
  uint32_t isr_high;
  uint32_t reserved;
} __attribute__((packed));

struct idt_ptr {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

void idt_init();

#endif // !__IDT_H
