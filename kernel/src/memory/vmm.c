#include "vmm.h"
#include "fb_console/fb_console.h"
#include "memory.h"
#include "pmm.h"
#include <stdint.h>

uint64_t *vmm_get_current_pml4() {
  uint64_t cr3;
  __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
  return (uint64_t *)(cr3 & PTE_ADDR_MASK);
}

static inline void invlpg(uint64_t vaddr) {
  __asm__ volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
}

void vmm_map_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr,
                  uint64_t flags) {
  uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
  uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
  uint64_t pd_idx = (vaddr >> 21) & 0x1FF;
  uint64_t pt_idx = (vaddr >> 12) & 0x1FF;

  uint64_t *pml4_virt = PHYS_TO_VIRT(pml4);

  if (!(pml4_virt[pml4_idx] & PTE_PRESENT)) {
    uint64_t new_pdpt_phys = pmm_alloc();
    memset(PHYS_TO_VIRT(new_pdpt_phys), 0, 4096);
    pml4_virt[pml4_idx] = new_pdpt_phys | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
  }

  uint64_t *pdpt_virt = PHYS_TO_VIRT(pml4_virt[pml4_idx] & PTE_ADDR_MASK);
  if (!(pdpt_virt[pdpt_idx] & PTE_PRESENT)) {
    uint64_t new_pd_phys = pmm_alloc();
    memset(PHYS_TO_VIRT(new_pd_phys), 0, 4096);
    pdpt_virt[pdpt_idx] = new_pd_phys | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
  }

  uint64_t *pd_virt = PHYS_TO_VIRT(pdpt_virt[pdpt_idx] & PTE_ADDR_MASK);
  if (!(pd_virt[pd_idx] & PTE_PRESENT)) {
    uint64_t new_pt_phys = pmm_alloc();
    memset(PHYS_TO_VIRT(new_pt_phys), 0, 4096);
    pd_virt[pd_idx] = new_pt_phys | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
  }

  uint64_t *pt_virt = PHYS_TO_VIRT(pd_virt[pd_idx] & PTE_ADDR_MASK);
  pt_virt[pt_idx] = (paddr & PTE_ADDR_MASK) | flags;

  invlpg(vaddr);
}

void vmm_init() {
  uint64_t *kernel_pml4 = vmm_get_current_pml4();

  fb_puts("VMM Initialized");
}
