#include "config.h"
#include "soc.h"

#include "local-include/device.h"
#include "local-include/ram.h"
#include "local-include/uart.h"
#include "local-include/timer.h"

#include <cstring>

RAM mem {0x80000000, 128 * 1024 * 1024};
UART uart {0x09000000};
Timer timer {0x09001000};
static constexpr Device *devices[] = {
  &mem,
  &uart,
  &timer,
};

void init_soc() {
  // TEMP: Nothing to do
}

void paddr_write(paddr_t addr, int len, word_t data) {
  for (Device *dev : devices) {
    if (dev->in(addr)) {
      dev->write(addr, len, data);
      return;
    }
  }
  assert(0);
}

word_t paddr_read(paddr_t addr, int len) {
  for (Device *dev : devices) {
    if (dev->in(addr)) {
      return dev->read(addr, len);
    }
  }
  assert(0);
}

void load_img(const char *filename) {
  if (filename) {
    FILE *fp = fopen(filename, "rb");
    Assert(fp, "Cannot open image file '%s'", filename);

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    Log("Image file %s, size = %ld", filename, size);

    fseek(fp, 0, SEEK_SET);
    assert(fread(mem.get_ptr(CONF_RESET_VEC), size, 1, fp) == 1);

    fclose(fp);
  } else {
    constexpr uint32_t img [] = {
      0x00500093, // addi x1, x0, 5
      0x008002ef, // jal x5, 8
      0x00608113, // addi x2, x1, 6
      0x00310093, // addi x1, x2, 3
      0x00100073, // ebreak
      0xdeadbeef  // some data
    };
    memcpy(mem.get_ptr(CONF_RESET_VEC), img, sizeof(img));
    Log("No image is given. Use the default built-in image.");
  }
}
