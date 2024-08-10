#include "common.h"
#include "soc.h"
#include "hart.h"

Hart::Hart(int id): HartState(id) { }

Hart::~Hart() { }

paddr_t Hart::mmu_translate(vaddr_t vaddr) {
  word_t satp = csr.satp;
  if (!(satp >> 31)) return vaddr;

  paddr_t pt1_addr = satp << 12;
  paddr_t vpn1 = (vaddr >> 22) & 0x3ff;
  paddr_t pte1_addr = pt1_addr | (vpn1 << 2); 
  word_t pte1 = paddr_read(pte1_addr, 4);
  if constexpr (rt_check) assert(pte1 & 1);

  paddr_t pt0_addr = (pte1 << 2) & ~0xfff;
  paddr_t vpn0 = (vaddr >> 12) & 0x3ff;
  paddr_t pte0_addr = pt0_addr | (vpn0 << 2);
  word_t pte0 = paddr_read(pte0_addr, 4);
  if constexpr (rt_check) assert(pte0 & 1);

  paddr_t paddr = ((pte0 << 2) & ~0xfff) | (vaddr & 0xfff);
  return paddr;
}

void Hart::vaddr_write(vaddr_t vaddr, int len, word_t data) {
  word_t paddr = mmu_translate(vaddr);
  paddr_write(paddr, len, data);
}

word_t Hart::vaddr_read(vaddr_t vaddr, int len) {
  word_t paddr = mmu_translate(vaddr);
  return paddr_read(paddr, len);
}

word_t Hart::inst_fetch() {
  vaddr_t vaddr = get_pc();
  paddr_t paddr = mmu_translate(vaddr);
  return vaddr_read(paddr, 4);
}
