#ifndef __CPU_H__
#define __CPU_H__

#include "common.h"

enum class HartPriv : int {
  PRIV_U, // User
  PRIV_S, // Supervisior
  PRIV_M  // Machine
};

// 硬件线程状态
class HartState {
  private:
    vaddr_t pc;
    word_t gpr[MUXDEF(CONF_RVE, 16, 32)];
    HartPriv priv;
  
  public:
    HartState(int id);
    ~HartState();

    vaddr_t get_pc();
    void set_pc(vaddr_t dnpc);

    word_t gpr_read(int id);
    void gpr_write(int id, word_t data);
};

// 硬件线程
class Hart : private HartState {
  private:
    void vaddr_write(vaddr_t addr, int len, word_t data);
    word_t vaddr_read(vaddr_t addr, int len);
  
    word_t inst_fetch();

  public:
    Hart(int id);
    ~Hart();

    void step();
};

#endif
