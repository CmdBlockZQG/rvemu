#ifndef __HART_STATE_H__
#define __HART_STATE_H__

#include "common.h"

// 硬件线程特权级
enum class HartPriv : int {
  PRIV_U = 0, // User
  PRIV_S = 1, // Supervisior
  PRIV_M = 3  // Machine
};

// 硬件线程CSR
class HartCSR {
  public:
    word_t mvendorid, marchid, mhartid;
    word_t mstatus, mtvec, mepc, mcause;

    HartCSR(int id);
    word_t read(word_t addr);
    void write(word_t addr, word_t data);
  
  private:
    word_t &csr(word_t addr);
};

// 硬件线程状态
class HartState {
  private:
    vaddr_t pc;
    word_t gpr[MUXDEF(CONF_RVE, 16, 32)];
    HartPriv priv;
  
  public:
    HartCSR csr;

    HartState(int id);
    ~HartState();

    vaddr_t get_pc();
    void set_pc(vaddr_t dnpc);

    word_t gpr_read(int id);
    void gpr_write(int id, word_t data);
};

#endif
