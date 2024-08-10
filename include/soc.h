#ifndef __SOC_H__
#define __SOC_H__

#include "common.h"

#include "soc/local-include/clint.h"

void paddr_write(paddr_t addr, int len, word_t data);
word_t paddr_read(paddr_t addr, int len);

extern CLINT clint;

#endif
