#ifndef __SOC_H__
#define __SOC_H__

#include "common.h"

void paddr_write(paddr_t addr, int len, word_t data);
word_t paddr_read(paddr_t addr, int len);

#endif
