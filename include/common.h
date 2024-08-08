#ifndef __COMMON_H__
#define __COMMON_H__

#include "macro.h"
#include "config.h"
#include "utils.h"

#include <cstdint>

#ifdef CONF_RV64
  using word_t = uint64_t;
  using vaddr_t = uint64_t;
  using paddr_t = uint64_t;
  #define FMT_WORD "0x%016x"
  #define FMT_VADDR "0x%016x"
  #define FMT_PADDR "0x%016x"
#else
  using word_t = uint32_t;
  using vaddr_t = uint32_t;
  using paddr_t = uint32_t;
  #define FMT_WORD "0x%08x"
  #define FMT_VADDR "0x%08x"
  #define FMT_PADDR "0x%08x"
#endif

constexpr inline int gpr_n = MUXNDEF(CONF_RVE, 16, 32);
constexpr inline int xlen = MUXDEF(CONF_RV64, 64, 32);

#endif
