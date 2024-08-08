#include "common.h"
#include "device.h"

#include <cassert>
#include <cstdint>

// Device 抽象类
Device::Device(paddr_t base, paddr_t size): base(base), size(size) { }

Device::~Device() { }

bool Device::in(paddr_t addr) {
  return base <= addr && addr < base + size;
}

// RAM 随机存储器设备
RAM::RAM(paddr_t base, paddr_t size): Device(base, size) {
  ptr = new uint8_t[size];
}

RAM::~RAM() {
  delete[] ptr;
}

void RAM::write(paddr_t addr, int len, word_t data) {
  if constexpr (CONF_RT_CHECK) {
    assert(in(addr) && in(addr + len - 1));
  }
  void *host_addr = ptr + (addr - base);
  switch (len) {
    case 1: *static_cast<uint8_t  *>(host_addr) = data; return;
    case 2: *static_cast<uint16_t *>(host_addr) = data; return;
    case 4: *static_cast<uint32_t *>(host_addr) = data; return;
    case 8:
      if constexpr (CONF_RV64) {
        *static_cast<uint64_t *>(host_addr) = data; return;
      } else assert(0);
    default:
      if constexpr (CONF_RT_CHECK) assert(0);
  }
}

word_t RAM::read(paddr_t addr, int len) {
  if constexpr (CONF_RT_CHECK) {
    assert(in(addr) && in(addr + len - 1));
  }
  void *host_addr = ptr + (addr - base);
  switch (len) {
    case 1: return *static_cast<uint8_t  *>(host_addr);
    case 2: return *static_cast<uint16_t *>(host_addr);
    case 4: return *static_cast<uint32_t *>(host_addr);
    case 8:
      if constexpr (CONF_RV64) {
        return *static_cast<uint64_t *>(host_addr);
      } else assert(0);
    default:
      if constexpr (CONF_RT_CHECK) assert(0);
  }
}

void *RAM::get_ptr(paddr_t addr) {
  if constexpr (CONF_RT_CHECK) {
    assert(in(addr));
  }
  return ptr + (addr - base);
}
