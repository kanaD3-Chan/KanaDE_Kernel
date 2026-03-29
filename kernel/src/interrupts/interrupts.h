#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H

#include <stdint.h>
struct interrupt_frame {
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} __attribute__((packed));

__attribute__((interrupt)) void
isr_divided_by_zero(struct interrupt_frame *frame);

#endif // !__INTERRUPTS_H
