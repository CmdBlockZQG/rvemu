#include "local-include/uart.h"

UART::UART(paddr_t addr): Device(addr, 8) { }

UART::~UART() { }

void UART::write(paddr_t addr, int len, word_t data) {
  if constexpr (rt_check) assert(addr == base && len == 1);
  putchar(data);
  fflush(stdout);
}

word_t UART::read(paddr_t addr, int len) {
  // TODO: 支持键盘输入
  return 0;
}
