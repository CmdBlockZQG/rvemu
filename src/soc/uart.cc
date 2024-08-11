#include "local-include/uart.h"

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>

static constexpr bool uart_input = ISDEF(CONF_UART_INPUT);

UART::UART(paddr_t addr): Device(addr, 0x1000) {
  if constexpr (uart_input) {
    int fd = stdin->_fileno;
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    // assert(system("stty raw") == 0);
    // assert(system("stty -echo") == 0);
  }
}

UART::~UART() {
  if constexpr (uart_input) {
    // assert(system("stty cooked") == 0);
  }
}

void UART::write(paddr_t addr, int len, word_t data) {
  if (addr == base) {
    putchar(data & 0xff);
    fflush(stdout);
  }
}

word_t UART::read(paddr_t addr, int len) {
  if (addr == base) {
    if (fifo.empty()) {
      return 0xff;
    } else {
      char c = fifo.front();
      fifo.pop();
      return c;
    }
  } else if (addr == base + 5) {
    return 0x60 | (!fifo.empty());
  }
  return 0;
}

static void _refill(std::queue<char> &q) {
  for (char c; read(stdin->_fileno, &c, 1) > 0; ) {
    q.push(c);
  }
}

void UART::refill() {
  _refill(fifo);
}

word_t UART::get_ip() {
  return !fifo.empty();
}
