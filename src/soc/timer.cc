#include "local-include/timer.h"

#include <ctime>

Timer::Timer(paddr_t addr): Device(addr, 8) { }

Timer::~Timer() { }

void Timer::write(paddr_t addr, int len, word_t data) {
  if constexpr (rt_check) assert(0);
}

static uint64_t get_time() {
  static uint64_t boot_time = 0;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
  uint64_t us = now.tv_sec * 1000000 + now.tv_nsec / 1000;

  if (boot_time == 0) boot_time = us;
  return us - boot_time;
}

word_t Timer::read(paddr_t addr, int len) {
  if constexpr (rt_check) {
    if constexpr (xlen == 32) assert(len == 4 && (addr == base || addr == base + 4));
    else assert(len == 8 && addr == base);
  }

  uint64_t now = get_time();
  if constexpr (xlen == 32) {
    if (addr == base) return now;
    else return now >> 32;
  } else {
    return now;
  }

  return 0;
}
