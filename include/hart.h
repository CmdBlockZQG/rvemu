#ifndef __HART_H__
#define __HART_H__

#include "common.h"

#include "../src/hart/local-include/state.h"

// 硬件线程
class Hart : private HartState {
  private:
    void vaddr_write(vaddr_t addr, int len, word_t data);
    word_t vaddr_read(vaddr_t addr, int len);
  
    word_t inst_fetch();
    void do_inst();

  public:
    Hart(int id);
    ~Hart();

    void step();
};

#endif
