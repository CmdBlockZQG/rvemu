#ifndef __HART_EXCEPTION_H__
#define __HART_EXCEPTION_H__

#include "common.h"

struct Exception {
  word_t cause;
  word_t tval;

  Exception(word_t cause, word_t tval);
};

#endif
