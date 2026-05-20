#ifndef __VMM_H
#define __VMM_H

#include <stddef.h>
#include <stdint.h>

#define PTE_PRESENT (1ull << 0)
#define PTE_WRITABLE (1ull << 1)
#define PTE_USER (1ull << 2)
#define PTE_NX (1ull << 63)

#define PTE_ADDR_MASK 0x000FFFFFFFFFF000

void vmm_init();
void vmm_map_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr,
                  uint64_t flags);
void vmm_unmap_page(uint64_t *pml4, uint64_t vaddr);

uint64_t *vmm_get_current_pml4();
#endif // !__VMM_H
