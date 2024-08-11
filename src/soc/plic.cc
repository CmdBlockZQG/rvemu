#include "local-include/plic.h"

PLIC::PLIC(paddr_t addr): Device(addr, 0x4000000) { }

PLIC::~PLIC() { }

void PLIC::write(paddr_t addr, int len, word_t data) {
}

word_t PLIC::read(paddr_t addr, int len) {
  return 0;
}
