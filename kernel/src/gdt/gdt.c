#include "gdt.h"
#include <stdint.h>

struct gdt_entry gdt[3];
struct gdt_ptr gdtr;

void load_gdt(uint64_t gdtr);

void gdt_set_gate(int num, uint64_t base, uint64_t limit, uint8_t access,
                  uint8_t gran) {
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;
  gdt[num].limit_low = limit & 0xFFFF;
  gdt[num].granularity = (limit >> 16) & 0x0F;
  gdt[num].granularity |= gran & 0xF0;
  gdt[num].access = access;
}

void gdt_init() {
  gdt_set_gate(0, 0, 0, 0, 0);
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xAF);
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0x00);

  gdtr.limit = (sizeof(struct gdt_entry) * 3) - 1;
  gdtr.base = (uint64_t)&gdt;
  load_gdt((uint64_t)&gdtr);
}
