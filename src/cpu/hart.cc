#include "common.h"
#include "soc.h"
#include "cpu.h"

Hart::Hart(int id): HartState(id) { }

Hart::~Hart() { }

void Hart::vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_write(addr, len, data);
}

word_t Hart::vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}
