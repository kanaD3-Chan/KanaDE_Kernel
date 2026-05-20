#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr idtr;

// 声明外部的 ISR 函数
extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_4();
extern void isr_5();
extern void isr_6();
extern void isr_7();
extern void isr_8();
extern void isr_9();
extern void isr_10();
extern void isr_11();
extern void isr_12();
extern void isr_13();
extern void isr_14();
extern void isr_15();
extern void isr_16();
extern void isr_17();
extern void isr_18();
extern void isr_19();
extern void isr_20();
extern void isr_21();
extern void isr_22();
extern void isr_23();
extern void isr_24();
extern void isr_25();
extern void isr_26();
extern void isr_27();
extern void isr_28();
extern void isr_29();
extern void isr_30();
extern void isr_31();

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

  // 批量注册 0~31 号异常 (0x8E 表示 64-bit Interrupt Gate, Present, Ring 0)
  void *isrs[32] = {isr_0,  isr_1,  isr_2,  isr_3,  isr_4,  isr_5,  isr_6,
                    isr_7,  isr_8,  isr_9,  isr_10, isr_11, isr_12, isr_13,
                    isr_14, isr_15, isr_16, isr_17, isr_18, isr_19, isr_20,
                    isr_21, isr_22, isr_23, isr_24, isr_25, isr_26, isr_27,
                    isr_28, isr_29, isr_30, isr_31};

  for (int i = 0; i < 32; i++) {
    idt_set_descriptor(i, isrs[i], 0x8E);
  }

  __asm__ volatile("lidt %0" ::"m"(idtr));
  __asm__ volatile("sti");
}
